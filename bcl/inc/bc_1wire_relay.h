#ifndef _BC_1WIRE_RELAY_H
#define _BC_1WIRE_RELAY_H

#include <bc_1wire.h>
#include <bc_module_sensor.h>

#define BC_1WIRE_RELAY_FAMILY_CODE 0x29

//! @addtogroup bc_onewire_relay bc_onewire_relay
//! @brief Driver for BigClown 1-wire relay, chipset: DS2408
//! @{


typedef enum
{
	BC_1WIRE_RELAY_Q1 = 0,
	BC_1WIRE_RELAY_Q2 = 1,
	BC_1WIRE_RELAY_Q3 = 2,
	BC_1WIRE_RELAY_Q4 = 3,
	BC_1WIRE_RELAY_Q5 = 4,
	BC_1WIRE_RELAY_Q6 = 5,
	BC_1WIRE_RELAY_Q7 = 6,
	BC_1WIRE_RELAY_Q8 = 7

} bc_1wire_relay_q_t;

//! @brief BigClown 1-wire relay instance

typedef struct
{
	uint64_t _device_number;
	bc_gpio_channel_t _channel;
	uint8_t _state;

} bc_1wire_relay_t;

//! @brief Initialize relay
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool bc_1wire_relay_init(bc_1wire_relay_t *self, uint64_t device_number, bc_module_sensor_channel_t channel);

//! @brief Set relay to specified state
//! @param[in] self Instance
//! @param[in] q Q
//! @param[in] state Desired relay state
//! @return true On success
//! @return false On failure

bool bc_1wire_relay_set_state(bc_1wire_relay_t *self, bc_1wire_relay_q_t q, bool state);

//! @brief Get current relay state
//! @param[in] self Instance
//! @param[in] q Q
//! @param[out] state
//! @return true When value is valid
//! @return false When value is invalid

bool bc_1wire_relay_get_state(bc_1wire_relay_t *self, bc_1wire_relay_q_t q, bool *state);

//! @}

#endif // _BC_1WIRE_RELAY_H
