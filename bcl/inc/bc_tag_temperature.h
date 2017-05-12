#ifndef _BC_TAG_TEMPERATURE_H
#define _BC_TAG_TEMPERATURE_H

#include <bc_tmp112.h>

//! @addtogroup bc_tag_temperature bc_tag_temperature
//! @brief Driver for BigClown Temperature Tag
//! @{

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT = 0x48,

    //! @brief Alternate I2C address
    BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE = 0x49

} bc_tag_temperature_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_TAG_TEMPERATURE_EVENT_ERROR = BC_TMP112_EVENT_ERROR,

    //! @brief Update event
    BC_TAG_TEMPERATURE_EVENT_UPDATE = BC_TMP112_EVENT_UPDATE

} bc_tag_temperature_event_t;

//! @brief BigClown Temperature Tag instance

typedef bc_tmp112_t bc_tag_temperature_t;

//! @brief Initialize BigClown Temperature Tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_tag_temperature_init(bc_tag_temperature_t *self, bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_tag_temperature_set_event_handler(bc_tag_temperature_t *self, void (*event_handler)(bc_tag_temperature_t *, bc_tag_temperature_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_tag_temperature_set_update_interval(bc_tag_temperature_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_tag_temperature_measure(bc_tag_temperature_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_temperature_get_temperature_raw(bc_tag_temperature_t *self, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_temperature_get_temperature_celsius(bc_tag_temperature_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[in] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_temperature_get_temperature_fahrenheit(bc_tag_temperature_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[in] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_temperature_get_temperature_kelvin(bc_tag_temperature_t *self, float *kelvin);

//! @}

#endif // _BC_TAG_TEMPERATURE_H
