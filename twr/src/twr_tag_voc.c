#include <twr_tag_voc.h>

void twr_tag_voc_init(twr_tag_voc_t *self, twr_i2c_channel_t i2c_channel)
{
    twr_sgp30_init(self, i2c_channel, 0x58);
}

void twr_tag_voc_set_event_handler(twr_tag_voc_t *self, void (*event_handler)(twr_tag_voc_t *, twr_tag_voc_event_t, void *), void *event_param)
{
    twr_sgp30_set_event_handler(self, (void (*)(twr_sgp30_t *, twr_sgp30_event_t, void *)) event_handler, event_param);
}

void twr_tag_voc_set_update_interval(twr_tag_voc_t *self, twr_tick_t interval)
{
    twr_sgp30_set_update_interval(self, interval);
}

bool twr_tag_voc_measure(twr_tag_voc_t *self)
{
    return twr_sgp30_measure(self);
}

bool twr_tag_voc_get_co2eq_ppm(twr_tag_voc_t *self, uint16_t *ppm)
{
    return twr_sgp30_get_co2eq_ppm(self, ppm);
}

bool twr_tag_voc_get_tvoc_ppb(twr_tag_voc_t *self, uint16_t *ppb)
{
    return twr_sgp30_get_tvoc_ppb(self, ppb);
}

float twr_tag_voc_set_compensation(twr_tag_voc_t *self, float *t_celsius, float *rh_percentage)
{
    return twr_sgp30_set_compensation(self, t_celsius, rh_percentage);
}
