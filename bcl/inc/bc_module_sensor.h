#ifndef _BC_MODULE_SENSOR_H
#define _BC_MODULE_SENSOR_H

#include <bc_gpio.h>
#include <bc_button.h>

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

} bc_module_channel_t;


//! @brief Sensor module pull

typedef enum
{
    BC_MODULE_PULL_NONE = 0,
    BC_MODULE_PULL_4K7 = 1,
    BC_MODULE_PULL_56 = 2,

} bc_module_pull_t;

//! @brief Initialize Sensor Module
//! @return true On success
//! @return false On Error

bool bc_module_sensor_init(void);

//! @brief Set mode of operation for Sensor module channel
//! @param[in] channel Sensor module channel
//! @param[in] mode Sensor module pull
//! @return true On success
//! @return false On Error

bool bc_module_sensor_set_pull(bc_module_channel_t channel, bc_module_pull_t pull);

//! @brief Set digital output mode of Sensor module
//! @param[in] channel Sensor module channel
//! @return true On success
//! @return false On Error
//

bool bc_module_sensor_set_digital_output_mode(bc_module_channel_t channel);

//! @brief Set output mode of Sensor module to button behaviour
//! @param[in] channel Sensor module channel
//! @return true On success
//! @return false On Error
//

bool bc_module_sensor_set_button_input_mode(bc_module_channel_t channel, void (*event_handler)(bc_button_t *, bc_button_event_t, void *));

//! @brief Set digital output of Sensor module channel to given value
//! @param[in] channel Sensor module channel
//! @param[in] value Value to be set
//

void bc_module_sensor_set_digital_output(bc_module_channel_t channel, bool value);

//! @}


#endif /* _BC_MODULE_SENSOR_H */
