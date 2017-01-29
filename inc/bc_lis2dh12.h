#ifndef _BC_LIS2DH12_H
#define _BC_LIS2DH12_H

#include <bc_i2c.h>
#include <bc_tick.h>

#define BC_LIS2DH12_ADDRESS_DEFAULT 0x18
#define BC_LIS2DH12_ADDRESS_ALTERNATE 0x19

typedef enum
{
    BC_LIS2DH12_EVENT_ERROR = 0, //!< Error event
    BC_LIS2DH12_EVENT_UPDATE = 1, //!< Update event
    BC_LIS2DH12_EVENT_ALARM = 2 //!< Alarm event

} bc_lis2dh12_event_t;

typedef enum
{
    BC_LIS2DH12_STATE_ERROR = -1,
    BC_LIS2DH12_STATE_MEASURE = 0,
    BC_LIS2DH12_STATE_READ = 1,
    BC_LIS2DH12_STATE_UPDATE = 2

} bc_lis2dh12_state_t;

typedef struct
{
    int16_t x_axis;
    int16_t y_axis;
    int16_t z_axis;

} bc_lis2dh12_result_raw_t;

typedef struct
{
    float x_axis;
    float y_axis;
    float z_axis;

} bc_lis2dh12_result_g_t;

typedef struct
{
    float treshold;
    uint8_t duration;
    bool x_low;
    bool x_high;
    bool y_low;
    bool y_high;
    bool z_low;
    bool z_high;

} bc_lis2dh12_alarm_t;

typedef struct bc_lis2dh12_t bc_lis2dh12_t;

struct bc_lis2dh12_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_lis2dh12_t *, bc_lis2dh12_event_t);
    bc_tick_t _update_interval;
    bc_lis2dh12_state_t _state;
    bool _accelerometer_valid;
    uint8_t _out_x_l;
    uint8_t _out_x_h;
    uint8_t _out_y_l;
    uint8_t _out_y_h;
    uint8_t _out_z_l;
    uint8_t _out_z_h;
    bool _alarm_active;
};

bool bc_lis2dh12_init(bc_lis2dh12_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
void bc_lis2dh12_set_event_handler(bc_lis2dh12_t *self, void (*event_handler)(bc_lis2dh12_t *, bc_lis2dh12_event_t));
void bc_lis2dh12_set_update_interval(bc_lis2dh12_t *self, bc_tick_t interval);
bool bc_lis2dh12_get_result_raw(bc_lis2dh12_t *self, bc_lis2dh12_result_raw_t *result_raw);
bool bc_lis2dh12_get_result_g(bc_lis2dh12_t *self, bc_lis2dh12_result_g_t *result_g);

bool bc_lis2dh12_set_alarm(bc_lis2dh12_t *self, bc_lis2dh12_alarm_t *alarm);

// Experimental testing
//void bc_lis2dh12_set_config(bc_lis2dh12_t *self);

#endif // _BC_LIS2DH12_H
