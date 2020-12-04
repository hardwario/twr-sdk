#include <bc_opt3001.h>

#define _BC_OPT3001_DELAY_RUN 50
#define _BC_OPT3001_DELAY_INITIALIZATION 50
#define _BC_OPT3001_DELAY_MEASUREMENT 1000

static void _bc_opt3001_task_interval(void *param);

static void _bc_opt3001_task_measure(void *param);

void bc_opt3001_init(bc_opt3001_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = bc_scheduler_register(_bc_opt3001_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_opt3001_task_measure, self, _BC_OPT3001_DELAY_RUN);

    self->_tick_ready = _BC_OPT3001_DELAY_RUN;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);
}

void bc_opt3001_set_event_handler(bc_opt3001_t *self, void (*event_handler)(bc_opt3001_t *, bc_opt3001_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_opt3001_set_update_interval(bc_opt3001_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_opt3001_measure(self);
    }
}

bool bc_opt3001_measure(bc_opt3001_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

bool bc_opt3001_get_illuminance_raw(bc_opt3001_t *self, uint16_t *raw)
{
    if (!self->_illuminance_valid)
    {
        return false;
    }

    *raw = self->_reg_result;

    return true;
}

bool bc_opt3001_get_illuminance_lux(bc_opt3001_t *self, float *lux)
{
    uint16_t raw;

    if (!bc_opt3001_get_illuminance_raw(self, &raw))
    {
        return false;
    }

    *lux = 0.01f * (float) (1 << (raw >> 12)) * (float) (raw & 0xfff);

    return true;
}

static void _bc_opt3001_task_interval(void *param)
{
    bc_opt3001_t *self = param;

    bc_opt3001_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_opt3001_task_measure(void *param)
{
    bc_opt3001_t *self = param;

start:

    switch (self->_state)
    {
        case BC_OPT3001_STATE_ERROR:
        {
            self->_illuminance_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_OPT3001_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_OPT3001_STATE_INITIALIZE;

            return;
        }
        case BC_OPT3001_STATE_INITIALIZE:
        {
            self->_state = BC_OPT3001_STATE_ERROR;

            if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x01, 0xc810))
            {
                goto start;
            }

            self->_state = BC_OPT3001_STATE_MEASURE;

            self->_tick_ready = bc_tick_get() + _BC_OPT3001_DELAY_INITIALIZATION;

            if (self->_measurement_active)
            {
                bc_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case BC_OPT3001_STATE_MEASURE:
        {
            self->_state = BC_OPT3001_STATE_ERROR;

            if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x01, 0xca10))
            {
                goto start;
            }

            self->_state = BC_OPT3001_STATE_READ;

            bc_scheduler_plan_current_from_now(_BC_OPT3001_DELAY_MEASUREMENT);

            return;
        }
        case BC_OPT3001_STATE_READ:
        {
            self->_state = BC_OPT3001_STATE_ERROR;

            uint16_t reg_configuration;

            if (!bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0x01, &reg_configuration))
            {
                goto start;
            }

            if ((reg_configuration & 0x0680) != 0x0080)
            {
                goto start;
            }

            if (!bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0x00, &self->_reg_result))
            {
                goto start;
            }

            self->_illuminance_valid = true;

            self->_state = BC_OPT3001_STATE_UPDATE;

            goto start;
        }
        case BC_OPT3001_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_OPT3001_EVENT_UPDATE, self->_event_param);
            }

            self->_state = BC_OPT3001_STATE_MEASURE;

            return;
        }
        default:
        {
            self->_state = BC_OPT3001_STATE_ERROR;

            goto start;
        }
    }
}
