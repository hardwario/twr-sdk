#ifndef _BC_BAROMETER_TAG_H
#define _BC_BAROMETER_TAG_H

#include <bc_mpl3115a2.h>

typedef bc_mpl3115a2_event_t bc_barometer_tag_event_t;

typedef bc_mpl3115a2_t bc_barometer_tag_t;

inline void bc_barometer_tag_init(bc_barometer_tag_t *self, bc_i2c_channel_t i2c_channel)
{
    bc_mpl3115a2_init(self, i2c_channel, 0x60);
}

inline void bc_barometer_tag_set_event_handler(bc_barometer_tag_t *self, void (*event_handler)(bc_barometer_tag_t *, bc_barometer_tag_event_t))
{
    bc_mpl3115a2_set_event_handler(self, event_handler);
}

inline void bc_barometer_tag_set_update_interval(bc_barometer_tag_t *self, bc_tick_t interval)
{
    bc_mpl3115a2_set_update_interval(self, interval);
}

inline bool bc_barometer_tag_get_altitude_meter(bc_barometer_tag_t *self, float *meter)
{
    return bc_mpl3115a2_get_altitude_meter(self, meter);
}

inline bool bc_barometer_tag_get_pressure_pascal(bc_barometer_tag_t *self, float *pascal)
{
    return bc_mpl3115a2_get_pressure_pascal(self, pascal);
}

#endif // _BC_BAROMETER_TAG_H
