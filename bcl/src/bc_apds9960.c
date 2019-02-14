#include <bc_apds9960.h>
#include <bc_exti.h>
#include <stm32l0xx.h>
#include <bc_usb_cdc.h>

#define BC_APDS9960_GESTURE_THRESHOLD_OUT 5
#define BC_APDS9960_FIFO_PAUSE_TIME       30
#define GESTURE_SENSITIVITY_1   20
#define GESTURE_SENSITIVITY_2   20

#define BC_APDS9960_GESTURE_STATE_NEAR 1
#define BC_APDS9960_GESTURE_STATE_FAR 2


static void _bc_apds9960_task(void *param);
static bool _bc_apds9960_is_gesture_available(bc_apds9960_t *self, bool *available);
static bool _bc_apds9960_get_available_data(bc_apds9960_t *self, uint8_t *fifo_level);
static bool _bc_apds9960_load_buffer_data(bc_apds9960_t *self, uint8_t *fifo_level);
static bool _bc_apds9960_process_gesture_data(bc_apds9960_t *self);
static bool _bc_apds9960_decode_gesture(bc_apds9960_t *self);
static void _bc_apds9960_reset_gesture_parameters(bc_apds9960_t *self);
static void _bc_apds9960_interrupt(bc_exti_line_t line, void *param);

void bc_apds9960_init(bc_apds9960_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    // Enable GPIOC clock
    RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    // Set input mode
    GPIOC->MODER &= ~GPIO_MODER_MODE13_Msk;

    self->_task_id = bc_scheduler_register(_bc_apds9960_task, self, bc_tick_get() + 50);
}

void bc_apds9960_set_event_handler(bc_apds9960_t *self, void (*event_handler)(bc_apds9960_t *, bc_apds9960_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_apds9960_set_proximity_threshold(bc_apds9960_t *self, uint8_t low, uint8_t high)
{

}

bool bc_apds9960_get_proximity(bc_apds9960_t *self, uint8_t *value)
{
    *value = self->_p;
    //    if (self->_state != BC_APDS9960_STATE_READY)
//    {
//        return false;
//    }
//
    if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x9C, value))
    {
        self->_state = BC_APDS9960_STATE_ERROR;
        return false;
    }

    return true;
}

void bc_apds9960_get_gesture(bc_apds9960_t *self, bc_apds9960_gesture_t *gesture)
{
    *gesture = self->_gesture;
}

static void _bc_apds9960_task(void *param)
{
    bc_apds9960_t *self = param;

start:

    switch (self->_state)
    {
        case BC_APDS9960_STATE_ERROR:
        {
            self->_state = BC_APDS9960_STATE_INITIALIZE;

            bc_scheduler_plan_current_relative(50);

            return;
        }
        case BC_APDS9960_STATE_INITIALIZE:
        {
            uint8_t device_id;

            if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x92, &device_id) || (device_id != 0xAB))
            {
                self->_state = BC_APDS9960_STATE_ERROR;
                goto start;
            }

            self->_control_reg = BC_APDS9960_LDRIVE_100MA | BC_APDS9960_PGAIN_4X | BC_APDS9960_AGAIN_4X;

            if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x80, 0x00) || // Set low power sleep mode

                /* Set default values for ambient light and proximity registers */

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x81, 246) || // 103ms
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x83, 171) || // 27ms
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x8E, 0x89) || // 16us, 8 pulses
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x9D, 0x00) ||
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x9E, 0x00) ||
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x8D, 0x40 | 0x02) ||  // No 12x wait (WTIME) factor
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x8F, self->_control_reg) ||

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x89, 0) || // Set low proximity threshold
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x8B, 50) || // Set high proximity threshold

                //TODO light treshold

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x8C, 0x11) || // 2 consecutive prox or ALS for int.

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x90, 0x01) ||  // No saturation interrupts or LED boost

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x9F, 0x00) || // Enable all photodiodes, no SAI

                //(1 << 2) |
//                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x80, (1 << 2) | (1 << 0)) || // Power ON. & Proximity Detect Enable.

//                /* Set default values for gesture sense registers */
//
                //setGestureEnterThresh
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA0, 30) || // Threshold for entering gesture mode, GPENTH
                //setGestureExitThresh
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA1, 20) || // Threshold for exiting gesture mode, GEPERS
                //
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA2, (1 << 6) ) || // 4 gesture events for int., 1 for exit
                //setGestureGain
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA2, 0x40) ||

                // setGestureGain setGestureLEDDrive setGestureWaitTime
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA3, (2 << 5) | (3 << 3) | (7)) ||

                //APDS9960_GOFFSET_U
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA4, 0x00) ||
                //APDS9960_GOFFSET_D
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA5, 0x00) ||
                //APDS9960_GOFFSET_L
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA7, 0x00) ||
                //APDS9960_GOFFSET_R
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA9, 0x00) ||


                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xA6, 0xC9) || //32us, 10 pulses

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xAA, 0x00) || //ll photodiodes active during gesture

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xAB, 0x00) || //Disable gesture interrupts


                //run questure
                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x90, (0 << 4) ) ||  // LED boost 100

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x83, 0xFF) ||

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x8E, 0x89) || // 16us, 10 pulses

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0xAB, 0x03) || //setGestureMode setGestureIntEnable(1)

                !bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x80, (1 << 6) | (1 << 2) | (1 << 3) | (1 << 0)) //GESTURE PROXIMITY WAIT ON

            )
            {
                self->_state = BC_APDS9960_STATE_ERROR;
                goto start;
            }

            bc_exti_register(BC_EXTI_LINE_PC13, BC_EXTI_EDGE_RISING_AND_FALLING, _bc_apds9960_interrupt, self); // BC_EXTI_EDGE_FALLING

            self->_state = BC_APDS9960_STATE_READY;
//            bc_scheduler_plan_current_relative(500);

            return;
        }
        case BC_APDS9960_STATE_READY:
        {
            if (self->_irq_flag)
            {
                self->_irq_flag = false;
                self->_state = BC_APDS9960_STATE_GESTURE_READ_DATA;
                goto start;
            }

//            bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x80, (1 << 2) | (1 << 0));
//            self->_state = BC_APDS9960_STATE_GESTURE_READ_PROXIMITY;
//            bc_scheduler_plan_current_relative(30);
            return;
        case BC_APDS9960_STATE_GESTURE_READ_PROXIMITY:


            if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x9C, &self->_p))
            {
                self->_state = BC_APDS9960_STATE_ERROR;
                goto start;
            }

            bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x80, 0x00);
            self->_state = BC_APDS9960_STATE_READY;
            bc_scheduler_plan_current_relative(500);
            return;
        }
        case BC_APDS9960_STATE_GESTURE_READ_DATA:
        {
            bool available;
            uint8_t fifo_level;

            if (!_bc_apds9960_is_gesture_available(self, &available))
            {
                goto start;
            }

            if (available)
            {
                if (!_bc_apds9960_get_available_data(self, &fifo_level))
                {
                    goto start;
                }

                if (fifo_level == 0)
                {
                    bc_scheduler_plan_current_relative(BC_APDS9960_FIFO_PAUSE_TIME);
                    return;
                }

                if(!_bc_apds9960_load_buffer_data(self, &fifo_level))
                {
                    goto start;
                }

                if (_bc_apds9960_process_gesture_data(self))
                {
                    if (_bc_apds9960_decode_gesture(self))
                    {
                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, BC_APDS9960_EVENT_UPDATE, self->_event_param);
                        }
                    }
                }

                bc_scheduler_plan_current_relative(BC_APDS9960_FIFO_PAUSE_TIME);
            }
            else
            {
                if (_bc_apds9960_decode_gesture(self))
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_APDS9960_EVENT_UPDATE, self->_event_param);
                    }
                }
                _bc_apds9960_reset_gesture_parameters(self);
            }
        }
    }
}

static bool _bc_apds9960_is_gesture_available(bc_apds9960_t *self, bool *available)
{
    uint8_t status;
    if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0xAF, &status))
    {
        self->_state = BC_APDS9960_STATE_ERROR;
        return false;
    }
    *available = status & 0x01 ? true : false;
    return true;
}

static bool _bc_apds9960_get_available_data(bc_apds9960_t *self, uint8_t *fifo_level)
{
    if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0xAE, fifo_level))
    {
        self->_state = BC_APDS9960_STATE_ERROR;
        return false;
    }
    return true;
}

static bool _bc_apds9960_load_buffer_data(bc_apds9960_t *self, uint8_t *fifo_level)
{
    bc_i2c_memory_transfer_t transfer;

    transfer.device_address = self->_i2c_address;
    transfer.memory_address = 0xFC;
    transfer.buffer = self->_buffer;
    transfer.length = *fifo_level * 4;

    if (transfer.length > sizeof(self->_buffer))
    {
        transfer.length = sizeof(self->_buffer);
    }

    if (!bc_i2c_memory_read(self->_i2c_channel, &transfer))
    {
        self->_state = BC_APDS9960_STATE_ERROR;
        return false;
    }

    self->_buffer_length = (uint8_t)transfer.length;

    return true;
}

static bool _bc_apds9960_process_gesture_data(bc_apds9960_t *self)
{
    int ud_ratio_first = 0;
    int lr_ratio_first = 0;
    int ud_ratio_last = 0;
    int lr_ratio_last = 0;
    int ud_delta;
    int lr_delta;
    int i;

    if (self->_buffer_length < 16)
    {
        return false;
    }

    for (i = 0; i < self->_buffer_length; i += 4){
        if( (self->_buffer[i] > BC_APDS9960_GESTURE_THRESHOLD_OUT) &&
        (self->_buffer[i + 1] > BC_APDS9960_GESTURE_THRESHOLD_OUT) &&
        (self->_buffer[i + 2] > BC_APDS9960_GESTURE_THRESHOLD_OUT) &&
        (self->_buffer[i + 3] > BC_APDS9960_GESTURE_THRESHOLD_OUT) ) {
            ud_ratio_first = ((self->_buffer[i] - self->_buffer[i + 1]) * 100) / (self->_buffer[i] + self->_buffer[i + 1]);
            lr_ratio_first = ((self->_buffer[i + 2] - self->_buffer[i + 3]) * 100) / (self->_buffer[i + 2] + self->_buffer[i + 3]);
            break;
        }
    }

    if (i == self->_buffer_length)
    {
        return false;
    }

    for (i = self->_buffer_length - 4; i >= 0; i -= 4){
        if( (self->_buffer[i] > BC_APDS9960_GESTURE_THRESHOLD_OUT) &&
        (self->_buffer[i + 1] > BC_APDS9960_GESTURE_THRESHOLD_OUT) &&
        (self->_buffer[i + 2] > BC_APDS9960_GESTURE_THRESHOLD_OUT) &&
        (self->_buffer[i + 3] > BC_APDS9960_GESTURE_THRESHOLD_OUT) ) {
            ud_ratio_last = ((self->_buffer[i] - self->_buffer[i + 1]) * 100) / (self->_buffer[i] + self->_buffer[i + 1]);
            lr_ratio_last = ((self->_buffer[i + 2] - self->_buffer[i + 3]) * 100) / (self->_buffer[i + 2] + self->_buffer[i + 3]);
            break;
        }
    }

    if (i < 0)
    {
        return false;
    }

    ud_delta = ud_ratio_last - ud_ratio_first;
    lr_delta = lr_ratio_last - lr_ratio_first;

    self->gesture_ud_delta_ += ud_delta;
    self->gesture_lr_delta_ += lr_delta;

    /* Determine U/D gesture */
    if (self->gesture_ud_delta_ >= GESTURE_SENSITIVITY_1 )
    {
        self->gesture_ud_count_ = 1;
    }
    else if (self->gesture_ud_delta_ <= -GESTURE_SENSITIVITY_1 )
    {
        self->gesture_ud_count_ = -1;
    }
    else
    {
        self->gesture_ud_count_ = 0;
    }

    /* Determine L/R gesture */
    if (self->gesture_lr_delta_ >= GESTURE_SENSITIVITY_1 )
    {
        self->gesture_lr_count_ = 1;
    }
    else if (self->gesture_lr_delta_ <= -GESTURE_SENSITIVITY_1 )
    {
        self->gesture_lr_count_ = -1;
    }
    else
    {
        self->gesture_lr_count_ = 0;
    }

    /* Determine Near/Far gesture */
    if ((self->gesture_ud_count_ == 0) && (self->gesture_lr_count_ == 0))
    {
        if ((abs(ud_delta) < GESTURE_SENSITIVITY_2) && (abs(lr_delta) < GESTURE_SENSITIVITY_2))
        {

            if ((ud_delta == 0) && (lr_delta == 0))
            {
                self->gesture_near_count_++;
            }
            else if((ud_delta != 0) || (lr_delta != 0))
            {
                self->gesture_far_count_++;
            }

            if ((self->gesture_near_count_ >= 10) && (self->gesture_far_count_ >= 2))
            {
                if ((ud_delta == 0) && (lr_delta == 0))
                {
                    self->gesture_state_ = BC_APDS9960_GESTURE_STATE_NEAR;
                }
                else if( (ud_delta != 0) && (lr_delta != 0))
                {
                    self->gesture_state_ = BC_APDS9960_GESTURE_STATE_FAR;
                }
                return true;
            }
        }
    }
    else
    {
        if ((abs(ud_delta) < GESTURE_SENSITIVITY_2) && (abs(lr_delta) < GESTURE_SENSITIVITY_2))
        {
//
            if ((ud_delta == 0) && (lr_delta == 0))
            {
                self->gesture_near_count_++;
            }

            if (self->gesture_near_count_ >= 10 ) {
                self->gesture_ud_count_ = 0;
                self->gesture_lr_count_ = 0;
                self->gesture_ud_delta_ = 0;
                self->gesture_lr_delta_ = 0;
            }
        }
    }

    return false;
}

static bool _bc_apds9960_decode_gesture(bc_apds9960_t *self)
{

    self->_gesture = BC_APDS9960_GESTURE_NONE;

    if (self->gesture_state_ == BC_APDS9960_GESTURE_STATE_NEAR)
    {
        self->_gesture = BC_APDS9960_GESTURE_NEAR;
        return true;
    }
    else if (self->gesture_state_ == BC_APDS9960_GESTURE_STATE_FAR)
    {
        self->_gesture = BC_APDS9960_GESTURE_FAR;
        return true;
    }

    /* Determine swipe direction */
    if ((self->gesture_ud_count_ == -1) && (self->gesture_lr_count_ == 0))
    {
        self->_gesture = BC_APDS9960_GESTURE_UP;
    }
    else if ((self->gesture_ud_count_ == 1) && (self->gesture_lr_count_ == 0))
    {
        self->_gesture = BC_APDS9960_GESTURE_DOWN;
    }
    else if ((self->gesture_ud_count_ == 0) && (self->gesture_lr_count_ == 1))
    {
        self->_gesture = BC_APDS9960_GESTURE_RIGHT;
    }
    else if ((self->gesture_ud_count_ == 0) && (self->gesture_lr_count_ == -1))
    {
        self->_gesture = BC_APDS9960_GESTURE_LEFT;
    }
    else if ((self->gesture_ud_count_ == -1) && (self->gesture_lr_count_ == 1))
    {
        if(abs(self->gesture_ud_delta_) > abs(self->gesture_lr_delta_) ) {
            self->_gesture = BC_APDS9960_GESTURE_UP;
        }
        else
        {
            self->_gesture = BC_APDS9960_GESTURE_RIGHT;
        }
    }
    else if ((self->gesture_ud_count_ == 1) && (self->gesture_lr_count_ == -1))
    {
        if (abs(self->gesture_ud_delta_) > abs(self->gesture_lr_delta_) ) {
            self->_gesture = BC_APDS9960_GESTURE_DOWN;
        }
        else
        {
            self->_gesture = BC_APDS9960_GESTURE_LEFT;
        }
    }
    else if ((self->gesture_ud_count_ == -1) && (self->gesture_lr_count_ == -1))
    {
        if (abs(self->gesture_ud_delta_) > abs(self->gesture_lr_delta_))
        {
           self->_gesture = BC_APDS9960_GESTURE_UP;
        }
        else
        {
           self->_gesture = BC_APDS9960_GESTURE_LEFT;
        }
    }
    else if ((self->gesture_ud_count_ == 1) && (self->gesture_lr_count_ == 1) )
    {
        if (abs(self->gesture_ud_delta_) > abs(self->gesture_lr_delta_))
        {
           self->_gesture = BC_APDS9960_GESTURE_DOWN;
        }
        else
        {
           self->_gesture = BC_APDS9960_GESTURE_RIGHT;
        }
    }
    else
    {
        return false;
    }

    return true;
}

static void _bc_apds9960_reset_gesture_parameters(bc_apds9960_t *self)
{
    self->gesture_ud_delta_ = 0;
    self->gesture_lr_delta_ = 0;
    self->gesture_ud_count_ = 0;
    self->gesture_lr_count_ = 0;
    self->gesture_near_count_ = 0;
    self->gesture_far_count_ = 0;
    self->gesture_state_ = 0;
}

static void _bc_apds9960_interrupt(bc_exti_line_t line, void *param)
{
    (void) line;

    bc_apds9960_t *self = param;

    self->_irq_flag = true;

    bc_scheduler_plan_relative(self->_task_id, BC_APDS9960_FIFO_PAUSE_TIME);

}
