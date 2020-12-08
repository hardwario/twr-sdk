#ifndef _TWR_MODULE_SENSOR_H
#define _TWR_MODULE_SENSOR_H

#include <twr_gpio.h>
#include <twr_onewire.h>

//! @addtogroup twr_module_sensor twr_module_sensor
//! @brief Driver for Sensor Module
//! @{

//! @brief Sensor Module channels

typedef enum
{
    //! @brief Channel A
    TWR_MODULE_SENSOR_CHANNEL_A = 0,

    //! @brief Channel B
    TWR_MODULE_SENSOR_CHANNEL_B = 1,

    //! @brief Channel C
    TWR_MODULE_SENSOR_CHANNEL_C = 2

} twr_module_sensor_channel_t;

//! @brief Sensor module pull

typedef enum
{
    //! @brief Channel has no pull
    TWR_MODULE_SENSOR_PULL_NONE = TWR_GPIO_PULL_NONE,

    //! @brief Channel has internal pull-up
    TWR_MODULE_SENSOR_PULL_UP_INTERNAL = TWR_GPIO_PULL_UP,

    //! @brief Channel has internal pull-down
    TWR_MODULE_SENSOR_PULL_DOWN_INTERNAL = TWR_GPIO_PULL_DOWN,

    //! @brief Channel has pull-up 4k7
    TWR_MODULE_SENSOR_PULL_UP_4K7 = 3,

    //! @brief Channel has pull-up 56R
    TWR_MODULE_SENSOR_PULL_UP_56R = 4,

} twr_module_sensor_pull_t;

//! @brief Sensor Module mode of operation

typedef enum
{
    //! @brief Channel operates as input
    TWR_MODULE_SENSOR_MODE_INPUT = TWR_GPIO_MODE_INPUT,

    //! @brief Channel operates as output
    TWR_MODULE_SENSOR_MODE_OUTPUT = TWR_GPIO_MODE_OUTPUT

} twr_module_sensor_mode_t;

//! @brief Sensor Module hardware revision

typedef enum
{
    //! @brief Hardware revision unknown
    TWR_MODULE_SENSOR_REVISION_UNKNOWN = 0,

    //! @brief Hardware revision R1.0
    TWR_MODULE_SENSOR_REVISION_R1_0 = 1,

    //! @brief Hardware revision R1.1
    TWR_MODULE_SENSOR_REVISION_R1_1 = 2,

} twr_module_sensor_revision_t;

//! @brief Initialize Sensor Module
//! @return true On success
//! @return false On Error

bool twr_module_sensor_init(void);

//! @brief Deinitialize Sensor Module

void twr_module_sensor_deinit(void);

//! @brief Set pull of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] pull Sensor Module pull
//! @return true On success
//! @return false On error

bool twr_module_sensor_set_pull(twr_module_sensor_channel_t channel, twr_module_sensor_pull_t pull);

//! @brief Get pull-up/pull-down configuration for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Pull-up/pull-down configuration

twr_module_sensor_pull_t twr_module_sensor_get_pull(twr_module_sensor_channel_t channel);

//! @brief Set output mode of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] mode Desired mode of operation

void twr_module_sensor_set_mode(twr_module_sensor_channel_t channel, twr_module_sensor_mode_t mode);

//! @brief Get mode of operation for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Mode of operation

twr_module_sensor_mode_t twr_module_sensor_get_mode(twr_module_sensor_channel_t channel);

//! @brief Get input of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Input state

int twr_module_sensor_get_input(twr_module_sensor_channel_t channel);

//! @brief Set output state of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] state State to be set

void twr_module_sensor_set_output(twr_module_sensor_channel_t channel, int state);

//! @brief Get output state for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Output state

int twr_module_sensor_get_output(twr_module_sensor_channel_t channel);

//! @brief Toggle output state for Sensor Module channel
//! @param[in] channel Sensor Module channel

void twr_module_sensor_toggle_output(twr_module_sensor_channel_t channel);

//! @brief Set VDD On / Off
//! @param[in] on On
//! @return true On success
//! @return false On error

bool twr_module_sensor_set_vdd(bool on);

//! @brief Get Sensor Module revision

twr_module_sensor_revision_t twr_module_sensor_get_revision(void);

//! @brief Initialize and get Instance 1-Wire for channel B
//! @return pointer on Instance 1-Wire

twr_onewire_t *twr_module_sensor_get_onewire(void);

//! @brief Semaphore for 1Wire Power up: for R1.1 set VDD On, for R1.0 pull up 56R on channel A
//! @return true On success
//! @return false On error

bool twr_module_sensor_onewire_power_up(void);

//! @brief Semaphore for 1Wire Power down: for R1.1 set VDD Off, for R1.0 pull none on channel A
//! @return true On success
//! @return false On error

bool twr_module_sensor_onewire_power_down(void);

//! @}


#endif // _TWR_MODULE_SENSOR_H
