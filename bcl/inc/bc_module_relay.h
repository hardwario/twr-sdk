#ifndef _BC_MODULE_RELAY_H
#define _BC_MODULE_RELAY_H

#include <bc_tca9534a.h>
#include <bc_scheduler.h>

#define BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT 0x3B
#define BC_MODULE_RELAY_I2C_ADDRESS_ALTERNATE 0x3F

//! @addtogroup bc_module_relay bc_module_relay
//! @brief Driver for BigClown Relay Module
//! @section example How to use this driver
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
//! // If you want to set relay to TRUE state...
//! bc_module_relay_set(&relay, true);
//!
//! // If you want to toggle relay...
//! bc_module_relay_toggle(&relay);
//!
//! // If you want to make 1 second pulse to TRUE state...
//! bc_module_relay_pulse(&relay, true, 1000);
//! @endcode
//! @{

//! @brief Reported relay states

typedef enum
{
    //! @brief State is unknown
    BC_MODULE_RELAY_STATE_UNKNOWN = -1,

    //! @brief State is false
    BC_MODULE_RELAY_STATE_FALSE = 0,

    //! @brief State is true
    BC_MODULE_RELAY_STATE_TRUE = 1

} bc_module_relay_state_t;

//! @brief BigClown Relay Module instance

typedef struct bc_module_relay_t bc_module_relay_t;

//! @cond

typedef enum
{
    BC_MODULE_RELAY_TASK_STATE_IDLE,
    BC_MODULE_RELAY_TASK_STATE_SET,
    BC_MODULE_RELAY_TASK_STATE_SET_DEMAGNETIZE,
    BC_MODULE_RELAY_TASK_STATE_PULSE,
    BC_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE,
    BC_MODULE_RELAY_TASK_STATE_PULSE_REVERSE,
    BC_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE_2

} bc_module_relay_task_state_t;

typedef enum
{
    BC_MODULE_RELAY_COMMAND_NONE = 0,
    BC_MODULE_RELAY_COMMAND_SET = 1,
    BC_MODULE_RELAY_COMMAND_PULSE = 2

} bc_module_relay_command_t;

struct bc_module_relay_t
{
    uint8_t _i2c_address;
    bc_tca9534a_t _tca9534a;
    bc_module_relay_state_t _relay_state;
    bc_tick_t _timestamp;
    bc_scheduler_task_id_t _task_id;
    bc_module_relay_task_state_t _state;
    bc_module_relay_command_t _command;
    bc_module_relay_state_t _desired_state;
    bc_tick_t _pulse_duration;
    bool _task_is_active;
    bool _error;
};

//! @endcond

//! @brief Initialize BigClown Relay Module
//! @param[in] self Instance

bool bc_module_relay_init(bc_module_relay_t *self, uint8_t i2c_address);

//! @brief Set relay to specified state
//! @param[in] self Instance
//! @param[in] state Desired relay state

void bc_module_relay_set_state(bc_module_relay_t *self, bool state);

//! @brief Get current relay state
//! @param[in] self Instance
//! @return Relay state

bc_module_relay_state_t bc_module_relay_get_state(bc_module_relay_t *self);

//! @brief Generate pulse to specified state for given duration
//! @param[in] self Instance
//! @param[in] direction Desired pulse state
//! @param[in] duration Desired pulse duration

void bc_module_relay_pulse(bc_module_relay_t *self, bool direction, bc_tick_t duration);

//! @brief Toggle relay to opposite state
//! @param[in] self Instance

void bc_module_relay_toggle(bc_module_relay_t *self);

//! @}

#endif // _BC_MODULE_RELAY_H
