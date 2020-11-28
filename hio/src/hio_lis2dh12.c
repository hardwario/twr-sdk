#include <hio_lis2dh12.h>
#include <hio_scheduler.h>
#include <hio_exti.h>
#include <stm32l0xx.h>

#define _HIO_LIS2DH12_DELAY_RUN 10
#define _HIO_LIS2DH12_DELAY_READ 10
#define _HIO_LIS2DH12_AUTOINCREMENT_ADR 0x80

static void _hio_lis2dh12_task_interval(void *param);
static void _hio_lis2dh12_task_measure(void *param);
static bool _hio_lis2dh12_power_down(hio_lis2dh12_t *self);
static bool _hio_lis2dh12_continuous_conversion(hio_lis2dh12_t *self);
static bool _hio_lis2dh12_read_result(hio_lis2dh12_t *self);
static void _hio_lis2dh12_interrupt(hio_exti_line_t line, void *param);

static const float _hio_lis2dh12_fs_lut[] =
{
        [HIO_LIS2DH12_SCALE_2G] = (1/1000.f),
        [HIO_LIS2DH12_SCALE_4G] = (2/1000.f),
        [HIO_LIS2DH12_SCALE_8G] = (4/1000.f),
        [HIO_LIS2DH12_SCALE_16G] = (12/1000.f)
};

bool hio_lis2dh12_init(hio_lis2dh12_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);

    // Enable GPIOB clock
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

    // Errata workaround
    RCC->IOPENR;

    // Set input mode
    GPIOB->MODER &= ~GPIO_MODER_MODE6_Msk;

    self->_task_id_interval = hio_scheduler_register(_hio_lis2dh12_task_interval, self, HIO_TICK_INFINITY);
    self->_task_id_measure = hio_scheduler_register(_hio_lis2dh12_task_measure, self, _HIO_LIS2DH12_DELAY_RUN);

    return true;
}

void hio_lis2dh12_set_event_handler(hio_lis2dh12_t *self, void (*event_handler)(hio_lis2dh12_t *, hio_lis2dh12_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_lis2dh12_set_update_interval(hio_lis2dh12_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        hio_lis2dh12_measure(self);
    }
}

bool hio_lis2dh12_measure(hio_lis2dh12_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    hio_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool hio_lis2dh12_get_result_raw(hio_lis2dh12_t *self, hio_lis2dh12_result_raw_t *result_raw)
{
    *result_raw = self->_raw;

    return self->_accelerometer_valid;
}

bool hio_lis2dh12_get_result_g(hio_lis2dh12_t *self, hio_lis2dh12_result_g_t *result_g)
{
    hio_lis2dh12_result_raw_t result_raw;

    if (!hio_lis2dh12_get_result_raw(self, &result_raw))
    {
        return false;
    }

    float sensitivity = _hio_lis2dh12_fs_lut[self->_scale];

    result_g->x_axis = (result_raw.x_axis >> 4) * sensitivity;
    result_g->y_axis = (result_raw.y_axis >> 4) * sensitivity;
    result_g->z_axis = (result_raw.z_axis >> 4) * sensitivity;

    return true;
}

static void _hio_lis2dh12_task_interval(void *param)
{
    hio_lis2dh12_t *self = param;

    hio_lis2dh12_measure(self);

    hio_scheduler_plan_current_relative(self->_update_interval);
}

static void _hio_lis2dh12_task_measure(void *param)
{
    hio_lis2dh12_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case HIO_LIS2DH12_STATE_ERROR:
            {
                self->_accelerometer_valid = false;

                self->_measurement_active = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, HIO_LIS2DH12_EVENT_ERROR, self->_event_param);
                }

                self->_state = HIO_LIS2DH12_STATE_INITIALIZE;

                return;
            }
            case HIO_LIS2DH12_STATE_INITIALIZE:
            {
                self->_state = HIO_LIS2DH12_STATE_ERROR;

                // Read and check WHO_AM_I register
                uint8_t who_am_i;
                if (!hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x0f, &who_am_i))
                {
                    continue;
                }

                if (who_am_i != 0x33)
                {
                    continue;
                }

                uint8_t cfg_reg4 = 0x80 | ((uint8_t) self->_scale << 4) | (((uint8_t) self->_resolution & 0x01) << 3);

                if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x23, cfg_reg4))
                {
                    continue;
                }

                if (!_hio_lis2dh12_power_down(self))
                {
                    continue;
                }

                self->_state = HIO_LIS2DH12_STATE_MEASURE;

                if (self->_measurement_active)
                {
                    hio_scheduler_plan_current_from_now(10);
                }

                return;
            }
            case HIO_LIS2DH12_STATE_MEASURE:
            {
                self->_state = HIO_LIS2DH12_STATE_ERROR;

                if (!_hio_lis2dh12_continuous_conversion(self))
                {
                    continue;
                }

                self->_state = HIO_LIS2DH12_STATE_READ;

                hio_scheduler_plan_current_from_now(_HIO_LIS2DH12_DELAY_READ);

                return;
            }
            case HIO_LIS2DH12_STATE_READ:
            {
                self->_state = HIO_LIS2DH12_STATE_ERROR;

                if (!_hio_lis2dh12_read_result(self))
                {
                    continue;
                }

                // Power down only when no alarm is set
                if(!self->_alarm_active)
                {
                    if (!_hio_lis2dh12_power_down(self))
                    {
                        continue;
                    }
                }

                self->_accelerometer_valid = true;

                self->_state = HIO_LIS2DH12_STATE_UPDATE;

                continue;
            }
            case HIO_LIS2DH12_STATE_UPDATE:
            {
                self->_state = HIO_LIS2DH12_STATE_ERROR;

                self->_measurement_active = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, HIO_LIS2DH12_EVENT_UPDATE, self->_event_param);
                }

                // When the interrupt alarm is active
                if(self->_alarm_active)
                {
                    uint8_t int1_src;

                    if(!hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x31, &int1_src))
                    {
                        continue;
                    }

                    if(self->_irq_flag)
                    {
                        self->_irq_flag = 0;

                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, HIO_LIS2DH12_EVENT_ALARM, self->_event_param);
                        }
                    }
                }

                self->_state = HIO_LIS2DH12_STATE_MEASURE;

                return;
            }
            default:
            {
                self->_state = HIO_LIS2DH12_STATE_ERROR;

                continue;
            }
        }
    }
}

static bool _hio_lis2dh12_power_down(hio_lis2dh12_t *self)
{
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, 0x07))
    {
        return false;
    }

    return true;
}

static bool _hio_lis2dh12_continuous_conversion(hio_lis2dh12_t *self)
{

    uint8_t cfg_reg1 = 0x57 | ((self->_resolution & 0x02) << 2);

    // ODR = 0x5 => 100Hz
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, cfg_reg1))
    {
        return false;
    }

    return true;
}

static bool _hio_lis2dh12_read_result(hio_lis2dh12_t *self)
{
     hio_i2c_memory_transfer_t transfer;

     transfer.device_address = self->_i2c_address;
     transfer.memory_address = _HIO_LIS2DH12_AUTOINCREMENT_ADR | 0x28;
     transfer.buffer = &self->_raw;
     transfer.length = 6;

     return hio_i2c_memory_read(self->_i2c_channel, &transfer);
}

bool hio_lis2dh12_set_alarm(hio_lis2dh12_t *self, hio_lis2dh12_alarm_t *alarm)
{
    if (alarm != NULL)
    {
        // Enable alarm
        self->_alarm_active = true;

        // Disable IRQ first to change the registers
        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, 0x00))
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

        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x32, int1_ths))
        {
            return false;
        }

        uint8_t int1_duration = (uint8_t)(alarm->duration / 1); // /10
        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x33, int1_duration))
        {
            return false;
        }

        // CTRL_REG3
        uint8_t ctrl_reg3 = (1 << 6);
        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x22, ctrl_reg3))
        {
            return false;
        }

        // CTRL_REG6 - invert interrupt
        uint8_t ctrl_reg6 = (1 << 1);
        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x25, ctrl_reg6))
        {
            return false;
        }

        // ctr_reg5
        uint8_t ctrl_reg5 = (0 << 3); // latch interrupt request
        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x24, ctrl_reg5))
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

        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, int_cfg1))
        {
            return false;
        }

        hio_exti_register(HIO_EXTI_LINE_PB6, HIO_EXTI_EDGE_FALLING, _hio_lis2dh12_interrupt, self);
    }
    else
    {
        // Disable alarm
        self->_alarm_active = false;

        if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, 0x00))
        {
            return false;
        }

        hio_exti_unregister(HIO_EXTI_LINE_PB6);
    }

    hio_lis2dh12_measure(self);

    return true;
}

bool hio_lis2dh12_set_resolution(hio_lis2dh12_t *self, hio_lis2dh12_resolution_t resolution)
{
    self->_resolution = resolution;

    self->_state = HIO_LIS2DH12_STATE_INITIALIZE;

    hio_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool hio_lis2dh12_set_scale(hio_lis2dh12_t *self, hio_lis2dh12_scale_t scale)
{
    self->_scale = scale;

    self->_state = HIO_LIS2DH12_STATE_INITIALIZE;

    hio_scheduler_plan_now(self->_task_id_measure);

    return true;
}

static void _hio_lis2dh12_interrupt(hio_exti_line_t line, void *param)
{
    (void) line;

    hio_lis2dh12_t *self = param;

    self->_irq_flag = true;

    hio_lis2dh12_measure(self);
}
