#include <twr_tag_voc_lp.h>

void twr_tag_voc_lp_init(twr_tag_voc_lp_t *self, twr_i2c_channel_t i2c_channel)
{
    twr_sgpc3_init(self, i2c_channel, 0x58);
}

void twr_tag_voc_lp_set_event_handler(twr_tag_voc_lp_t *self, void (*event_handler)(twr_tag_voc_lp_t *, twr_tag_voc_lp_event_t, void *), void *event_param)
{
    twr_sgpc3_set_event_handler(self, (void (*)(twr_sgpc3_t *, twr_sgpc3_event_t, void *)) event_handler, event_param);
}

void twr_tag_voc_lp_set_update_interval(twr_tag_voc_lp_t *self, twr_tick_t interval)
{
    twr_sgpc3_set_update_interval(self, interval);
}

bool twr_tag_voc_lp_measure(twr_tag_voc_lp_t *self)
{
    return twr_sgpc3_measure(self);
}

bool twr_tag_voc_lp_get_tvoc_ppb(twr_tag_voc_lp_t *self, uint16_t *ppb)
{
    return twr_sgpc3_get_tvoc_ppb(self, ppb);
}

float twr_tag_voc_lp_set_compensation(twr_tag_voc_lp_t *self, float *t_celsius, float *rh_percentage)
{
    return twr_sgpc3_set_compensation(self, t_celsius, rh_percentage);
}
