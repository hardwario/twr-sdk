#include <hio_tag_lux_meter.h>

void hio_tag_lux_meter_init(hio_tag_lux_meter_t *self, hio_i2c_channel_t i2c_channel, hio_tag_lux_meter_i2c_address_t i2c_address)
{
    hio_opt3001_init(self, i2c_channel, (uint8_t) i2c_address);
}

void hio_tag_lux_meter_set_event_handler(hio_tag_lux_meter_t *self, void (*event_handler)(hio_tag_lux_meter_t *, hio_tag_lux_meter_event_t, void *), void *event_param)
{
    hio_opt3001_set_event_handler(self, (void (*)(hio_opt3001_t *, hio_opt3001_event_t, void *)) event_handler, event_param);
}

void hio_tag_lux_meter_set_update_interval(hio_tag_lux_meter_t *self, hio_tick_t interval)
{
    hio_opt3001_set_update_interval(self, interval);
}

bool hio_tag_lux_meter_measure(hio_tag_lux_meter_t *self)
{
    return hio_opt3001_measure(self);
}

bool hio_tag_lux_meter_get_illuminance_raw(hio_tag_lux_meter_t *self, uint16_t *raw)
{
    return hio_opt3001_get_illuminance_raw(self, raw);
}

bool hio_tag_lux_meter_get_illuminance_lux(hio_tag_lux_meter_t *self, float *lux)
{
    return hio_opt3001_get_illuminance_lux(self, lux);
}
