#ifndef _TWR_TAG_LUX_METER_H
#define _TWR_TAG_LUX_METER_H

#include <twr_opt3001.h>

//! @addtogroup twr_tag_lux_meter twr_tag_lux_meter
//! @brief Driver for HARDWARIO Lux Meter Module
//! @{

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    TWR_TAG_LUX_METER_I2C_ADDRESS_DEFAULT = 0x44,

    //! @brief Alternate I2C address
    TWR_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE = 0x45

} twr_tag_lux_meter_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_TAG_LUX_METER_EVENT_ERROR = TWR_OPT3001_EVENT_ERROR,

    //! @brief Update event
    TWR_TAG_LUX_METER_EVENT_UPDATE = TWR_OPT3001_EVENT_UPDATE

} twr_tag_lux_meter_event_t;

//! @brief HARDWARIO Lux Meter Module instance

typedef twr_opt3001_t twr_tag_lux_meter_t;

//! @brief Initialize HARDWARIO Lux Meter Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_tag_lux_meter_init(twr_tag_lux_meter_t *self, twr_i2c_channel_t i2c_channel, twr_tag_lux_meter_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_tag_lux_meter_set_event_handler(twr_tag_lux_meter_t *self, void (*event_handler)(twr_tag_lux_meter_t *, twr_tag_lux_meter_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_tag_lux_meter_set_update_interval(twr_tag_lux_meter_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_tag_lux_meter_measure(twr_tag_lux_meter_t *self);

//! @brief Get measured illuminance as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_lux_meter_get_illuminance_raw(twr_tag_lux_meter_t *self, uint16_t *raw);

//! @brief Get measured illuminance in lux
//! @param[in] self Instance
//! @param[in] lux Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_lux_meter_get_illuminance_lux(twr_tag_lux_meter_t *self, float *lux);

//! @}

#endif // _TWR_TAG_LUX_METER_H
