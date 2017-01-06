#ifndef _BC_LUX_METER_TAG_H
#define _BC_LUX_METER_TAG_H

#include <bc_opt3001.h>

typedef enum
{
    BC_LUX_METER_TAG_I2C_ADDRESS_DEFAULT = 0x44,
    BC_LUX_METER_TAG_I2C_ADDRESS_ALTERNATE = 0x45

} bc_lux_meter_tag_i2c_address_t;

typedef bc_opt3001_event_t bc_lux_meter_tag_event_t;

typedef bc_opt3001_t bc_lux_meter_tag_t;

inline void bc_lux_meter_tag_init(bc_lux_meter_tag_t *self, bc_i2c_channel_t i2c_channel, bc_lux_meter_tag_i2c_address_t i2c_address)
{
    bc_opt3001_init(self, i2c_channel, (uint8_t) i2c_address);
}

inline void bc_lux_meter_tag_set_event_handler(bc_lux_meter_tag_t *self, void (*event_handler)(bc_lux_meter_tag_t *, bc_lux_meter_tag_event_t))
{
    bc_opt3001_set_event_handler(self, event_handler);
}

inline void bc_lux_meter_tag_set_update_interval(bc_lux_meter_tag_t *self, bc_tick_t interval)
{
    bc_opt3001_set_update_interval(self, interval);
}

inline bool bc_lux_meter_tag_get_luminosity_raw(bc_lux_meter_tag_t *self, uint16_t *raw)
{
    return bc_opt3001_get_luminosity_raw(self, raw);
}

inline bool bc_lux_meter_tag_get_luminosity_lux(bc_lux_meter_tag_t *self, float *lux)
{
    return bc_opt3001_get_luminosity_lux(self, lux);
}

#endif // _BC_LUX_METER_TAG_H
