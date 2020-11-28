#include <hio_tag_temperature.h>

void hio_tag_temperature_init(hio_tag_temperature_t *self, hio_i2c_channel_t i2c_channel, hio_tag_temperature_i2c_address_t i2c_address)
{
    hio_tmp112_init(self, i2c_channel, (uint8_t) i2c_address);
}

void hio_tag_temperature_set_event_handler(hio_tag_temperature_t *self, void (*event_handler)(hio_tag_temperature_t *, hio_tag_temperature_event_t, void *), void *event_param)
{
    hio_tmp112_set_event_handler(self, (void (*)(hio_tmp112_t *, hio_tmp112_event_t, void *)) event_handler, event_param);
}

void hio_tag_temperature_set_update_interval(hio_tag_temperature_t *self, hio_tick_t interval)
{
    hio_tmp112_set_update_interval(self, interval);
}

bool hio_tag_temperature_measure(hio_tag_temperature_t *self)
{
    return hio_tmp112_measure(self);
}

bool hio_tag_temperature_get_temperature_raw(hio_tag_temperature_t *self, int16_t *raw)
{
    return hio_tmp112_get_temperature_raw(self, raw);
}

bool hio_tag_temperature_get_temperature_celsius(hio_tag_temperature_t *self, float *celsius)
{
    return hio_tmp112_get_temperature_celsius(self, celsius);
}

bool hio_tag_temperature_get_temperature_fahrenheit(hio_tag_temperature_t *self, float *fahrenheit)
{
    return hio_tmp112_get_temperature_fahrenheit(self, fahrenheit);
}

bool hio_tag_temperature_get_temperature_kelvin(hio_tag_temperature_t *self, float *kelvin)
{
    return hio_tmp112_get_temperature_kelvin(self, kelvin);
}
