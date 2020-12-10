#include <twr_tag_lux_meter.h>

void twr_tag_lux_meter_init(twr_tag_lux_meter_t *self, twr_i2c_channel_t i2c_channel, twr_tag_lux_meter_i2c_address_t i2c_address)
{
    twr_opt3001_init(self, i2c_channel, (uint8_t) i2c_address);
}

void twr_tag_lux_meter_set_event_handler(twr_tag_lux_meter_t *self, void (*event_handler)(twr_tag_lux_meter_t *, twr_tag_lux_meter_event_t, void *), void *event_param)
{
    twr_opt3001_set_event_handler(self, (void (*)(twr_opt3001_t *, twr_opt3001_event_t, void *)) event_handler, event_param);
}

void twr_tag_lux_meter_set_update_interval(twr_tag_lux_meter_t *self, twr_tick_t interval)
{
    twr_opt3001_set_update_interval(self, interval);
}

bool twr_tag_lux_meter_measure(twr_tag_lux_meter_t *self)
{
    return twr_opt3001_measure(self);
}

bool twr_tag_lux_meter_get_illuminance_raw(twr_tag_lux_meter_t *self, uint16_t *raw)
{
    return twr_opt3001_get_illuminance_raw(self, raw);
}

bool twr_tag_lux_meter_get_illuminance_lux(twr_tag_lux_meter_t *self, float *lux)
{
    return twr_opt3001_get_illuminance_lux(self, lux);
}
