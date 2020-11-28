#include <hio_tag_barometer.h>

void hio_tag_barometer_init(hio_tag_barometer_t *self, hio_i2c_channel_t i2c_channel)
{
    hio_mpl3115a2_init(self, i2c_channel, 0x60);
}

void hio_tag_barometer_set_event_handler(hio_tag_barometer_t *self, void (*event_handler)(hio_tag_barometer_t *, hio_tag_barometer_event_t, void *), void *event_param)
{
    hio_mpl3115a2_set_event_handler(self, (void (*)(hio_mpl3115a2_t *, hio_mpl3115a2_event_t, void *)) event_handler, event_param);
}

void hio_tag_barometer_set_update_interval(hio_tag_barometer_t *self, hio_tick_t interval)
{
    hio_mpl3115a2_set_update_interval(self, interval);
}

bool hio_tag_barometer_measure(hio_tag_barometer_t *self)
{
    return hio_mpl3115a2_measure(self);
}

bool hio_tag_barometer_get_altitude_meter(hio_tag_barometer_t *self, float *meter)
{
    return hio_mpl3115a2_get_altitude_meter(self, meter);
}

bool hio_tag_barometer_get_pressure_pascal(hio_tag_barometer_t *self, float *pascal)
{
    return hio_mpl3115a2_get_pressure_pascal(self, pascal);
}
