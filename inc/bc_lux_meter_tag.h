#ifndef _BC_LUX_METER_TAG_H
#define _BC_LUX_METER_TAG_H

#include <bc_opt3001.h>

//! @addtogroup bc_lux_meter_tag bc_lux_meter_tag
//! @brief Driver for lux meter tag
//! @{

typedef enum
{
    BC_LUX_METER_TAG_I2C_ADDRESS_DEFAULT = 0x44,    //!< Default I2C address
    BC_LUX_METER_TAG_I2C_ADDRESS_ALTERNATE = 0x45   //!< Alternate I2C address

} bc_lux_meter_tag_i2c_address_t;

//! @brief Callback events

typedef bc_opt3001_event_t bc_lux_meter_tag_event_t;

//! @brief Temperature tag instance

typedef bc_opt3001_t bc_lux_meter_tag_t;

//! @brief Initialize lux meter tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address Address of the I2C device

inline void bc_lux_meter_tag_init(bc_lux_meter_tag_t *self, bc_i2c_channel_t i2c_channel, bc_lux_meter_tag_i2c_address_t i2c_address)
{
    bc_opt3001_init(self, i2c_channel, (uint8_t) i2c_address);
}

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Pointer to the function

inline void bc_lux_meter_tag_set_event_handler(bc_lux_meter_tag_t *self, void (*event_handler)(bc_lux_meter_tag_t *, bc_lux_meter_tag_event_t))
{
    bc_opt3001_set_event_handler(self, event_handler);
}

//! @brief Set update interval of the measurement
//! @param[in] self Instance
//! @param[in] interval Measuring interval

inline void bc_lux_meter_tag_set_update_interval(bc_lux_meter_tag_t *self, bc_tick_t interval)
{
    bc_opt3001_set_update_interval(self, interval);
}

//! @brief Get measured luminosity in raw values
//! @param[in] self Instance
//! @param[in] raw Pointer to the data buffer
//! @return true When value is valid
//! @return false When value is invalid

inline bool bc_lux_meter_tag_get_luminosity_raw(bc_lux_meter_tag_t *self, uint16_t *raw)
{
    return bc_opt3001_get_luminosity_raw(self, raw);
}

//! @brief Get measured luminosity in lux
//! @param[in] self Instance
//! @param[in] lux Pointer to the variable
//! @return true When value is valid
//! @return false When value is invalid

inline bool bc_lux_meter_tag_get_luminosity_lux(bc_lux_meter_tag_t *self, float *lux)
{
    return bc_opt3001_get_luminosity_lux(self, lux);
}

//! @}

#endif // _BC_LUX_METER_TAG_H
