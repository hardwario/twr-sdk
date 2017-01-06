#ifndef _BC_MPL3115A2_H
#define _BC_MPL3115A2_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_tick.h>

typedef enum
{
    BC_MPL3115A2_EVENT_ERROR = 0,
    BC_MPL3115A2_EVENT_UPDATE = 1

} bc_mpl3115a2_event_t;

typedef enum
{
    BC_MPL3115A2_STATE_ERROR = -1,
    BC_MPL3115A2_STATE_INITIALIZE = 0,
    BC_MPL3115A2_STATE_MEASURE_ALTITUDE = 1,
    BC_MPL3115A2_STATE_READ_ALTITUDE = 2,
    BC_MPL3115A2_STATE_MEASURE_PRESSURE = 3,
    BC_MPL3115A2_STATE_READ_PRESSURE = 4,
    BC_MPL3115A2_STATE_UPDATE = 5

} bc_mpl3115a2_state_t;

typedef struct bc_mpl3115a2_t bc_mpl3115a2_t;

struct bc_mpl3115a2_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_mpl3115a2_t *, bc_mpl3115a2_event_t);
    bc_tick_t _update_interval;
    bc_mpl3115a2_state_t _state;
    bool _altitude_valid;
    bool _pressure_valid;
    uint8_t _reg_out_p_msb_altitude;
    uint8_t _reg_out_p_csb_altitude;
    uint8_t _reg_out_p_lsb_altitude;
    uint8_t _reg_out_t_msb_altitude;
    uint8_t _reg_out_t_lsb_altitude;
    uint8_t _reg_out_p_msb_pressure;
    uint8_t _reg_out_p_csb_pressure;
    uint8_t _reg_out_p_lsb_pressure;
    uint8_t _reg_out_t_msb_pressure;
    uint8_t _reg_out_t_lsb_pressure;
};

void bc_mpl3115a2_init(bc_mpl3115a2_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
void bc_mpl3115a2_set_event_handler(bc_mpl3115a2_t *self, void (*event_handler)(bc_mpl3115a2_t *, bc_mpl3115a2_event_t));
void bc_mpl3115a2_set_update_interval(bc_mpl3115a2_t *self, bc_tick_t interval);
bool bc_mpl3115a2_get_altitude_meter(bc_mpl3115a2_t *self, float *meter);
bool bc_mpl3115a2_get_pressure_pascal(bc_mpl3115a2_t *self, float *pascal);

#endif // _BC_MPL3115A2_H
