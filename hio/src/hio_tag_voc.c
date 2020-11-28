#include <hio_tag_voc.h>

void hio_tag_voc_init(hio_tag_voc_t *self, hio_i2c_channel_t i2c_channel)
{
    hio_sgp30_init(self, i2c_channel, 0x58);
}

void hio_tag_voc_set_event_handler(hio_tag_voc_t *self, void (*event_handler)(hio_tag_voc_t *, hio_tag_voc_event_t, void *), void *event_param)
{
    hio_sgp30_set_event_handler(self, (void (*)(hio_sgp30_t *, hio_sgp30_event_t, void *)) event_handler, event_param);
}

void hio_tag_voc_set_update_interval(hio_tag_voc_t *self, hio_tick_t interval)
{
    hio_sgp30_set_update_interval(self, interval);
}

bool hio_tag_voc_measure(hio_tag_voc_t *self)
{
    return hio_sgp30_measure(self);
}

bool hio_tag_voc_get_co2eq_ppm(hio_tag_voc_t *self, uint16_t *ppm)
{
    return hio_sgp30_get_co2eq_ppm(self, ppm);
}

bool hio_tag_voc_get_tvoc_ppb(hio_tag_voc_t *self, uint16_t *ppb)
{
    return hio_sgp30_get_tvoc_ppb(self, ppb);
}

float hio_tag_voc_set_compensation(hio_tag_voc_t *self, float *t_celsius, float *rh_percentage)
{
    return hio_sgp30_set_compensation(self, t_celsius, rh_percentage);
}
