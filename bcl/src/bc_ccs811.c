#include <bc_ccs811.h>
#include <bc_spi.h>

#define _BC_CCS811_DELAY_RESTART 100
#define _BC_CCS811_DELAY_MEASURE 2000

#define _BC_CCS811_DEFAULT_BASELINE 58483

#define _BC_CCS811_WAKE_ON() bc_gpio_set_output(BC_GPIO_P15, 0)
#define _BC_CCS811_WAKE_OFF() bc_gpio_set_output(BC_GPIO_P15, 1)

static void _bc_ccs811_task_interval(void *param);

static void _bc_ccs811_task_measure(void *param);

static bool _bc_ccs811_write_cmd(bc_ccs811_t *self, const void *cmd, size_t len);

void bc_ccs811_init(bc_ccs811_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_baseline = _BC_CCS811_DEFAULT_BASELINE;

    self->_task_id_interval = bc_scheduler_register(_bc_ccs811_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_ccs811_task_measure, self, BC_TICK_INFINITY);

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_100_KHZ);

    // TODO Just for Sensation
    bc_spi_init(BC_SPI_SPEED_1_MHZ, BC_SPI_MODE_0);
}

void bc_ccs811_set_event_handler(bc_ccs811_t *self, void (*event_handler)(bc_ccs811_t *, bc_ccs811_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_ccs811_set_update_interval(bc_ccs811_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_ccs811_measure(self);
    }
}

void bc_ccs811_get_baseline(bc_ccs811_t *self, uint16_t *baseline)
{
    *baseline = self->_baseline;
}

void bc_ccs811_set_baseline(bc_ccs811_t *self, uint16_t baseline)
{
        self->_baseline = baseline;
}

bool bc_ccs811_measure(bc_ccs811_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool bc_ccs811_get_co2_ppm(bc_ccs811_t *self, float *ppm)
{
    if (!self->_co2_valid)
    {
        return false;
    }

    *ppm = self->_reg_co2;

    return true;
}

static void _bc_ccs811_task_interval(void *param)
{
    bc_ccs811_t *self = param;

    bc_ccs811_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_ccs811_task_measure(void *param)
{
    bc_ccs811_t *self = param;

    while (1)
    {
        switch (self->_state)
        {
            case BC_CCS811_STATE_ERROR:
            {
                self->_co2_valid = false;

                self->_measurement_active = false;

                // TODO Just for Sensation
                bc_spi_unlock();

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CCS811_EVENT_ERROR, self->_event_param);
                }

                self->_state = BC_CCS811_STATE_RESTART;

                return;
            }
            case BC_CCS811_STATE_RESTART:
            {
                static const uint8_t _bc_ccs811_cmd_reset[5] = { 0xff, 0x11, 0xe5, 0x72, 0x8a };

                // TODO Just for Sensation
                if (!bc_spi_lock())
                {
                    bc_scheduler_plan_current_absolute(bc_tick_get() + 100);

                    return;
                }

                _BC_CCS811_WAKE_ON();

                // Reset sensor
                _bc_ccs811_write_cmd(self, _bc_ccs811_cmd_reset, sizeof(_bc_ccs811_cmd_reset));

                self->_state = BC_CCS811_STATE_INITIALIZE;

                bc_scheduler_plan_current_absolute(bc_tick_get() + _BC_CCS811_DELAY_RESTART);

                return;
            }
            case BC_CCS811_STATE_INITIALIZE:
            {
                self->_state = BC_CCS811_STATE_ERROR;

                uint8_t reg;
                static const uint8_t _bc_ccs811_cmd_start_app[1] = { 0xf4 };

                // Read HW_ID ... != 0x81 ? error
                if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x20, &reg) || (reg != 0x81))
                {
                    continue;
                }

                // Read status ... APP_VALID != 1 ? error
                if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x00, &reg) || (!(reg & 0x10)))
                {
                    continue;
                }

                // Write APP_START
                _bc_ccs811_write_cmd(self, _bc_ccs811_cmd_start_app, sizeof(_bc_ccs811_cmd_start_app));

                // read status ... FW_MODE != 1 ? error
                if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x00, &reg) || (!(reg & 0x80)))
                {
                    continue;
                }

                self->_state = BC_CCS811_STATE_MEAS_START;

                continue;
            }
            case BC_CCS811_STATE_MEAS_START:
            {
                self->_state = BC_CCS811_STATE_ERROR;

                _BC_CCS811_WAKE_ON();

                // Write MEAS_MODE to update every 1s
                if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x01, 0x10))
                {
                    continue;
                }

                // Update baseline
                if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x11, self->_baseline))
                {
                    continue;
                }

                self->_state = BC_CCS811_STATE_MEAS;

                continue;
            }
            case BC_CCS811_STATE_MEAS:
            {
                bc_scheduler_plan_current_absolute(bc_tick_get() + _BC_CCS811_DELAY_MEASURE);

                self->_state = BC_CCS811_STATE_READ;

                return;
            }
            case BC_CCS811_STATE_READ:
            {
                self->_state = BC_CCS811_STATE_ERROR;

                uint8_t reg;
                uint16_t buffer;

                // read status
                if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x00, &reg))
                {
                    continue;
                }

                // If DATA_READY != 1 ? data not ready yet ... try it later
                if (!(reg & 0x08))
                {
                    self->_state = BC_CCS811_STATE_READ;

                    bc_scheduler_plan_current_relative(100);

                    return;
                }

                // Read co2 register
                if (!bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0x02, &buffer))
                {
                    continue;
                }

                self->_reg_co2 = buffer;
                self->_co2_valid = true;

                if (self->_reg_co2 == 0)
                {
                    self->_state = BC_CCS811_STATE_MEAS;
                }
                else
                {
                    self->_state = BC_CCS811_STATE_UPDATE;

                    // Write MEAS_MODE to idle (power-down)
                    if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x01, 0x00))
                    {
                        continue;
                    }

                    _BC_CCS811_WAKE_OFF();

                    // TODO Just for Sensation
                    bc_spi_unlock();
                }

                continue;
            }
            case BC_CCS811_STATE_UPDATE:
            {
                self->_state = BC_CCS811_STATE_ERROR;

                self->_measurement_active = false;

                self->_event_handler(self, BC_CCS811_EVENT_UPDATE, self->_event_param);

                self->_state = BC_CCS811_STATE_MEAS_START;

                return;
            }
            default:
            {
                self->_state = BC_CCS811_STATE_ERROR;

                continue;
            }
        }
    }
}

static bool _bc_ccs811_write_cmd(bc_ccs811_t *self, const void *cmd, size_t len)
{
    bc_i2c_transfer_t t;
    t.device_address = self->_i2c_address;
    t.length = len;
    t.buffer = (void *)cmd;

    return bc_i2c_write(self->_i2c_channel, &t);
}
