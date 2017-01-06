#ifndef _BC_TEMPERATURE_TAG_H
#define _BC_TEMPERATURE_TAG_H

#include <bc_tmp112.h>

typedef enum
{
    BC_TEMPERATURE_TAG_I2C_ADDRESS_DEFAULT = 0x48,
    BC_TEMPERATURE_TAG_I2C_ADDRESS_ALTERNATE = 0x49

} bc_temperature_tag_i2c_address_t;

typedef bc_tmp112_event_t bc_temperature_tag_event_t;

typedef bc_tmp112_t bc_temperature_tag_t;

inline void bc_temperature_tag_init(bc_temperature_tag_t *self, bc_i2c_channel_t i2c_channel, bc_temperature_tag_i2c_address_t i2c_address)
{
    bc_tmp112_init(self, i2c_channel, (uint8_t) i2c_address);
}

inline void bc_temperature_tag_set_event_handler(bc_temperature_tag_t *self, void (*event_handler)(bc_temperature_tag_t *, bc_temperature_tag_event_t))
{
    bc_tmp112_set_event_handler(self, event_handler);
}

inline void bc_temperature_tag_set_update_interval(bc_temperature_tag_t *self, bc_tick_t interval)
{
    bc_tmp112_set_update_interval(self, interval);
}

inline bool bc_temperature_tag_get_temperature_raw(bc_temperature_tag_t *self, int16_t *raw)
{
    return bc_tmp112_get_temperature_raw(self, raw);
}

inline bool bc_temperature_tag_get_temperature_celsius(bc_temperature_tag_t *self, float *celsius)
{
    return bc_tmp112_get_temperature_celsius(self, celsius);
}

inline bool bc_temperature_tag_get_temperature_fahrenheit(bc_temperature_tag_t *self, float *fahrenheit)
{
    return bc_tmp112_get_temperature_fahrenheit(self, fahrenheit);
}

inline bool bc_temperature_tag_get_temperature_kelvin(bc_temperature_tag_t *self, float *kelvin)
{
    return bc_tmp112_get_temperature_kelvin(self, kelvin);
}

#endif // _BC_TEMPERATURE_TAG_H
