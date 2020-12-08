#ifndef _TWR_TAG_TEMPERATURE_H
#define _TWR_TAG_TEMPERATURE_H

#include <twr_tmp112.h>

//! @addtogroup twr_tag_temperature twr_tag_temperature
//! @brief Driver for HARDWARIO Temperature Module
//! @{

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    TWR_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT = 0x48,

    //! @brief Alternate I2C address
    TWR_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE = 0x49

} twr_tag_temperature_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_TAG_TEMPERATURE_EVENT_ERROR = TWR_TMP112_EVENT_ERROR,

    //! @brief Update event
    TWR_TAG_TEMPERATURE_EVENT_UPDATE = TWR_TMP112_EVENT_UPDATE

} twr_tag_temperature_event_t;

//! @brief HARDWARIO Temperature Module instance

typedef twr_tmp112_t twr_tag_temperature_t;

//! @brief Initialize HARDWARIO Temperature Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_tag_temperature_init(twr_tag_temperature_t *self, twr_i2c_channel_t i2c_channel, twr_tag_temperature_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_tag_temperature_set_event_handler(twr_tag_temperature_t *self, void (*event_handler)(twr_tag_temperature_t *, twr_tag_temperature_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_tag_temperature_set_update_interval(twr_tag_temperature_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_tag_temperature_measure(twr_tag_temperature_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_temperature_get_temperature_raw(twr_tag_temperature_t *self, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_temperature_get_temperature_celsius(twr_tag_temperature_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[in] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_temperature_get_temperature_fahrenheit(twr_tag_temperature_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[in] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_temperature_get_temperature_kelvin(twr_tag_temperature_t *self, float *kelvin);

//! @}

#endif // _TWR_TAG_TEMPERATURE_H
