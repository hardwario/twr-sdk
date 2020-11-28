#include <hio_tag_voc_lp.h>

void hio_tag_voc_lp_init(hio_tag_voc_lp_t *self, hio_i2c_channel_t i2c_channel)
{
    hio_sgpc3_init(self, i2c_channel, 0x58);
}

void hio_tag_voc_lp_set_event_handler(hio_tag_voc_lp_t *self, void (*event_handler)(hio_tag_voc_lp_t *, hio_tag_voc_lp_event_t, void *), void *event_param)
{
    hio_sgpc3_set_event_handler(self, (void (*)(hio_sgpc3_t *, hio_sgpc3_event_t, void *)) event_handler, event_param);
}

void hio_tag_voc_lp_set_update_interval(hio_tag_voc_lp_t *self, hio_tick_t interval)
{
    hio_sgpc3_set_update_interval(self, interval);
}

bool hio_tag_voc_lp_measure(hio_tag_voc_lp_t *self)
{
    return hio_sgpc3_measure(self);
}

bool hio_tag_voc_lp_get_tvoc_ppb(hio_tag_voc_lp_t *self, uint16_t *ppb)
{
    return hio_sgpc3_get_tvoc_ppb(self, ppb);
}

float hio_tag_voc_lp_set_compensation(hio_tag_voc_lp_t *self, float *t_celsius, float *rh_percentage)
{
    return hio_sgpc3_set_compensation(self, t_celsius, rh_percentage);
}
