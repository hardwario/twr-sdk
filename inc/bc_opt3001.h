#ifndef _BC_OPT3001_H
#define _BC_OPT3001_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_tick.h>

typedef enum
{
    BC_OPT3001_EVENT_ERROR = 0,
    BC_OPT3001_EVENT_UPDATE = 1

} bc_opt3001_event_t;

typedef enum
{
    BC_OPT3001_STATE_ERROR = -1,
    BC_OPT3001_STATE_INITIALIZE = 0,
    BC_OPT3001_STATE_MEASURE = 1,
    BC_OPT3001_STATE_READ = 2,
    BC_OPT3001_STATE_UPDATE = 3

} bc_opt3001_state_t;

typedef struct bc_opt3001_t bc_opt3001_t;

struct bc_opt3001_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_opt3001_t *, bc_opt3001_event_t);
    bc_tick_t _update_interval;
    bc_opt3001_state_t _state;
    bool _luminosity_valid;
    uint16_t _reg_result;
};

void bc_opt3001_init(bc_opt3001_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
void bc_opt3001_set_event_handler(bc_opt3001_t *self, void (*event_handler)(bc_opt3001_t *, bc_opt3001_event_t));
void bc_opt3001_set_update_interval(bc_opt3001_t *self, bc_tick_t interval);
bool bc_opt3001_get_luminosity_raw(bc_opt3001_t *self, uint16_t *raw);
bool bc_opt3001_get_luminosity_lux(bc_opt3001_t *self, float *lux);

#endif // _BC_OPT3001_H
