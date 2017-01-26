#include <bc_tag_humidity.h>

void bc_tag_humidity_init(bc_tag_humidity_t *self, bc_i2c_channel_t i2c_channel, bc_tag_humidity_i2c_address_t i2c_address)
{
    bc_hdc2080_init(self, i2c_channel, (uint8_t) i2c_address);
}

void bc_tag_humidity_set_event_handler(bc_tag_humidity_t *self, void (*event_handler)(bc_tag_humidity_t *, bc_tag_humidity_event_t))
{
    bc_hdc2080_set_event_handler(self, (void (*)(bc_hdc2080_t *, bc_hdc2080_event_t)) event_handler);
}

void bc_tag_humidity_set_update_interval(bc_tag_humidity_t *self, bc_tick_t interval)
{
    bc_hdc2080_set_update_interval(self, interval);
}

bool bc_tag_humidity_get_humidity_raw(bc_tag_humidity_t *self, uint16_t *raw)
{
    return bc_hdc2080_get_humidity_raw(self, raw);
}

bool bc_tag_humidity_get_humidity_percentage(bc_tag_humidity_t *self, float *percentage)
{
    return bc_hdc2080_get_humidity_percentage(self, percentage);
}
