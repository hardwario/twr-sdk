#ifndef _BC_MODULE_SENSOR_H
#define _BC_MODULE_SENSOR_H

#include <bc_gpio.h>

//! @addtogroup bc_module_sensor bc_module_sensor
//! @brief Driver for Sensor Module
//! @{

//! @brief Sensor module channels

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
    //! @brief Channel without pull-up
    BC_MODULE_SENSOR_PULL_NONE = 0,

    //! @brief Channel with pull-up 4k7
    BC_MODULE_SENSOR_PULL_UP_4K7 = 1,

    //! @brief Channel with pull-up 56R
    BC_MODULE_SENSOR_PULL_UP_56R = 2

} bc_module_sensor_pull_t;

//! @brief Initialize Sensor Module
//! @return true On success
//! @return false On Error

bool bc_module_sensor_init(void);

//! @brief Set mode of operation for Sensor module channel
//! @param[in] channel Sensor module channel
//! @param[in] mode Sensor module pull
//! @return true On success
//! @return false On Error

bool bc_module_sensor_set_pull(bc_module_sensor_channel_t channel, bc_module_sensor_pull_t pull);

//! @brief Set digital output mode of Sensor module
//! @param[in] channel Sensor module channel
//! @return true On success
//! @return false On Error

bool bc_module_sensor_set_digital_output_mode(bc_module_sensor_channel_t channel);

//! @brief Set digital output of Sensor module channel to given value
//! @param[in] channel Sensor module channel
//! @param[in] value Value to be set

void bc_module_sensor_set_digital_output(bc_module_sensor_channel_t channel, bool value);

//! @}

#endif /* _BC_MODULE_SENSOR_H */
