#include <bc_tag_barometer.h>

void bc_tag_barometer_init(bc_tag_barometer_t *self, bc_i2c_channel_t i2c_channel)
{
    bc_mpl3115a2_init(self, i2c_channel, 0x60);
}

void bc_tag_barometer_set_event_handler(bc_tag_barometer_t *self, void (*event_handler)(bc_tag_barometer_t *, bc_tag_barometer_event_t, void *), void *event_param)
{
    bc_mpl3115a2_set_event_handler(self, (void (*)(bc_mpl3115a2_t *, bc_mpl3115a2_event_t, void *)) event_handler, event_param);
}

void bc_tag_barometer_set_update_interval(bc_tag_barometer_t *self, bc_tick_t interval)
{
    bc_mpl3115a2_set_update_interval(self, interval);
}

bool bc_tag_barometer_measure(bc_tag_barometer_t *self)
{
    return bc_mpl3115a2_measure(self);
}

bool bc_tag_barometer_get_altitude_meter(bc_tag_barometer_t *self, float *meter)
{
    return bc_mpl3115a2_get_altitude_meter(self, meter);
}

bool bc_tag_barometer_get_pressure_pascal(bc_tag_barometer_t *self, float *pascal)
{
    return bc_mpl3115a2_get_pressure_pascal(self, pascal);
}
