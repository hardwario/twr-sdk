#ifndef _BC_HDC2080_H
#define _BC_HDC2080_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_tick.h>

typedef enum
{
    BC_HDC2080_EVENT_ERROR = 0,
    BC_HDC2080_EVENT_UPDATE = 1

} bc_hdc2080_event_t;

typedef enum
{
    BC_HDC2080_STATE_ERROR = -1,
    BC_HDC2080_STATE_INITIALIZE = 0,
    BC_HDC2080_STATE_MEASURE = 1,
    BC_HDC2080_STATE_READ = 2,
    BC_HDC2080_STATE_UPDATE = 3

} bc_hdc2080_state_t;

typedef struct bc_hdc2080_t bc_hdc2080_t;

struct bc_hdc2080_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_hdc2080_t *, bc_hdc2080_event_t);
    bc_tick_t _update_interval;
    bc_hdc2080_state_t _state;
    bool _humidity_valid;
    uint16_t _reg_humidity;
};

void bc_hdc2080_init(bc_hdc2080_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
void bc_hdc2080_set_event_handler(bc_hdc2080_t *self, void (*event_handler)(bc_hdc2080_t *, bc_hdc2080_event_t));
void bc_hdc2080_set_update_interval(bc_hdc2080_t *self, bc_tick_t interval);
bool bc_hdc2080_get_humidity_raw(bc_hdc2080_t *self, uint16_t *raw);
bool bc_hdc2080_get_humidity_percentage(bc_hdc2080_t *self, float *percentage);

#endif // _BC_HDC2080_H
