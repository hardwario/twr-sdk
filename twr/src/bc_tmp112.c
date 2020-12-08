#include <twr_tmp112.h>

#define _TWR_TMP112_DELAY_RUN 50
#define _TWR_TMP112_DELAY_INITIALIZATION 50
#define _TWR_TMP112_DELAY_MEASUREMENT 50

static void _twr_tmp112_task_interval(void *param);

static void _twr_tmp112_task_measure(void *param);

void twr_tmp112_init(twr_tmp112_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = twr_scheduler_register(_twr_tmp112_task_interval, self, TWR_TICK_INFINITY);
    self->_task_id_measure = twr_scheduler_register(_twr_tmp112_task_measure, self, _TWR_TMP112_DELAY_RUN);

    self->_tick_ready = _TWR_TMP112_DELAY_RUN;

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);
}

void twr_tmp112_deinit(twr_tmp112_t *self)
{
    twr_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x01, 0x0180);

    twr_scheduler_unregister(self->_task_id_interval);

    twr_scheduler_unregister(self->_task_id_measure);
}

void twr_tmp112_set_event_handler(twr_tmp112_t *self, void (*event_handler)(twr_tmp112_t *, twr_tmp112_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_tmp112_set_update_interval(twr_tmp112_t *self, twr_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(self->_task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        twr_tmp112_measure(self);
    }
}

bool twr_tmp112_measure(twr_tmp112_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    twr_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

bool twr_tmp112_get_temperature_raw(twr_tmp112_t *self, int16_t *raw)
{
    if (!self->_temperature_valid)
    {
        return false;
    }

    *raw = (int16_t) self->_reg_temperature >> 4;

    return true;
}

bool twr_tmp112_get_temperature_celsius(twr_tmp112_t *self, float *celsius)
{
    int16_t raw;

    if (!twr_tmp112_get_temperature_raw(self, &raw))
    {
        return false;
    }

    *celsius = (float) raw / 16.f;

    return true;
}

bool twr_tmp112_get_temperature_fahrenheit(twr_tmp112_t *self, float *fahrenheit)
{
    float celsius;

    if (!twr_tmp112_get_temperature_celsius(self, &celsius))
    {
        return false;
    }

    *fahrenheit = celsius * 1.8f + 32.f;

    return true;
}

bool twr_tmp112_get_temperature_kelvin(twr_tmp112_t *self, float *kelvin)
{
    float celsius;

    if (!twr_tmp112_get_temperature_celsius(self, &celsius))
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

static void _twr_tmp112_task_interval(void *param)
{
    twr_tmp112_t *self = param;

    twr_tmp112_measure(self);

    twr_scheduler_plan_current_relative(self->_update_interval);
}

static void _twr_tmp112_task_measure(void *param)
{
    twr_tmp112_t *self = param;

start:

    switch (self->_state)
    {
        case TWR_TMP112_STATE_ERROR:
        {
            self->_temperature_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_TMP112_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_TMP112_STATE_INITIALIZE;

            return;
        }
        case TWR_TMP112_STATE_INITIALIZE:
        {
            self->_state = TWR_TMP112_STATE_ERROR;

            if (!twr_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x01, 0x0180))
            {
                goto start;
            }

            self->_state = TWR_TMP112_STATE_MEASURE;

            self->_tick_ready = twr_tick_get() + _TWR_TMP112_DELAY_INITIALIZATION;

            if (self->_measurement_active)
            {
                twr_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case TWR_TMP112_STATE_MEASURE:
        {
            self->_state = TWR_TMP112_STATE_ERROR;

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x01, 0x81))
            {
                goto start;
            }

            self->_state = TWR_TMP112_STATE_READ;

            twr_scheduler_plan_current_from_now(_TWR_TMP112_DELAY_MEASUREMENT);

            return;
        }
        case TWR_TMP112_STATE_READ:
        {
            self->_state = TWR_TMP112_STATE_ERROR;

            uint8_t reg_configuration;

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x01, &reg_configuration))
            {
                goto start;
            }

            if ((reg_configuration & 0x81) != 0x81)
            {
                goto start;
            }

            if (!twr_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0x00, &self->_reg_temperature))
            {
                goto start;
            }

            self->_temperature_valid = true;

            self->_state = TWR_TMP112_STATE_UPDATE;

            goto start;
        }
        case TWR_TMP112_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_TMP112_EVENT_UPDATE, self->_event_param);
            }

            self->_state = TWR_TMP112_STATE_MEASURE;

            return;
        }
        default:
        {
            self->_state = TWR_TMP112_STATE_ERROR;

            goto start;
        }
    }
}
