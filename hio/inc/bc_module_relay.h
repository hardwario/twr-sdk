#ifndef _HIO_MODULE_RELAY_H
#define _HIO_MODULE_RELAY_H

#include <hio_tca9534a.h>
#include <hio_scheduler.h>

#define HIO_MODULE_RELAY_I2C_ADDRESS_DEFAULT 0x3B
#define HIO_MODULE_RELAY_I2C_ADDRESS_ALTERNATE 0x3F

//! @addtogroup hio_module_relay hio_module_relay
//! @brief Driver for HARDWARIO Relay Module
//! @section example How to use this driver
//! @code
//! #include <hio_module_relay.h>
//!
//! hio_module_relay_t relay;
//!
//! void application_init(void)
//! {
//!     hio_module_relay_init(&relay);
//! }
//!
//! // If you want to set relay to TRUE state...
//! hio_module_relay_set(&relay, true);
//!
//! // If you want to toggle relay...
//! hio_module_relay_toggle(&relay);
//!
//! // If you want to make 1 second pulse to TRUE state...
//! hio_module_relay_pulse(&relay, true, 1000);
//! @endcode
//! @{

//! @brief Reported relay states

typedef enum
{
    //! @brief State is unknown
    HIO_MODULE_RELAY_STATE_UNKNOWN = -1,

    //! @brief State is false
    HIO_MODULE_RELAY_STATE_FALSE = 0,

    //! @brief State is true
    HIO_MODULE_RELAY_STATE_TRUE = 1

} hio_module_relay_state_t;

//! @brief HARDWARIO Relay Module instance

typedef struct hio_module_relay_t hio_module_relay_t;

//! @cond

typedef enum
{
    HIO_MODULE_RELAY_TASK_STATE_IDLE,
    HIO_MODULE_RELAY_TASK_STATE_SET,
    HIO_MODULE_RELAY_TASK_STATE_SET_DEMAGNETIZE,
    HIO_MODULE_RELAY_TASK_STATE_PULSE,
    HIO_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE,
    HIO_MODULE_RELAY_TASK_STATE_PULSE_REVERSE,
    HIO_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE_2

} hio_module_relay_task_state_t;

typedef enum
{
    HIO_MODULE_RELAY_COMMAND_NONE = 0,
    HIO_MODULE_RELAY_COMMAND_SET = 1,
    HIO_MODULE_RELAY_COMMAND_PULSE = 2

} hio_module_relay_command_t;

struct hio_module_relay_t
{
    uint8_t _i2c_address;
    hio_tca9534a_t _tca9534a;
    hio_module_relay_state_t _relay_state;
    hio_tick_t _timestamp;
    hio_scheduler_task_id_t _task_id;
    hio_module_relay_task_state_t _state;
    hio_module_relay_command_t _command;
    hio_module_relay_state_t _desired_state;
    hio_tick_t _pulse_duration;
    bool _task_is_active;
    bool _error;
};

//! @endcond

//! @brief Initialize HARDWARIO Relay Module
//! @param[in] self Instance

bool hio_module_relay_init(hio_module_relay_t *self, uint8_t i2c_address);

//! @brief Set relay to specified state
//! @param[in] self Instance
//! @param[in] state Desired relay state

void hio_module_relay_set_state(hio_module_relay_t *self, bool state);

//! @brief Get current relay state
//! @param[in] self Instance
//! @return Relay state

hio_module_relay_state_t hio_module_relay_get_state(hio_module_relay_t *self);

//! @brief Generate pulse to specified state for given duration
//! @param[in] self Instance
//! @param[in] direction Desired pulse state
//! @param[in] duration Desired pulse duration

void hio_module_relay_pulse(hio_module_relay_t *self, bool direction, hio_tick_t duration);

//! @brief Toggle relay to opposite state
//! @param[in] self Instance

void hio_module_relay_toggle(hio_module_relay_t *self);

//! @}

#endif // _HIO_MODULE_RELAY_H
