#ifndef _TWR_MODULE_RELAY_H
#define _TWR_MODULE_RELAY_H

#include <twr_tca9534a.h>
#include <twr_scheduler.h>

#define TWR_MODULE_RELAY_I2C_ADDRESS_DEFAULT 0x3B
#define TWR_MODULE_RELAY_I2C_ADDRESS_ALTERNATE 0x3F

//! @addtogroup twr_module_relay twr_module_relay
//! @brief Driver for HARDWARIO Relay Module
//! @section example How to use this driver
//! @code
//! #include <twr_module_relay.h>
//!
//! twr_module_relay_t relay;
//!
//! void application_init(void)
//! {
//!     twr_module_relay_init(&relay);
//! }
//!
//! // If you want to set relay to TRUE state...
//! twr_module_relay_set(&relay, true);
//!
//! // If you want to toggle relay...
//! twr_module_relay_toggle(&relay);
//!
//! // If you want to make 1 second pulse to TRUE state...
//! twr_module_relay_pulse(&relay, true, 1000);
//! @endcode
//! @{

//! @brief Reported relay states

typedef enum
{
    //! @brief State is unknown
    TWR_MODULE_RELAY_STATE_UNKNOWN = -1,

    //! @brief State is false
    TWR_MODULE_RELAY_STATE_FALSE = 0,

    //! @brief State is true
    TWR_MODULE_RELAY_STATE_TRUE = 1

} twr_module_relay_state_t;

//! @brief HARDWARIO Relay Module instance

typedef struct twr_module_relay_t twr_module_relay_t;

//! @cond

typedef enum
{
    TWR_MODULE_RELAY_TASK_STATE_IDLE,
    TWR_MODULE_RELAY_TASK_STATE_SET,
    TWR_MODULE_RELAY_TASK_STATE_SET_DEMAGNETIZE,
    TWR_MODULE_RELAY_TASK_STATE_PULSE,
    TWR_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE,
    TWR_MODULE_RELAY_TASK_STATE_PULSE_REVERSE,
    TWR_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE_2

} twr_module_relay_task_state_t;

typedef enum
{
    TWR_MODULE_RELAY_COMMAND_NONE = 0,
    TWR_MODULE_RELAY_COMMAND_SET = 1,
    TWR_MODULE_RELAY_COMMAND_PULSE = 2

} twr_module_relay_command_t;

struct twr_module_relay_t
{
    uint8_t _i2c_address;
    twr_tca9534a_t _tca9534a;
    twr_module_relay_state_t _relay_state;
    twr_tick_t _timestamp;
    twr_scheduler_task_id_t _task_id;
    twr_module_relay_task_state_t _state;
    twr_module_relay_command_t _command;
    twr_module_relay_state_t _desired_state;
    twr_tick_t _pulse_duration;
    bool _task_is_active;
    bool _error;
};

//! @endcond

//! @brief Initialize HARDWARIO Relay Module
//! @param[in] self Instance

bool twr_module_relay_init(twr_module_relay_t *self, uint8_t i2c_address);

//! @brief Set relay to specified state
//! @param[in] self Instance
//! @param[in] state Desired relay state

void twr_module_relay_set_state(twr_module_relay_t *self, bool state);

//! @brief Get current relay state
//! @param[in] self Instance
//! @return Relay state

twr_module_relay_state_t twr_module_relay_get_state(twr_module_relay_t *self);

//! @brief Generate pulse to specified state for given duration
//! @param[in] self Instance
//! @param[in] direction Desired pulse state
//! @param[in] duration Desired pulse duration

void twr_module_relay_pulse(twr_module_relay_t *self, bool direction, twr_tick_t duration);

//! @brief Toggle relay to opposite state
//! @param[in] self Instance

void twr_module_relay_toggle(twr_module_relay_t *self);

//! @}

#endif // _TWR_MODULE_RELAY_H
