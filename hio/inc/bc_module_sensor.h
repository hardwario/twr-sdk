#ifndef _HIO_MODULE_SENSOR_H
#define _HIO_MODULE_SENSOR_H

#include <hio_gpio.h>
#include <hio_onewire.h>

//! @addtogroup hio_module_sensor hio_module_sensor
//! @brief Driver for Sensor Module
//! @{

//! @brief Sensor Module channels

typedef enum
{
    //! @brief Channel A
    HIO_MODULE_SENSOR_CHANNEL_A = 0,

    //! @brief Channel B
    HIO_MODULE_SENSOR_CHANNEL_B = 1,

    //! @brief Channel C
    HIO_MODULE_SENSOR_CHANNEL_C = 2

} hio_module_sensor_channel_t;

//! @brief Sensor module pull

typedef enum
{
    //! @brief Channel has no pull
    HIO_MODULE_SENSOR_PULL_NONE = HIO_GPIO_PULL_NONE,

    //! @brief Channel has internal pull-up
    HIO_MODULE_SENSOR_PULL_UP_INTERNAL = HIO_GPIO_PULL_UP,

    //! @brief Channel has internal pull-down
    HIO_MODULE_SENSOR_PULL_DOWN_INTERNAL = HIO_GPIO_PULL_DOWN,

    //! @brief Channel has pull-up 4k7
    HIO_MODULE_SENSOR_PULL_UP_4K7 = 3,

    //! @brief Channel has pull-up 56R
    HIO_MODULE_SENSOR_PULL_UP_56R = 4,

} hio_module_sensor_pull_t;

//! @brief Sensor Module mode of operation

typedef enum
{
    //! @brief Channel operates as input
    HIO_MODULE_SENSOR_MODE_INPUT = HIO_GPIO_MODE_INPUT,

    //! @brief Channel operates as output
    HIO_MODULE_SENSOR_MODE_OUTPUT = HIO_GPIO_MODE_OUTPUT

} hio_module_sensor_mode_t;

//! @brief Sensor Module hardware revision

typedef enum
{
    //! @brief Hardware revision unknown
    HIO_MODULE_SENSOR_REVISION_UNKNOWN = 0,

    //! @brief Hardware revision R1.0
    HIO_MODULE_SENSOR_REVISION_R1_0 = 1,

    //! @brief Hardware revision R1.1
    HIO_MODULE_SENSOR_REVISION_R1_1 = 2,

} hio_module_sensor_revision_t;

//! @brief Initialize Sensor Module
//! @return true On success
//! @return false On Error

bool hio_module_sensor_init(void);

//! @brief Deinitialize Sensor Module

void hio_module_sensor_deinit(void);

//! @brief Set pull of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] pull Sensor Module pull
//! @return true On success
//! @return false On error

bool hio_module_sensor_set_pull(hio_module_sensor_channel_t channel, hio_module_sensor_pull_t pull);

//! @brief Get pull-up/pull-down configuration for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Pull-up/pull-down configuration

hio_module_sensor_pull_t hio_module_sensor_get_pull(hio_module_sensor_channel_t channel);

//! @brief Set output mode of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] mode Desired mode of operation

void hio_module_sensor_set_mode(hio_module_sensor_channel_t channel, hio_module_sensor_mode_t mode);

//! @brief Get mode of operation for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Mode of operation

hio_module_sensor_mode_t hio_module_sensor_get_mode(hio_module_sensor_channel_t channel);

//! @brief Get input of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Input state

int hio_module_sensor_get_input(hio_module_sensor_channel_t channel);

//! @brief Set output state of Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @param[in] state State to be set

void hio_module_sensor_set_output(hio_module_sensor_channel_t channel, int state);

//! @brief Get output state for Sensor Module channel
//! @param[in] channel Sensor Module channel
//! @return Output state

int hio_module_sensor_get_output(hio_module_sensor_channel_t channel);

//! @brief Toggle output state for Sensor Module channel
//! @param[in] channel Sensor Module channel

void hio_module_sensor_toggle_output(hio_module_sensor_channel_t channel);

//! @brief Set VDD On / Off
//! @param[in] on On
//! @return true On success
//! @return false On error

bool hio_module_sensor_set_vdd(bool on);

//! @brief Get Sensor Module revision

hio_module_sensor_revision_t hio_module_sensor_get_revision(void);

//! @brief Initialize and get Instance 1-Wire for channel B
//! @return pointer on Instance 1-Wire

hio_onewire_t *hio_module_sensor_get_onewire(void);

//! @brief Semaphore for 1Wire Power up: for R1.1 set VDD On, for R1.0 pull up 56R on channel A
//! @return true On success
//! @return false On error

bool hio_module_sensor_onewire_power_up(void);

//! @brief Semaphore for 1Wire Power down: for R1.1 set VDD Off, for R1.0 pull none on channel A
//! @return true On success
//! @return false On error

bool hio_module_sensor_onewire_power_down(void);

//! @}


#endif // _HIO_MODULE_SENSOR_H
