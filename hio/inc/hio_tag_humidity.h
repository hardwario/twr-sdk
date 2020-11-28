#ifndef _HIO_TAG_HUMIDITY_H
#define _HIO_TAG_HUMIDITY_H

#include <hio_hts221.h>
#include <hio_hdc2080.h>
#include <hio_sht20.h>
#include <hio_sht30.h>

//! @addtogroup hio_tag_humidity hio_tag_humidity
//! @brief Driver for HARDWARIO Humidity Module
//! @{

//! @brief Humidity Tag hardware revision

typedef enum
{
    //! @brief Hardware revision R1
    HIO_TAG_HUMIDITY_REVISION_R1 = 0,

    //! @brief Hardware revision R2
    HIO_TAG_HUMIDITY_REVISION_R2 = 1,

    //! @brief Hardware revision R3
    HIO_TAG_HUMIDITY_REVISION_R3 = 2,

    //! @brief Hardware revision R4
    HIO_TAG_HUMIDITY_REVISION_R4 = 3


} hio_tag_humidity_revision_t;

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    HIO_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT = 0,

    //! @brief Alternate I2C address
    HIO_TAG_HUMIDITY_I2C_ADDRESS_ALTERNATE = 1

} hio_tag_humidity_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_TAG_HUMIDITY_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_TAG_HUMIDITY_EVENT_UPDATE = 1

} hio_tag_humidity_event_t;

//! @brief HARDWARIO Humidity Module instance

typedef struct hio_tag_humidity_t hio_tag_humidity_t;

//! @cond

struct hio_tag_humidity_t
{
    hio_tag_humidity_revision_t _revision;
    void (*_event_handler)(hio_tag_humidity_t *, hio_tag_humidity_event_t, void *);
    void *_event_param;
    union
    {
        hio_hts221_t hts221;
        hio_hdc2080_t hdc2080;
        hio_sht20_t sht20;
        hio_sht30_t sht30;
    } _sensor;
};

//! @endcond

//! @brief Initialize HARDWARIO Humidity Module
//! @param[in] self Instance
//! @param[in] revision Hardware revision of connected Humidity Tag
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void hio_tag_humidity_init(hio_tag_humidity_t *self, hio_tag_humidity_revision_t revision, hio_i2c_channel_t i2c_channel, hio_tag_humidity_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_tag_humidity_set_event_handler(hio_tag_humidity_t *self, void (*event_handler)(hio_tag_humidity_t *, hio_tag_humidity_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_tag_humidity_set_update_interval(hio_tag_humidity_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_tag_humidity_measure(hio_tag_humidity_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_humidity_get_temperature_raw(hio_tag_humidity_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_humidity_get_temperature_celsius(hio_tag_humidity_t *self, float *celsius);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_humidity_get_humidity_raw(hio_tag_humidity_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_humidity_get_humidity_percentage(hio_tag_humidity_t *self, float *percentage);

//! @}

#endif // _HIO_TAG_HUMIDITY_H
