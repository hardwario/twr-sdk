#include <hio_sht30.h>

#define _HIO_SHT30_DELAY_RUN 20
#define _HIO_SHT30_DELAY_INITIALIZATION 20
#define _HIO_SHT30_DELAY_MEASUREMENT 20

static void _hio_sht30_task_interval(void *param);

static void _hio_sht30_task_measure(void *param);

static bool _hio_sht30_write(hio_sht30_t *self, const uint16_t data);

void hio_sht30_init(hio_sht30_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = hio_scheduler_register(_hio_sht30_task_interval, self, HIO_TICK_INFINITY);
    self->_task_id_measure = hio_scheduler_register(_hio_sht30_task_measure, self, _HIO_SHT30_DELAY_RUN);

    self->_tick_ready = _HIO_SHT30_DELAY_RUN;

    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);
}

void hio_sht30_deinit(hio_sht30_t *self)
{
    _hio_sht30_write(self, 0xa230);
    hio_scheduler_unregister(self->_task_id_interval);
    hio_scheduler_unregister(self->_task_id_measure);
}

void hio_sht30_set_event_handler(hio_sht30_t *self, void (*event_handler)(hio_sht30_t *, hio_sht30_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_sht30_set_update_interval(hio_sht30_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        hio_sht30_measure(self);
    }
}

bool hio_sht30_measure(hio_sht30_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    hio_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

bool hio_sht30_get_humidity_raw(hio_sht30_t *self, uint16_t *raw)
{
    if (!self->_humidity_valid)
    {
        return false;
    }

    *raw = self->_reg_humidity;

    return true;
}

bool hio_sht30_get_humidity_percentage(hio_sht30_t *self, float *percentage)
{
    uint16_t raw;

    if (!hio_sht30_get_humidity_raw(self, &raw))
    {
        return false;
    }

    *percentage = 100.f * (float) raw / (65536.f - 1.f);

    if (*percentage >= 100.f)
    {
        *percentage = 100.f;
    }
    else if (*percentage <= 0.f)
    {
        *percentage = 0.f;
    }

    return true;
}

bool hio_sht30_get_temperature_raw(hio_sht30_t *self, uint16_t *raw)
{
    if (!self->_temperature_valid)
    {
        return false;
    }

    *raw = self->_reg_temperature;

    return true;
}

bool hio_sht30_get_temperature_celsius(hio_sht30_t *self, float *celsius)
{
    uint16_t raw;

    if (!hio_sht30_get_temperature_raw(self, &raw))
    {
        return false;
    }

    *celsius = -45.f + (175.f * (float) raw / (65535.f));

    return true;
}

bool hio_sht30_get_temperature_fahrenheit(hio_sht30_t *self, float *fahrenheit)
{
    float celsius;

    if (!hio_sht30_get_temperature_celsius(self, &celsius))
    {
        return false;
    }

    *fahrenheit = celsius * 1.8f + 32.f;

    return true;
}

bool hio_sht30_get_temperature_kelvin(hio_sht30_t *self, float *kelvin)
{
    float celsius;

    if (!hio_sht30_get_temperature_celsius(self, &celsius))
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

static void _hio_sht30_task_interval(void *param)
{
    hio_sht30_t *self = param;

    hio_sht30_measure(self);

    hio_scheduler_plan_current_relative(self->_update_interval);
}

static void _hio_sht30_task_measure(void *param)
{
    hio_sht30_t *self = param;

start:

    switch (self->_state)
    {
        case HIO_SHT30_STATE_ERROR:
        {
            self->_humidity_valid = false;
            self->_temperature_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_SHT30_EVENT_ERROR, self->_event_param);
            }

            self->_state = HIO_SHT30_STATE_INITIALIZE;

            return;
        }
        case HIO_SHT30_STATE_INITIALIZE:
        {
            self->_state = HIO_SHT30_STATE_ERROR;

            if (!_hio_sht30_write(self, 0xa230))
            {
                goto start;
            }

            self->_state = HIO_SHT30_STATE_MEASURE;

            self->_tick_ready = hio_tick_get() + _HIO_SHT30_DELAY_INITIALIZATION;

            if (self->_measurement_active)
            {
                hio_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case HIO_SHT30_STATE_MEASURE:
        {
            self->_state = HIO_SHT30_STATE_ERROR;

            if (!_hio_sht30_write(self, 0x0d2c))
            {
                goto start;
            }

            self->_state = HIO_SHT30_STATE_READ;

            hio_scheduler_plan_current_from_now(_HIO_SHT30_DELAY_MEASUREMENT);

            return;
        }
        case HIO_SHT30_STATE_READ:
        {
            self->_state = HIO_SHT30_STATE_ERROR;

            uint8_t buffer[6];

            hio_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!hio_i2c_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_reg_humidity = buffer[3] << 8 | buffer[4];
            self->_reg_temperature = buffer[0] << 8 | buffer[1];

            self->_humidity_valid = true;
            self->_temperature_valid = true;

            self->_state = HIO_SHT30_STATE_UPDATE;

            goto start;
        }
        case HIO_SHT30_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_SHT30_EVENT_UPDATE, self->_event_param);
            }

            self->_state = HIO_SHT30_STATE_MEASURE;

            return;
        }
        default:
        {
            self->_state = HIO_SHT30_STATE_ERROR;

            goto start;
        }
    }
}

static bool _hio_sht30_write(hio_sht30_t *self, const uint16_t data)
{
    hio_i2c_transfer_t transfer;

    transfer.device_address = self->_i2c_address;
    transfer.buffer = (void *) &data;
    transfer.length = sizeof(data);

    return hio_i2c_write(self->_i2c_channel, &transfer);
}
