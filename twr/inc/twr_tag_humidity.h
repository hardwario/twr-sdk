#ifndef _TWR_TAG_HUMIDITY_H
#define _TWR_TAG_HUMIDITY_H

#include <twr_hts221.h>
#include <twr_hdc2080.h>
#include <twr_sht20.h>
#include <twr_sht30.h>

//! @addtogroup twr_tag_humidity twr_tag_humidity
//! @brief Driver for HARDWARIO Humidity Module
//! @{

//! @brief Humidity Tag hardware revision

typedef enum
{
    //! @brief Hardware revision R1
    TWR_TAG_HUMIDITY_REVISION_R1 = 0,

    //! @brief Hardware revision R2
    TWR_TAG_HUMIDITY_REVISION_R2 = 1,

    //! @brief Hardware revision R3
    TWR_TAG_HUMIDITY_REVISION_R3 = 2,

    //! @brief Hardware revision R4
    TWR_TAG_HUMIDITY_REVISION_R4 = 3


} twr_tag_humidity_revision_t;

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    TWR_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT = 0,

    //! @brief Alternate I2C address
    TWR_TAG_HUMIDITY_I2C_ADDRESS_ALTERNATE = 1

} twr_tag_humidity_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_TAG_HUMIDITY_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_TAG_HUMIDITY_EVENT_UPDATE = 1

} twr_tag_humidity_event_t;

//! @brief HARDWARIO Humidity Module instance

typedef struct twr_tag_humidity_t twr_tag_humidity_t;

//! @cond

struct twr_tag_humidity_t
{
    twr_tag_humidity_revision_t _revision;
    void (*_event_handler)(twr_tag_humidity_t *, twr_tag_humidity_event_t, void *);
    void *_event_param;
    union
    {
        twr_hts221_t hts221;
        twr_hdc2080_t hdc2080;
        twr_sht20_t sht20;
        twr_sht30_t sht30;
    } _sensor;
};

//! @endcond

//! @brief Initialize HARDWARIO Humidity Module
//! @param[in] self Instance
//! @param[in] revision Hardware revision of connected Humidity Tag
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_tag_humidity_init(twr_tag_humidity_t *self, twr_tag_humidity_revision_t revision, twr_i2c_channel_t i2c_channel, twr_tag_humidity_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_tag_humidity_set_event_handler(twr_tag_humidity_t *self, void (*event_handler)(twr_tag_humidity_t *, twr_tag_humidity_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_tag_humidity_set_update_interval(twr_tag_humidity_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_tag_humidity_measure(twr_tag_humidity_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_humidity_get_temperature_raw(twr_tag_humidity_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_humidity_get_temperature_celsius(twr_tag_humidity_t *self, float *celsius);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_humidity_get_humidity_raw(twr_tag_humidity_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_humidity_get_humidity_percentage(twr_tag_humidity_t *self, float *percentage);

//! @}

#endif // _TWR_TAG_HUMIDITY_H
