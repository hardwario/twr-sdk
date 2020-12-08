#include <twr_tag_barometer.h>

void twr_tag_barometer_init(twr_tag_barometer_t *self, twr_i2c_channel_t i2c_channel)
{
    twr_mpl3115a2_init(self, i2c_channel, 0x60);
}

void twr_tag_barometer_set_event_handler(twr_tag_barometer_t *self, void (*event_handler)(twr_tag_barometer_t *, twr_tag_barometer_event_t, void *), void *event_param)
{
    twr_mpl3115a2_set_event_handler(self, (void (*)(twr_mpl3115a2_t *, twr_mpl3115a2_event_t, void *)) event_handler, event_param);
}

void twr_tag_barometer_set_update_interval(twr_tag_barometer_t *self, twr_tick_t interval)
{
    twr_mpl3115a2_set_update_interval(self, interval);
}

bool twr_tag_barometer_measure(twr_tag_barometer_t *self)
{
    return twr_mpl3115a2_measure(self);
}

bool twr_tag_barometer_get_altitude_meter(twr_tag_barometer_t *self, float *meter)
{
    return twr_mpl3115a2_get_altitude_meter(self, meter);
}

bool twr_tag_barometer_get_pressure_pascal(twr_tag_barometer_t *self, float *pascal)
{
    return twr_mpl3115a2_get_pressure_pascal(self, pascal);
}
