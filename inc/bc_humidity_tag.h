#ifndef _BC_HUMIDITY_TAG_H
#define _BC_HUMIDITY_TAG_H

#include <bc_hdc2080.h>

typedef enum
{
    BC_HUMIDITY_TAG_I2C_ADDRESS_DEFAULT = 0x40,
    BC_HUMIDITY_TAG_I2C_ADDRESS_ALTERNATE = 0x41

} bc_humidity_tag_i2c_address_t;

typedef bc_hdc2080_event_t bc_humidity_tag_event_t;

typedef bc_hdc2080_t bc_humidity_tag_t;

inline void bc_humidity_tag_init(bc_humidity_tag_t *self, bc_i2c_channel_t i2c_channel, bc_humidity_tag_i2c_address_t i2c_address)
{
    bc_hdc2080_init(self, i2c_channel, (uint8_t) i2c_address);
}

inline void bc_humidity_tag_set_event_handler(bc_humidity_tag_t *self, void (*event_handler)(bc_humidity_tag_t *, bc_humidity_tag_event_t))
{
    bc_hdc2080_set_event_handler(self, event_handler);
}

inline void bc_humidity_tag_set_update_interval(bc_humidity_tag_t *self, bc_tick_t interval)
{
    bc_hdc2080_set_update_interval(self, interval);
}

inline bool bc_humidity_tag_get_humidity_raw(bc_humidity_tag_t *self, uint16_t *raw)
{
    return bc_hdc2080_get_humidity_raw(self, raw);
}

inline bool bc_humidity_tag_get_humidity_percentage(bc_humidity_tag_t *self, float *percentage)
{
    return bc_hdc2080_get_humidity_percentage(self, percentage);
}

#endif // _BC_HUMIDITY_TAG_H
