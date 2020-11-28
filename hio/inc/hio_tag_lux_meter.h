#ifndef _HIO_TAG_LUX_METER_H
#define _HIO_TAG_LUX_METER_H

#include <hio_opt3001.h>

//! @addtogroup hio_tag_lux_meter hio_tag_lux_meter
//! @brief Driver for HARDWARIO Lux Meter Module
//! @{

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    HIO_TAG_LUX_METER_I2C_ADDRESS_DEFAULT = 0x44,

    //! @brief Alternate I2C address
    HIO_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE = 0x45

} hio_tag_lux_meter_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_TAG_LUX_METER_EVENT_ERROR = HIO_OPT3001_EVENT_ERROR,

    //! @brief Update event
    HIO_TAG_LUX_METER_EVENT_UPDATE = HIO_OPT3001_EVENT_UPDATE

} hio_tag_lux_meter_event_t;

//! @brief HARDWARIO Lux Meter Module instance

typedef hio_opt3001_t hio_tag_lux_meter_t;

//! @brief Initialize HARDWARIO Lux Meter Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void hio_tag_lux_meter_init(hio_tag_lux_meter_t *self, hio_i2c_channel_t i2c_channel, hio_tag_lux_meter_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_tag_lux_meter_set_event_handler(hio_tag_lux_meter_t *self, void (*event_handler)(hio_tag_lux_meter_t *, hio_tag_lux_meter_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_tag_lux_meter_set_update_interval(hio_tag_lux_meter_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_tag_lux_meter_measure(hio_tag_lux_meter_t *self);

//! @brief Get measured illuminance as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_lux_meter_get_illuminance_raw(hio_tag_lux_meter_t *self, uint16_t *raw);

//! @brief Get measured illuminance in lux
//! @param[in] self Instance
//! @param[in] lux Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_lux_meter_get_illuminance_lux(hio_tag_lux_meter_t *self, float *lux);

//! @}

#endif // _HIO_TAG_LUX_METER_H
