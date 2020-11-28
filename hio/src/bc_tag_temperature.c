#include <bc_tag_temperature.h>

void bc_tag_temperature_init(bc_tag_temperature_t *self, bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address)
{
    bc_tmp112_init(self, i2c_channel, (uint8_t) i2c_address);
}

void bc_tag_temperature_set_event_handler(bc_tag_temperature_t *self, void (*event_handler)(bc_tag_temperature_t *, bc_tag_temperature_event_t, void *), void *event_param)
{
    bc_tmp112_set_event_handler(self, (void (*)(bc_tmp112_t *, bc_tmp112_event_t, void *)) event_handler, event_param);
}

void bc_tag_temperature_set_update_interval(bc_tag_temperature_t *self, bc_tick_t interval)
{
    bc_tmp112_set_update_interval(self, interval);
}

bool bc_tag_temperature_measure(bc_tag_temperature_t *self)
{
    return bc_tmp112_measure(self);
}

bool bc_tag_temperature_get_temperature_raw(bc_tag_temperature_t *self, int16_t *raw)
{
    return bc_tmp112_get_temperature_raw(self, raw);
}

bool bc_tag_temperature_get_temperature_celsius(bc_tag_temperature_t *self, float *celsius)
{
    return bc_tmp112_get_temperature_celsius(self, celsius);
}

bool bc_tag_temperature_get_temperature_fahrenheit(bc_tag_temperature_t *self, float *fahrenheit)
{
    return bc_tmp112_get_temperature_fahrenheit(self, fahrenheit);
}

bool bc_tag_temperature_get_temperature_kelvin(bc_tag_temperature_t *self, float *kelvin)
{
    return bc_tmp112_get_temperature_kelvin(self, kelvin);
}
