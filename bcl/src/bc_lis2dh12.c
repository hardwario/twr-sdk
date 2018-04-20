#include <bc_lis2dh12.h>
#include <bc_scheduler.h>
#include <bc_exti.h>
#include <stm32l0xx.h>

#define _BC_LIS2DH12_DELAY_RUN 10
#define _BC_LIS2DH12_DELAY_READ 10
#define _BC_LIS2DH12_AUTOINCREMENT_ADR 0x80

static void _bc_lis2dh12_task_interval(void *param);
static void _bc_lis2dh12_task_measure(void *param);
static bool _bc_lis2dh12_power_down(bc_lis2dh12_t *self);
static bool _bc_lis2dh12_continuous_conversion(bc_lis2dh12_t *self);
static bool _bc_lis2dh12_read_result(bc_lis2dh12_t *self);
static void _bc_lis2dh12_interrupt(bc_exti_line_t line, void *param);

static const float _bc_lis2dh12_fs_lut[] =
{
        [BC_LIS2DH12_SCALE_2G] = (1/1000.f),
        [BC_LIS2DH12_SCALE_4G] = (2/1000.f),
        [BC_LIS2DH12_SCALE_8G] = (4/1000.f),
        [BC_LIS2DH12_SCALE_16G] = (12/1000.f)
};

bool bc_lis2dh12_init(bc_lis2dh12_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    // Enable GPIOB clock
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

    // Errata workaround
    RCC->IOPENR;

    // Set input mode
    GPIOB->MODER &= ~GPIO_MODER_MODE6_Msk;

    self->_task_id_interval = bc_scheduler_register(_bc_lis2dh12_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_lis2dh12_task_measure, self, _BC_LIS2DH12_DELAY_RUN);

    return true;
}

void bc_lis2dh12_set_event_handler(bc_lis2dh12_t *self, void (*event_handler)(bc_lis2dh12_t *, bc_lis2dh12_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_lis2dh12_set_update_interval(bc_lis2dh12_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_lis2dh12_measure(self);
    }
}

bool bc_lis2dh12_measure(bc_lis2dh12_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool bc_lis2dh12_get_result_raw(bc_lis2dh12_t *self, bc_lis2dh12_result_raw_t *result_raw)
{
    *result_raw = self->_raw;

    return self->_accelerometer_valid;
}

bool bc_lis2dh12_get_result_g(bc_lis2dh12_t *self, bc_lis2dh12_result_g_t *result_g)
{
    bc_lis2dh12_result_raw_t result_raw;

    if (!bc_lis2dh12_get_result_raw(self, &result_raw))
    {
        return false;
    }

    float sensitivity = _bc_lis2dh12_fs_lut[self->_scale];

    result_g->x_axis = (result_raw.x_axis >> 4) * sensitivity;
    result_g->y_axis = (result_raw.y_axis >> 4) * sensitivity;
    result_g->z_axis = (result_raw.z_axis >> 4) * sensitivity;

    return true;
}

static void _bc_lis2dh12_task_interval(void *param)
{
    bc_lis2dh12_t *self = param;

    bc_lis2dh12_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_lis2dh12_task_measure(void *param)
{
    bc_lis2dh12_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case BC_LIS2DH12_STATE_ERROR:
            {
                self->_accelerometer_valid = false;

                self->_measurement_active = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_LIS2DH12_EVENT_ERROR, self->_event_param);
                }

                self->_state = BC_LIS2DH12_STATE_INITIALIZE;

                return;
            }
            case BC_LIS2DH12_STATE_INITIALIZE:
            {
                self->_state = BC_LIS2DH12_STATE_ERROR;

                // Read and check WHO_AM_I register
                uint8_t who_am_i;
                if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x0f, &who_am_i))
                {
                    continue;
                }

                if (who_am_i != 0x33)
                {
                    continue;
                }

                uint8_t cfg_reg4 = 0x80 | ((uint8_t) self->_scale << 4) | (((uint8_t) self->_resolution & 0x01) << 3);

                if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x23, cfg_reg4))
                {
                    continue;
                }

                if (!_bc_lis2dh12_power_down(self))
                {
                    continue;
                }

                self->_state = BC_LIS2DH12_STATE_MEASURE;

                if (self->_measurement_active)
                {
                    bc_scheduler_plan_current_from_now(10);
                }

                return;
            }
            case BC_LIS2DH12_STATE_MEASURE:
            {
                self->_state = BC_LIS2DH12_STATE_ERROR;

                if (!_bc_lis2dh12_continuous_conversion(self))
                {
                    continue;
                }

                self->_state = BC_LIS2DH12_STATE_READ;

                bc_scheduler_plan_current_from_now(_BC_LIS2DH12_DELAY_READ);

                return;
            }
            case BC_LIS2DH12_STATE_READ:
            {
                self->_state = BC_LIS2DH12_STATE_ERROR;

                if (!_bc_lis2dh12_read_result(self))
                {
                    continue;
                }

                // Power down only when no alarm is set
                if(!self->_alarm_active)
                {
                    if (!_bc_lis2dh12_power_down(self))
                    {
                        continue;
                    }
                }

                self->_accelerometer_valid = true;

                self->_state = BC_LIS2DH12_STATE_UPDATE;

                continue;
            }
            case BC_LIS2DH12_STATE_UPDATE:
            {
                self->_state = BC_LIS2DH12_STATE_ERROR;

                self->_measurement_active = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_LIS2DH12_EVENT_UPDATE, self->_event_param);
                }

                // When the interrupt alarm is active
                if(self->_alarm_active)
                {
                    uint8_t int1_src;

                    if(!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x31, &int1_src))
                    {
                        continue;
                    }

                    if(self->_irq_flag)
                    {
                        self->_irq_flag = 0;

                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, BC_LIS2DH12_EVENT_ALARM, self->_event_param);
                        }
                    }
                }

                self->_state = BC_LIS2DH12_STATE_MEASURE;

                return;
            }
            default:
            {
                self->_state = BC_LIS2DH12_STATE_ERROR;

                continue;
            }
        }
    }
}

static bool _bc_lis2dh12_power_down(bc_lis2dh12_t *self)
{
    if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, 0x07))
    {
        return false;
    }

    return true;
}

static bool _bc_lis2dh12_continuous_conversion(bc_lis2dh12_t *self)
{

    uint8_t cfg_reg1 = 0x57 | ((self->_resolution & 0x02) << 2);

    // ODR = 0x5 => 100Hz
    if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, cfg_reg1))
    {
        return false;
    }

    return true;
}

static bool _bc_lis2dh12_read_result(bc_lis2dh12_t *self)
{
     bc_i2c_memory_transfer_t transfer;

     transfer.device_address = self->_i2c_address;
     transfer.memory_address = _BC_LIS2DH12_AUTOINCREMENT_ADR | 0x28;
     transfer.buffer = &self->_raw;
     transfer.length = 6;

     return bc_i2c_memory_read(self->_i2c_channel, &transfer);
}

bool bc_lis2dh12_set_alarm(bc_lis2dh12_t *self, bc_lis2dh12_alarm_t *alarm)
{
    if (alarm != NULL)
    {
        // Enable alarm
        self->_alarm_active = true;

        // Disable IRQ first to change the registers
        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, 0x00))
        {
            return false;
        }

        // Recalculate threshold to the 4g full-scale setting
        uint8_t int1_ths = (uint8_t)((alarm->threshold) / 0.031f);

        // Ensure minimum threshold level
        if (int1_ths == 0)
        {
            int1_ths = 1;
        }

        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x32, int1_ths))
        {
            return false;
        }

        uint8_t int1_duration = (uint8_t)(alarm->duration / 1); // /10
        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x33, int1_duration))
        {
            return false;
        }

        // CTRL_REG3
        uint8_t ctrl_reg3 = (1 << 6);
        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x22, ctrl_reg3))
        {
            return false;
        }

        // CTRL_REG6 - invert interrupt
        uint8_t ctrl_reg6 = (1 << 1);
        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x25, ctrl_reg6))
        {
            return false;
        }

        // ctr_reg5
        uint8_t ctrl_reg5 = (0 << 3); // latch interrupt request
        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x24, ctrl_reg5))
        {
            return false;
        }

        // INT_CFG1
        uint8_t int_cfg1;

        int_cfg1 = 0; //(1 << 7) | (1 << 6); // AOI = 0, 6D = 1

        int_cfg1 |= (alarm->z_high) ? (1 << 5) : 0;
        int_cfg1 |= (alarm->z_low) ? (1 << 4) : 0;

        int_cfg1 |= (alarm->y_high) ? (1 << 3) : 0;
        int_cfg1 |= (alarm->y_low) ? (1 << 2) : 0;

        int_cfg1 |= (alarm->x_high) ? (1 << 1) : 0;
        int_cfg1 |= (alarm->x_low) ? (1 << 0) : 0;

        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, int_cfg1))
        {
            return false;
        }

        bc_exti_register(BC_EXTI_LINE_PB6, BC_EXTI_EDGE_FALLING, _bc_lis2dh12_interrupt, self);
    }
    else
    {
        // Disable alarm
        self->_alarm_active = false;

        if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, 0x00))
        {
            return false;
        }

        bc_exti_unregister(BC_EXTI_LINE_PB6);
    }

    bc_lis2dh12_measure(self);

    return true;
}

bool bc_lis2dh12_set_resolution(bc_lis2dh12_t *self, bc_lis2dh12_resolution_t resolution)
{
    self->_resolution = resolution;

    self->_state = BC_LIS2DH12_STATE_INITIALIZE;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool bc_lis2dh12_set_scale(bc_lis2dh12_t *self, bc_lis2dh12_scale_t scale)
{
    self->_scale = scale;

    self->_state = BC_LIS2DH12_STATE_INITIALIZE;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

static void _bc_lis2dh12_interrupt(bc_exti_line_t line, void *param)
{
    (void) line;

    bc_lis2dh12_t *self = param;

    self->_irq_flag = true;

    bc_lis2dh12_measure(self);
}
