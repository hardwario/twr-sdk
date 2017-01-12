#ifndef _BC_TEMPERATURE_TAG_H
#define _BC_TEMPERATURE_TAG_H

#include <bc_tmp112.h>

//! @addtogroup bc_temperature_tag bc_temperature_tag
//! @brief Driver for temperature tag
//! @{

//! @brief I2C addresses for temperature tag

typedef enum
{
    BC_TEMPERATURE_TAG_I2C_ADDRESS_DEFAULT = 0x48,  //!< Default I2C address
    BC_TEMPERATURE_TAG_I2C_ADDRESS_ALTERNATE = 0x49 //!< Alternate I2C address

} bc_temperature_tag_i2c_address_t;

//! @brief Callback events

typedef bc_tmp112_event_t bc_temperature_tag_event_t;

//! @brief Temperature tag instance

typedef bc_tmp112_t bc_temperature_tag_t;

//! @brief Initialize temperature tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel ::bc_i2c_channel_t
//! @param[in] i2c_address Address of the I2C device

inline void bc_temperature_tag_init(bc_temperature_tag_t *self, bc_i2c_channel_t i2c_channel, bc_temperature_tag_i2c_address_t i2c_address)
{
    bc_tmp112_init(self, i2c_channel, (uint8_t) i2c_address);
}

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Pointer to the function

inline void bc_temperature_tag_set_event_handler(bc_temperature_tag_t *self, void (*event_handler)(bc_temperature_tag_t *, bc_temperature_tag_event_t))
{
    bc_tmp112_set_event_handler(self, event_handler);
}

//! @brief Set update interval of the measurement
//! @param[in] self Instance
//! @param[in] interval Measurement interval

inline void bc_temperature_tag_set_update_interval(bc_temperature_tag_t *self, bc_tick_t interval)
{
    bc_tmp112_set_update_interval(self, interval);
}

//! @brief Get measured temperature in raw values
//! @param[in] self Instance
//! @param[in] raw Pointer to the data buffer
//! @return true when value is valid
//! @return false when value is invalid

inline bool bc_temperature_tag_get_temperature_raw(bc_temperature_tag_t *self, int16_t *raw)
{
    return bc_tmp112_get_temperature_raw(self, raw);
}

//! @brief Get measured temperature in degrees of celsius
//! @param[in] self Instance
//! @param[in] celsius pointer to the variable
//! @return true when value is valid
//! @return false when value is invalid

inline bool bc_temperature_tag_get_temperature_celsius(bc_temperature_tag_t *self, float *celsius)
{
    return bc_tmp112_get_temperature_celsius(self, celsius);
}

//! @brief Get measured temperature in fahrenheit
//! @param[in] self Instance
//! @param[in] fahrenheit pointer to the variable
//! @return true when value is valid
//! @return false when value is invalid

inline bool bc_temperature_tag_get_temperature_fahrenheit(bc_temperature_tag_t *self, float *fahrenheit)
{
    return bc_tmp112_get_temperature_fahrenheit(self, fahrenheit);
}

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[in] kelvin pointer to the variable
//! @return true when value is valid
//! @return false when value is invalid

inline bool bc_temperature_tag_get_temperature_kelvin(bc_temperature_tag_t *self, float *kelvin)
{
    return bc_tmp112_get_temperature_kelvin(self, kelvin);
}

//! @}

#endif // _BC_TEMPERATURE_TAG_H
