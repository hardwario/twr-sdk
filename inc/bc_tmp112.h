#ifndef _BC_TMP112_H
#define _BC_TMP112_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_tick.h>

typedef enum
{
    BC_TMP112_EVENT_ERROR = 0,
    BC_TMP112_EVENT_UPDATE = 1

} bc_tmp112_event_t;

typedef enum
{
    BC_TMP112_STATE_ERROR = -1,
    BC_TMP112_STATE_MEASURE = 0,
    BC_TMP112_STATE_READ = 1,
    BC_TMP112_STATE_UPDATE = 2

} bc_tmp112_state_t;

typedef struct bc_tmp112_t bc_tmp112_t;

struct bc_tmp112_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_tmp112_t *, bc_tmp112_event_t);
    bc_tick_t _update_interval;
    bc_tmp112_state_t _state;
    bool _temperature_valid;
    uint16_t _reg_temperature;
};

void bc_tmp112_init(bc_tmp112_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
void bc_tmp112_set_event_handler(bc_tmp112_t *self, void (*event_handler)(bc_tmp112_t *, bc_tmp112_event_t));
void bc_tmp112_set_update_interval(bc_tmp112_t *self, bc_tick_t interval);
bool bc_tmp112_get_temperature_raw(bc_tmp112_t *self, int16_t *raw);
bool bc_tmp112_get_temperature_celsius(bc_tmp112_t *self, float *celsius);
bool bc_tmp112_get_temperature_fahrenheit(bc_tmp112_t *self, float *fahrenheit);
bool bc_tmp112_get_temperature_kelvin(bc_tmp112_t *self, float *kelvin);

#endif // _BC_TMP112_H
