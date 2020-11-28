#include <bc_tag_lux_meter.h>

void bc_tag_lux_meter_init(bc_tag_lux_meter_t *self, bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address)
{
    bc_opt3001_init(self, i2c_channel, (uint8_t) i2c_address);
}

void bc_tag_lux_meter_set_event_handler(bc_tag_lux_meter_t *self, void (*event_handler)(bc_tag_lux_meter_t *, bc_tag_lux_meter_event_t, void *), void *event_param)
{
    bc_opt3001_set_event_handler(self, (void (*)(bc_opt3001_t *, bc_opt3001_event_t, void *)) event_handler, event_param);
}

void bc_tag_lux_meter_set_update_interval(bc_tag_lux_meter_t *self, bc_tick_t interval)
{
    bc_opt3001_set_update_interval(self, interval);
}

bool bc_tag_lux_meter_measure(bc_tag_lux_meter_t *self)
{
    return bc_opt3001_measure(self);
}

bool bc_tag_lux_meter_get_illuminance_raw(bc_tag_lux_meter_t *self, uint16_t *raw)
{
    return bc_opt3001_get_illuminance_raw(self, raw);
}

bool bc_tag_lux_meter_get_illuminance_lux(bc_tag_lux_meter_t *self, float *lux)
{
    return bc_opt3001_get_illuminance_lux(self, lux);
}
