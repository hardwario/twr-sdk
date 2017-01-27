#ifndef _BC_HTS221_H
#define _BC_HTS221_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

typedef enum
{
    BC_HTS221_EVENT_ERROR = 0,
    BC_HTS221_EVENT_UPDATE = 1

} bc_hts221_event_t;

typedef enum
{
    BC_HTS221_STATE_ERROR = -1,
    BC_HTS221_STATE_INITIALIZE = 0,
    BC_HTS221_STATE_MEASURE = 1,
    BC_HTS221_STATE_READ = 2,
    BC_HTS221_STATE_UPDATE = 3

} bc_hts221_state_t;

typedef struct bc_hts221_t bc_hts221_t;

struct bc_hts221_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_hts221_t *, bc_hts221_event_t, void *);
    void *_event_param;
    bc_scheduler_task_id_t _task_id;
    bc_tick_t _update_interval;
    bc_hts221_state_t _state;
    bool _humidity_valid;
    int16_t _reg_humidity;
    int16_t _h0_rh;
    int16_t _h0_t0_out;
    float h_grad;
};

void bc_hts221_init(bc_hts221_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
void bc_hts221_set_event_handler(bc_hts221_t *self, void (*event_handler)(bc_hts221_t *, bc_hts221_event_t, void *), void *event_param);
void bc_hts221_set_update_interval(bc_hts221_t *self, bc_tick_t interval);
bool bc_hts221_get_humidity_percentage(bc_hts221_t *self, float *percentage);

#endif // _BC_HTS221_H
