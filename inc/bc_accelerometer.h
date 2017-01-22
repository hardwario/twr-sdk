#ifndef _BC_ACCELEROMETER_H
#define _BC_ACCELEROMETER_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_tick.h>

#define BC_ACCELEROMETER_ADDRESS_DEFAULT 0x18
#define BC_ACCELEROMETER_ADDRESS_ALTERNATE 0x19

typedef enum
{
    BC_ACCELEROMETER_EVENT_ERROR = 0,  //!< Error event
    BC_ACCELEROMETER_EVENT_UPDATE = 1  //!< Update event

} bc_accelerometer_event_t;

typedef enum
{
    BC_ACCELEROMETER_STATE_ERROR = -1,
    BC_ACCELEROMETER_STATE_MEASURE = 0,
    BC_ACCELEROMETER_STATE_READ = 1,
    BC_ACCELEROMETER_STATE_UPDATE = 2

} bc_accelerometer_state_t;

typedef struct
{
	int16_t x_axis;
	int16_t y_axis;
	int16_t z_axis;

} bc_accelerometer_result_raw_t;

typedef struct
{
	float x_axis;
	float y_axis;
	float z_axis;

} bc_accelerometer_result_g_t;

typedef struct bc_accelerometer_t bc_accelerometer_t;

struct bc_accelerometer_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_accelerometer_t *, bc_accelerometer_event_t);
    bc_tick_t _update_interval;
    bc_accelerometer_state_t _state;
    bool _accelerometer_valid;
    uint16_t _reg_temperature;


	bool _communication_fault;
	uint8_t _out_x_l;
	uint8_t _out_x_h;
	uint8_t _out_y_l;
	uint8_t _out_y_h;
	uint8_t _out_z_l;
	uint8_t _out_z_h;

};

bool bc_accelerometer_init(bc_accelerometer_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
void bc_accelerometer_set_event_handler(bc_accelerometer_t *self, void (*event_handler)(bc_accelerometer_t *, bc_accelerometer_event_t));
bool bc_accelerometer_is_communication_fault(bc_accelerometer_t *self);
bool bc_accelerometer_get_state(bc_accelerometer_t *self, bc_accelerometer_state_t *state);
bool bc_accelerometer_power_down(bc_accelerometer_t *self);
bool bc_accelerometer_continuous_conversion(bc_accelerometer_t *self);
bool bc_accelerometer_read_result(bc_accelerometer_t *self);
bool bc_accelerometer_get_result_raw(bc_accelerometer_t *self, bc_accelerometer_result_raw_t *result_raw);
bool bc_accelerometer_get_result_g(bc_accelerometer_t *self, bc_accelerometer_result_g_t *result_g);

#endif /* _BC_ACCELEROMETER_H */
