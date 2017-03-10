#ifndef _BC_TAG_HUMIDITY_H
#define _BC_TAG_HUMIDITY_H

#include <bc_hts221.h>
#include <bc_hdc2080.h>

//! @addtogroup bc_tag_humidity bc_tag_humidity
//! @brief Driver for BigClown Humidity Tag
//! @{

//! @brief Humidity Tag hardware revision

typedef enum
{
    //! @brief Hardware revision R1
    BC_TAG_HUMIDITY_REVISION_R1 = 0,

    //! @brief Hardware revision R2
    BC_TAG_HUMIDITY_REVISION_R2 = 1

} bc_tag_humidity_revision_t;

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT = 0,

    //! @brief Alternate I2C address
    BC_TAG_HUMIDITY_I2C_ADDRESS_ALTERNATE = 1

} bc_tag_humidity_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_TAG_HUMIDITY_EVENT_ERROR = 0,

    //! @brief Update event
    BC_TAG_HUMIDITY_EVENT_UPDATE = 1

} bc_tag_humidity_event_t;

//! @brief BigClown Temperature Tag instance

typedef struct bc_tag_humidity_t bc_tag_humidity_t;

//! @cond

struct bc_tag_humidity_t
{
    bc_tag_humidity_revision_t _revision;
    void (*_event_handler)(bc_tag_humidity_t *, bc_tag_humidity_event_t, void *);
    void *_event_param;
    union
    {
        bc_hts221_t hts221;
        bc_hdc2080_t hdc2080;
    } _sensor;
};

//! @endcond

//! @brief Initialize BigClown Humidity Tag
//! @param[in] self Instance
//! @param[in] revision Hardware revision of connected Humidity Tag
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_tag_humidity_init(bc_tag_humidity_t *self, bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, bc_tag_humidity_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_tag_humidity_set_event_handler(bc_tag_humidity_t *self, void (*event_handler)(bc_tag_humidity_t *, bc_tag_humidity_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_tag_humidity_set_update_interval(bc_tag_humidity_t *self, bc_tick_t interval);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_humidity_get_temperature_raw(bc_tag_humidity_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_humidity_get_temperature_celsius(bc_tag_humidity_t *self, float *celsius);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_humidity_get_humidity_raw(bc_tag_humidity_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_humidity_get_humidity_percentage(bc_tag_humidity_t *self, float *percentage);

//! @}

#endif // _BC_TAG_HUMIDITY_H
