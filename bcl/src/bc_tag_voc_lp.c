#include <bc_tag_voc_lp.h>

void bc_tag_voc_lp_init(bc_tag_voc_lp_t *self, bc_i2c_channel_t i2c_channel)
{
    bc_sgpc3_init(self, i2c_channel, 0x58);
}

void bc_tag_voc_lp_set_event_handler(bc_tag_voc_lp_t *self, void (*event_handler)(bc_tag_voc_lp_t *, bc_tag_voc_lp_event_t, void *), void *event_param)
{
    bc_sgpc3_set_event_handler(self, (void (*)(bc_sgpc3_t *, bc_sgpc3_event_t, void *)) event_handler, event_param);
}

void bc_tag_voc_lp_set_update_interval(bc_tag_voc_lp_t *self, bc_tick_t interval)
{
    bc_sgpc3_set_update_interval(self, interval);
}

bool bc_tag_voc_lp_measure(bc_tag_voc_lp_t *self)
{
    return bc_sgpc3_measure(self);
}

bool bc_tag_voc_lp_get_tvoc_ppb(bc_tag_voc_lp_t *self, uint16_t *ppb)
{
    return bc_sgpc3_get_tvoc_ppb(self, ppb);
}

float bc_tag_voc_lp_set_compensation(bc_tag_voc_lp_t *self, float *t_celsius, float *rh_percentage)
{
    return bc_sgpc3_set_compensation(self, t_celsius, rh_percentage);
}
