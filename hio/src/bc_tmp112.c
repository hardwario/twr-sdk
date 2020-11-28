#include <bc_tmp112.h>

#define _BC_TMP112_DELAY_RUN 50
#define _BC_TMP112_DELAY_INITIALIZATION 50
#define _BC_TMP112_DELAY_MEASUREMENT 50

static void _bc_tmp112_task_interval(void *param);

static void _bc_tmp112_task_measure(void *param);

void bc_tmp112_init(bc_tmp112_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = bc_scheduler_register(_bc_tmp112_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_tmp112_task_measure, self, _BC_TMP112_DELAY_RUN);

    self->_tick_ready = _BC_TMP112_DELAY_RUN;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);
}

void bc_tmp112_deinit(bc_tmp112_t *self)
{
    bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x01, 0x0180);

    bc_scheduler_unregister(self->_task_id_interval);

    bc_scheduler_unregister(self->_task_id_measure);
}

void bc_tmp112_set_event_handler(bc_tmp112_t *self, void (*event_handler)(bc_tmp112_t *, bc_tmp112_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_tmp112_set_update_interval(bc_tmp112_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_tmp112_measure(self);
    }
}

bool bc_tmp112_measure(bc_tmp112_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

bool bc_tmp112_get_temperature_raw(bc_tmp112_t *self, int16_t *raw)
{
    if (!self->_temperature_valid)
    {
        return false;
    }

    *raw = (int16_t) self->_reg_temperature >> 4;

    return true;
}

bool bc_tmp112_get_temperature_celsius(bc_tmp112_t *self, float *celsius)
{
    int16_t raw;

    if (!bc_tmp112_get_temperature_raw(self, &raw))
    {
        return false;
    }

    *celsius = (float) raw / 16.f;

    return true;
}

bool bc_tmp112_get_temperature_fahrenheit(bc_tmp112_t *self, float *fahrenheit)
{
    float celsius;

    if (!bc_tmp112_get_temperature_celsius(self, &celsius))
    {
        return false;
    }

    *fahrenheit = celsius * 1.8f + 32.f;

    return true;
}

bool bc_tmp112_get_temperature_kelvin(bc_tmp112_t *self, float *kelvin)
{
    float celsius;

    if (!bc_tmp112_get_temperature_celsius(self, &celsius))
    {
        return false;
    }

    *kelvin = celsius + 273.15f;

    if (*kelvin < 0.f)
    {
        *kelvin = 0.f;
    }

    return true;
}

static void _bc_tmp112_task_interval(void *param)
{
    bc_tmp112_t *self = param;

    bc_tmp112_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_tmp112_task_measure(void *param)
{
    bc_tmp112_t *self = param;

start:

    switch (self->_state)
    {
        case BC_TMP112_STATE_ERROR:
        {
            self->_temperature_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_TMP112_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_TMP112_STATE_INITIALIZE;

            return;
        }
        case BC_TMP112_STATE_INITIALIZE:
        {
            self->_state = BC_TMP112_STATE_ERROR;

            if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x01, 0x0180))
            {
                goto start;
            }

            self->_state = BC_TMP112_STATE_MEASURE;

            self->_tick_ready = bc_tick_get() + _BC_TMP112_DELAY_INITIALIZATION;

            if (self->_measurement_active)
            {
                bc_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case BC_TMP112_STATE_MEASURE:
        {
            self->_state = BC_TMP112_STATE_ERROR;

            if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x01, 0x81))
            {
                goto start;
            }

            self->_state = BC_TMP112_STATE_READ;

            bc_scheduler_plan_current_from_now(_BC_TMP112_DELAY_MEASUREMENT);

            return;
        }
        case BC_TMP112_STATE_READ:
        {
            self->_state = BC_TMP112_STATE_ERROR;

            uint8_t reg_configuration;

            if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x01, &reg_configuration))
            {
                goto start;
            }

            if ((reg_configuration & 0x81) != 0x81)
            {
                goto start;
            }

            if (!bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0x00, &self->_reg_temperature))
            {
                goto start;
            }

            self->_temperature_valid = true;

            self->_state = BC_TMP112_STATE_UPDATE;

            goto start;
        }
        case BC_TMP112_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_TMP112_EVENT_UPDATE, self->_event_param);
            }

            self->_state = BC_TMP112_STATE_MEASURE;

            return;
        }
        default:
        {
            self->_state = BC_TMP112_STATE_ERROR;

            goto start;
        }
    }
}
