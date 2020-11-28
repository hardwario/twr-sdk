#include <hio_module_infra_grid.h>

// Reference registers, commands
// https://na.industrial.panasonic.com/sites/default/pidsa/files/downloads/files/grid-eye-high-performance-specifications.pdf

// Adafruit Lib
// https://github.com/adafruit/Adafruit_AMG88xx/blob/master/Adafruit_AMG88xx.cpp

// interrupt flags
// https://github.com/adafruit/Adafruit_AMG88xx/blob/master/examples/amg88xx_interrupt/amg88xx_interrupt.ino

#define _HIO_AMG88xx_ADDR 0x68 // in 7bit

#define _HIO_AMG88xx_PCLT 0x00
#define _HIO_AMG88xx_RST 0x01
#define _HIO_AMG88xx_FPSC 0x02
#define _HIO_AMG88xx_INTC 0x03
#define _HIO_AMG88xx_STAT 0x04
#define _HIO_AMG88xx_SCLR 0x05
#define _HIO_AMG88xx_AVE 0x07
#define _HIO_AMG88xx_INTHL 0x08
#define _HIO_AMG88xx_TTHL 0x0e
#define _HIO_AMG88xx_TTHH 0x0f
#define _HIO_AMG88xx_INT0 0x10
#define _HIO_AMG88xx_AVG 0x1f
#define _HIO_AMG88xx_T01L 0x80

#define _HIO_MODULE_INFRA_GRID_DELAY_RUN 50
#define _HIO_MODULE_INFRA_GRID_DELAY_INITIALIZATION 50
#define _HIO_MODULE_INFRA_GRID_DELAY_MEASUREMENT 5

#define _HIO_MODULE_INFRA_GRID_DELAY_MODE_CHANGE 50
#define _HIO_MODULE_INFRA_GRID_DELAY_POWER_UP 50
#define _HIO_MODULE_INFRA_GRID_DELAY_INITIAL_RESET 10
#define _HIO_MODULE_INFRA_GRID_DELAY_FLAG_RESET 110

#define _HIO_MODULE_INFRA_GRID_PIN_POWER HIO_TCA9534A_PIN_P7

static void _hio_module_infra_grid_task_interval(void *param);
static void _hio_module_infra_grid_task_measure(void *param);

void hio_module_infra_grid_init(hio_module_infra_grid_t *self)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = HIO_I2C_I2C0;
    self->_i2c_address = _HIO_AMG88xx_ADDR;
    self->_cmd_sleep = true;

    self->_task_id_interval = hio_scheduler_register(_hio_module_infra_grid_task_interval, self, HIO_TICK_INFINITY);
    self->_task_id_measure = hio_scheduler_register(_hio_module_infra_grid_task_measure, self, _HIO_MODULE_INFRA_GRID_DELAY_RUN);

    self->_tick_ready = _HIO_MODULE_INFRA_GRID_DELAY_RUN;
    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_100_KHZ);
}

void hio_module_infra_grid_set_event_handler(hio_module_infra_grid_t *self, void (*event_handler)(hio_module_infra_grid_t *, hio_module_infra_grid_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_module_infra_grid_set_update_interval(hio_module_infra_grid_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval >= 1000)
    {
        self->_cmd_sleep = true;
    }

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);
        hio_module_infra_grid_measure(self);
    }
}

bool hio_module_infra_grid_measure(hio_module_infra_grid_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    hio_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

float hio_module_infra_grid_read_thermistor(hio_module_infra_grid_t *self)
{
    (void) self;
    int8_t temperature[2];

    hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_TTHL, (uint8_t *) &temperature[0]);
    hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_TTHH, (uint8_t *) &temperature[1]);

    return (temperature[0] | temperature[1] << 8) * 0.0625f;
}

bool hio_module_infra_grid_read_values(hio_module_infra_grid_t *self)
{
    hio_i2c_memory_transfer_t transfer;

    transfer.device_address = self->_i2c_address;
    transfer.memory_address = _HIO_AMG88xx_T01L;
    transfer.buffer = self->_sensor_data;
    transfer.length = 64 * 2;

    return hio_i2c_memory_read(self->_i2c_channel, &transfer);
}

bool hio_module_infra_grid_get_temperatures_celsius(hio_module_infra_grid_t *self, float *values)
{
    if (!self->_temperature_valid)
    {
        return false;
    }

    for (int i = 0; i < 64 ;i++)
    {
        float temperature;
        int16_t temporary_data = self->_sensor_data[i];

        if (temporary_data > 0x200)
        {
            temperature = (-temporary_data + 0xfff) * -0.25f;
        }
        else
        {
            temperature = temporary_data * 0.25f;
        }

        values[i] = temperature;
    }

    return true;
}

static void _hio_module_infra_grid_task_interval(void *param)
{
    hio_module_infra_grid_t *self = param;
    hio_module_infra_grid_measure(self);
    hio_scheduler_plan_current_relative(self->_update_interval);
}

static void _hio_module_infra_grid_task_measure(void *param)
{
    hio_module_infra_grid_t *self = param;

    start:

    switch (self->_state)
    {
        case HIO_MODULE_INFRA_GRID_STATE_ERROR:
        {
            self->_temperature_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_MODULE_INFRA_GRID_EVENT_ERROR, self->_event_param);
            }

            self->_state = HIO_MODULE_INFRA_GRID_STATE_INITIALIZE;

            return;
        }
        case HIO_MODULE_INFRA_GRID_STATE_INITIALIZE:
        {
            self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;

            if (hio_tca9534a_init(&self->_tca9534, HIO_I2C_I2C0 , 0x23))
            {
                self->_revision = HIO_MODULE_INFRA_GRID_REVISION_R1_1;
            }
            else
            {
                self->_revision = HIO_MODULE_INFRA_GRID_REVISION_R1_0;
            }

            if (self->_revision == HIO_MODULE_INFRA_GRID_REVISION_R1_1)
            {
                if (!hio_tca9534a_set_pin_direction(&self->_tca9534, _HIO_MODULE_INFRA_GRID_PIN_POWER , HIO_TCA9534A_PIN_DIRECTION_OUTPUT))
                {
                    goto start;
                }
                if (!hio_tca9534a_write_pin(&self->_tca9534, _HIO_MODULE_INFRA_GRID_PIN_POWER , 1))
                {
                    goto start;
                }
            }

            // Update sleep flag
            if (self->_enable_sleep != self->_cmd_sleep)
            {
                self->_enable_sleep = self->_cmd_sleep;
            }

            if (self->_enable_sleep)
            {
                if (self->_revision == HIO_MODULE_INFRA_GRID_REVISION_R1_0)
                {
                    // Sleep mode
                    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_PCLT, 0x10))
                    {
                        goto start;
                    }
                }
                else if (self->_revision == HIO_MODULE_INFRA_GRID_REVISION_R1_1)
                {
                    // Revision 1.1 - Disconnect power to Infra Grid Module
                    if (!hio_tca9534a_write_pin(&self->_tca9534, _HIO_MODULE_INFRA_GRID_PIN_POWER , 0))
                    {
                        goto start;
                    }
                }
            }
            else
            {
                // Set 10 FPS
                if (!hio_i2c_memory_write_8b (self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_FPSC, 0x00))
                {
                    goto start;
                }

                // Diff interrpt mode, INT output reactive
                hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_INTC, 0x00);
                // Moving average output mode active
                hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x50);
                hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x45);
                hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x57);
                hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVE, 0x20);
                hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x00);
            }

            self->_state = HIO_MODULE_INFRA_GRID_STATE_MODE_CHANGE; //HIO_MODULE_INFRA_GRID_STATE_MEASURE;

            self->_tick_ready = hio_tick_get() + _HIO_MODULE_INFRA_GRID_DELAY_INITIALIZATION;

            if (self->_measurement_active)
            {
                hio_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case HIO_MODULE_INFRA_GRID_STATE_MODE_CHANGE:
        {
            // Skip wakeup commands in case of fast reading
            if (!self->_enable_sleep)
            {
                self->_state = HIO_MODULE_INFRA_GRID_STATE_READ;
                goto start;
            }

            if (self->_revision == HIO_MODULE_INFRA_GRID_REVISION_R1_0)
            {
                // Revision 1.0 - The module is already powered up
                self->_state = HIO_MODULE_INFRA_GRID_STATE_POWER_UP;
                goto start;
            }
            else if (self->_revision == HIO_MODULE_INFRA_GRID_REVISION_R1_1)
            {
                // Revision 1.1 - Enable power for Infra Grid Module
                if (!hio_tca9534a_write_pin(&self->_tca9534, _HIO_MODULE_INFRA_GRID_PIN_POWER , 1))
                {
                    self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;
                    goto start;
                }
                self->_state = HIO_MODULE_INFRA_GRID_STATE_POWER_UP;
                hio_scheduler_plan_current_from_now(_HIO_MODULE_INFRA_GRID_DELAY_POWER_UP);
            }

            return;
        }
        case HIO_MODULE_INFRA_GRID_STATE_POWER_UP:
        {
            self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;

            // Normal Mode
            if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_PCLT, 0x00))
            {
                goto start;
            }
            self->_state = HIO_MODULE_INFRA_GRID_STATE_INITIAL_RESET;
            hio_scheduler_plan_current_from_now(_HIO_MODULE_INFRA_GRID_DELAY_MODE_CHANGE);
            return;
        }
        case HIO_MODULE_INFRA_GRID_STATE_INITIAL_RESET:
        {
            self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;

            // Write initial reset
            if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_RST, 0x3f))
            {
                goto start;
            }

            self->_state = HIO_MODULE_INFRA_GRID_STATE_FLAG_RESET;
            hio_scheduler_plan_current_from_now(_HIO_MODULE_INFRA_GRID_DELAY_INITIAL_RESET);
            return;
        }
        case HIO_MODULE_INFRA_GRID_STATE_FLAG_RESET:
        {
            self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;

            // Write flag reset
            if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_RST, 0x30))
            {
                goto start;
            }

            self->_state = HIO_MODULE_INFRA_GRID_STATE_MEASURE;
            hio_scheduler_plan_current_from_now(_HIO_MODULE_INFRA_GRID_DELAY_FLAG_RESET);
            return;
        }
        case HIO_MODULE_INFRA_GRID_STATE_MEASURE:
        {
            self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;

            if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_FPSC, 0x01))
            {
                goto start;
            }

            // Diff interrpt mode, INT output reactive
            hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_INTC, 0x00);
            // Moving average output mode active
            hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x50);
            hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x45);
            hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x57);
            hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVE, 0x20);
            hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_AVG, 0x00);

            self->_state = HIO_MODULE_INFRA_GRID_STATE_READ;

            hio_scheduler_plan_current_from_now(_HIO_MODULE_INFRA_GRID_DELAY_MEASUREMENT);

            return;
        }
        case HIO_MODULE_INFRA_GRID_STATE_READ:
        {
            self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;

            hio_i2c_memory_transfer_t transfer;
            transfer.device_address = self->_i2c_address;
            transfer.memory_address = _HIO_AMG88xx_T01L;
            transfer.buffer = self->_sensor_data;
            transfer.length = 64 * 2;

            if (!hio_i2c_memory_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            // Enable sleep mode
            if (self->_enable_sleep)
            {
                // Sleep Mode
                if (self->_revision == HIO_MODULE_INFRA_GRID_REVISION_R1_0)
                {
                    // Sleep mode
                    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_AMG88xx_PCLT, 0x10))
                    {
                        goto start;
                    }
                }
                else if (self->_revision == HIO_MODULE_INFRA_GRID_REVISION_R1_1)
                {
                    // Revision 1.1 has power disconnect
                    hio_tca9534a_write_pin(&self->_tca9534, _HIO_MODULE_INFRA_GRID_PIN_POWER , 0);
                }
            }

            self->_temperature_valid = true;
            self->_state = HIO_MODULE_INFRA_GRID_STATE_UPDATE;

            goto start;
        }
        case HIO_MODULE_INFRA_GRID_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_MODULE_INFRA_GRID_EVENT_UPDATE, self->_event_param);
            }

            // Update sleep flag
            if (self->_enable_sleep != self->_cmd_sleep)
            {
                self->_enable_sleep = self->_cmd_sleep;
            }

            self->_state = HIO_MODULE_INFRA_GRID_STATE_MODE_CHANGE;
            return;
        }
        default:
        {
            self->_state = HIO_MODULE_INFRA_GRID_STATE_ERROR;
            goto start;
        }
    }
}

hio_module_infra_grid_revision_t hio_module_infra_grid_get_revision(hio_module_infra_grid_t *self)
{
    return self->_revision;
}
