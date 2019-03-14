#ifndef _BC_MODULE_SENSOR_H
#define _BC_MODULE_SENSOR_H

#include <bc_gpio.h>

//! @addtogroup bc_module_sensor bc_module_sensor
//! @brief Driver for Sensor Module
//! @{

//! @brief Sensor Module channels

typedef enum
{
    //! @brief Channel A
    BC_MODULE_SENSOR_CHANNEL_A = 0,

    //! @brief Channel B
    BC_MODULE_SENSOR_CHANNEL_B = 1

} bc_module_sensor_channel_t;

//! @brief Sensor module pull

typedef enum
{
    //! @brief Channel has no pull
    BC_MODULE_SENSOR_PULL_NONE = 0,

    //! @brief Channel has pull-up 4k7
    BC_MODULE_SENSOR_PULL_UP_4K7 = 1,

    //! @brief Channel has pull-up 56R
    BC_MODULE_SENSOR_PULL_UP_56R = 2,

    //! @brief Channel has internal pull-up
    BC_MODULE_SENSOR_PULL_UP_INTERNAL = 3,

    //! @brief Channel has internal pull-down
    BC_MODULE_SENSOR_PULL_DOWN_INTERNAL = 4

} bc_module_sensor_pull_t;

//! @brief Sensor Module mode of operation

typedef enum
{
    //! @brief Channel operates as input
    BC_MODULE_SENSOR_MODE_INPUT = BC_GPIO_MODE_INPUT,

    //! @brief Channel operates as output
    BC_MODULE_SENSOR_MODE_OUTPUT = BC_GPIO_MODE_OUTPUT

} bc_module_sensor_mode_t;

//! @brief Sensor Module hardware revision

typedef enum
{
    //! @brief Hardware revision unknown
    BC_MODULE_SENSOR_REVISION_UNKNOWN = 0,

    //! @brief Hardware revision R1.0
    BC_MODULE_SENSOR_REVISION_R1_0 = 1,

    //! @brief Hardware revision R1.1
    BC_MODULE_SENSOR_REVISION_R1_1 = 2,

} bc_module_sensor_revision_t;

//! @brief Initialize Sensor Module
//! @return true On success
//! @return false On Error

bool bc_module_sensor_init(void);

//! @brief Deinitialize Sensor Module

void bc_module_sensor_deinit(void);

//! @brief Set pull of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] pull Sensor Module pull
//! @return true On success
//! @return false On error

bool bc_module_sensor_set_pull(bc_module_sensor_channel_t channel, bc_module_sensor_pull_t pull);

//! @brief Get pull-up/pull-down configuration for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Pull-up/pull-down configuration

bc_module_sensor_pull_t bc_module_sensor_get_pull(bc_module_sensor_channel_t channel);

//! @brief Set output mode of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] mode Desired mode of operation

void bc_module_sensor_set_mode(bc_module_sensor_channel_t channel, bc_module_sensor_mode_t mode);

//! @brief Get mode of operation for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Mode of operation

bc_module_sensor_mode_t bc_module_sensor_get_mode(bc_module_sensor_channel_t channel);

//! @brief Get input of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Input state

int bc_module_sensor_get_input(bc_module_sensor_channel_t channel);

//! @brief Set output state of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] state State to be set

void bc_module_sensor_set_output(bc_module_sensor_channel_t channel, int state);

//! @brief Get output state for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Output state

int bc_module_sensor_get_output(bc_module_sensor_channel_t channel);

//! @brief Toggle output state for Sensor Module channel
//! @param[in] channel Sensor Module channel

void bc_module_sensor_toggle_output(bc_module_sensor_channel_t channel);

//! @brief Set VDD On / Off
//! @param[in] on On
//! @return true On success
//! @return false On error

bool bc_module_sensor_set_vdd(bool on);

//! @brief Get Sensor Module revision

bc_module_sensor_revision_t bc_module_sensor_get_revision(void);

//! @brief Initialize 1-Wire on channel B

void bc_module_sensor_onewire_init(void);

//! @brief Semaphore for 1Wire Power up: for R1.1 set VDD On, for R1.0 pull up 56R on channel A
//! @return true On success
//! @return false On error

bool bc_module_sensor_onewire_power_up(void);

//! @brief Semaphore for 1Wire Power down: for R1.1 set VDD Off, for R1.0 pull none on channel A
//! @return true On success
//! @return false On error

bool bc_module_sensor_onewire_power_down(void);

//! @}


#endif // _BC_MODULE_SENSOR_H
