#ifndef _BC_TAG_LUX_METER_H
#define _BC_TAG_LUX_METER_H

#include <bc_opt3001.h>

//! @addtogroup bc_tag_lux_meter bc_tag_lux_meter
//! @brief Driver for BigClown Lux Meter Tag
//! @{

//! @brief I2C address

typedef enum
{
    //! @brief Default I2C address
    BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT = 0x44,

    //! @brief Alternate I2C address
    BC_TAG_LUX_METER_I2C_ADDRESS_ALTERNATE = 0x45

} bc_tag_lux_meter_i2c_address_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_TAG_LUX_METER_EVENT_ERROR = BC_OPT3001_EVENT_ERROR,

    //! @brief Update event
    BC_TAG_LUX_METER_EVENT_UPDATE = BC_OPT3001_EVENT_UPDATE

} bc_tag_lux_meter_event_t;

//! @brief BigClown Lux Meter Tag instance

typedef bc_opt3001_t bc_tag_lux_meter_t;

//! @brief Initialize BigClown Lux Meter Tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_tag_lux_meter_init(bc_tag_lux_meter_t *self, bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_tag_lux_meter_set_event_handler(bc_tag_lux_meter_t *self, void (*event_handler)(bc_tag_lux_meter_t *, bc_tag_lux_meter_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_tag_lux_meter_set_update_interval(bc_tag_lux_meter_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_tag_lux_meter_measure(bc_tag_lux_meter_t *self);

//! @brief Get measured illuminance as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_lux_meter_get_illuminance_raw(bc_tag_lux_meter_t *self, uint16_t *raw);

//! @brief Get measured illuminance in lux
//! @param[in] self Instance
//! @param[in] lux Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_lux_meter_get_illuminance_lux(bc_tag_lux_meter_t *self, float *lux);

//! @}

#endif // _BC_TAG_LUX_METER_H
