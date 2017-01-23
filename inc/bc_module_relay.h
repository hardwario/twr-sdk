#ifndef _BC_MODULE_RELAY_H
#define _BC_MODULE_RELAY_H

#include <bc_common.h>
#include <bc_tca9534a.h>

//! @addtogroup bc_module_relay bc_module_relay
//! @brief Driver for Relay Module
//! @section example How to use Relay Module
//! @code
//! #include <bc_module_relay.h>
//!
//! bc_module_relay_t relay;
//!
//! void application_init(void)
//! {
//!     bc_module_relay_init(&relay);
//! }
//!
//! // Then use the functions to set the relay
//! bc_module_relay_set(&relay, BC_MODULE_RELAY_STATE_TRUE);
//! // Toggle the relay
//! bc_module_relay_toggle(&relay);
//! // Make a 1000ms long pulse
//! bc_module_relay_pulse(&relay, 1000);
//!
//! @endcode
//! @{

//! @brief Relay state

typedef enum
{
    BC_MODULE_RELAY_STATE_FALSE = 0x00,
    BC_MODULE_RELAY_STATE_TRUE = 0x01,
    BC_MODULE_RELAY_STATE_UNKNOWN = 0x02
} bc_module_relay_state_t;

//! Internal state machine states

typedef enum
{
    BC_MODULE_RELAY_TASK_STATE_DEMAGNETIZE = 0x00,
    BC_MODULE_RELAY_TASK_STATE_DEMAGNETIZE_PULSE = 0x01,
    BC_MODULE_RELAY_TASK_STATE_REVERSE_PULSE = 0x02,
} bc_module_relay_task_state_t;


typedef struct bc_module_relay_t bc_module_relay_t;

//! @brief Structure of Relay Module instance

struct bc_module_relay_t
{
    bc_tca9534a_t _tca9534a;
    bc_module_relay_state_t _relay_state;
    bc_tick_t _timestamp;
    bc_scheduler_task_id_t _task_id;

    bc_module_relay_task_state_t _state;
    bc_tick_t _pulse_length;
};

//! @brief Initialize Relay Module driver
//! @param[in] self Instance

void bc_module_relay_init(bc_module_relay_t *self);

//! @brief Set the relay on Relay Module
//! @param[in] self Instance
//! @param[in] state State to set or reset the relay

void bc_module_relay_set(bc_module_relay_t *self, bc_module_relay_state_t state);

//! @brief Creates an output pulse on Relay Module
//! @param[in] self Instance
//! @param[in] duration Duration of the output pulse

void bc_module_relay_pulse(bc_module_relay_t *self, bc_tick_t duration);

//! @brief Toggle the relay state to another
//! @param[in] self Instance

bool bc_module_relay_toggle(bc_module_relay_t *self);

//! @}

#endif /* _BC_MODULE_RELAY_H */
