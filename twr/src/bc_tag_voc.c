#include <bc_tag_voc.h>

void bc_tag_voc_init(bc_tag_voc_t *self, bc_i2c_channel_t i2c_channel)
{
    bc_sgp30_init(self, i2c_channel, 0x58);
}

void bc_tag_voc_set_event_handler(bc_tag_voc_t *self, void (*event_handler)(bc_tag_voc_t *, bc_tag_voc_event_t, void *), void *event_param)
{
    bc_sgp30_set_event_handler(self, (void (*)(bc_sgp30_t *, bc_sgp30_event_t, void *)) event_handler, event_param);
}

void bc_tag_voc_set_update_interval(bc_tag_voc_t *self, bc_tick_t interval)
{
    bc_sgp30_set_update_interval(self, interval);
}

bool bc_tag_voc_measure(bc_tag_voc_t *self)
{
    return bc_sgp30_measure(self);
}

bool bc_tag_voc_get_co2eq_ppm(bc_tag_voc_t *self, uint16_t *ppm)
{
    return bc_sgp30_get_co2eq_ppm(self, ppm);
}

bool bc_tag_voc_get_tvoc_ppb(bc_tag_voc_t *self, uint16_t *ppb)
{
    return bc_sgp30_get_tvoc_ppb(self, ppb);
}

float bc_tag_voc_set_compensation(bc_tag_voc_t *self, float *t_celsius, float *rh_percentage)
{
    return bc_sgp30_set_compensation(self, t_celsius, rh_percentage);
}
