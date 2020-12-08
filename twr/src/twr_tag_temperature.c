#include <twr_tag_temperature.h>

void twr_tag_temperature_init(twr_tag_temperature_t *self, twr_i2c_channel_t i2c_channel, twr_tag_temperature_i2c_address_t i2c_address)
{
    twr_tmp112_init(self, i2c_channel, (uint8_t) i2c_address);
}

void twr_tag_temperature_set_event_handler(twr_tag_temperature_t *self, void (*event_handler)(twr_tag_temperature_t *, twr_tag_temperature_event_t, void *), void *event_param)
{
    twr_tmp112_set_event_handler(self, (void (*)(twr_tmp112_t *, twr_tmp112_event_t, void *)) event_handler, event_param);
}

void twr_tag_temperature_set_update_interval(twr_tag_temperature_t *self, twr_tick_t interval)
{
    twr_tmp112_set_update_interval(self, interval);
}

bool twr_tag_temperature_measure(twr_tag_temperature_t *self)
{
    return twr_tmp112_measure(self);
}

bool twr_tag_temperature_get_temperature_raw(twr_tag_temperature_t *self, int16_t *raw)
{
    return twr_tmp112_get_temperature_raw(self, raw);
}

bool twr_tag_temperature_get_temperature_celsius(twr_tag_temperature_t *self, float *celsius)
{
    return twr_tmp112_get_temperature_celsius(self, celsius);
}

bool twr_tag_temperature_get_temperature_fahrenheit(twr_tag_temperature_t *self, float *fahrenheit)
{
    return twr_tmp112_get_temperature_fahrenheit(self, fahrenheit);
}

bool twr_tag_temperature_get_temperature_kelvin(twr_tag_temperature_t *self, float *kelvin)
{
    return twr_tmp112_get_temperature_kelvin(self, kelvin);
}
