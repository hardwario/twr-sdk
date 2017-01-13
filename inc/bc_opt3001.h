#ifndef _BC_OPT3001_H
#define _BC_OPT3001_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_tick.h>

//! @addtogroup bc_opt3001 bc_opt3001
//! @brief Driver for OPT3001 ambient light sensor
//! @{

//! @brief Callback events

typedef enum
{
    BC_OPT3001_EVENT_ERROR = 0, //!< Error event
    BC_OPT3001_EVENT_UPDATE = 1 //!< Update event

} bc_opt3001_event_t;

//! @brief Library measuring states

typedef enum
{
    BC_OPT3001_STATE_ERROR = -1,
    BC_OPT3001_STATE_INITIALIZE = 0,
    BC_OPT3001_STATE_MEASURE = 1,
    BC_OPT3001_STATE_READ = 2,
    BC_OPT3001_STATE_UPDATE = 3

} bc_opt3001_state_t;

typedef struct bc_opt3001_t bc_opt3001_t;

//! @brief Structure of OPT3001 instance

struct bc_opt3001_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_opt3001_t *, bc_opt3001_event_t);
    bc_tick_t _update_interval;
    bc_opt3001_state_t _state;
    bool _luminosity_valid;
    uint16_t _reg_result;
};

//! @brief Initialize OPT3001 driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address Address of the I2C device

void bc_opt3001_init(bc_opt3001_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Pointer to the function

void bc_opt3001_set_event_handler(bc_opt3001_t *self, void (*event_handler)(bc_opt3001_t *, bc_opt3001_event_t));

//! @brief Set update interval of the measurement
//! @param[in] self Instance
//! @param[in] interval Measuring interval

void bc_opt3001_set_update_interval(bc_opt3001_t *self, bc_tick_t interval);

//! @brief Get measured luminosity in raw values
//! @param[in] self Instance
//! @param[in] raw pointer to the data buffer
//! @return true when value is valid
//! @return false when value is invalid

bool bc_opt3001_get_luminosity_raw(bc_opt3001_t *self, uint16_t *raw);

//! @brief Get measured luminosity in lux
//! @param[in] self Instance
//! @param[in] lux pointer to the variable
//! @return true when value is valid
//! @return false when value is invalid

bool bc_opt3001_get_luminosity_lux(bc_opt3001_t *self, float *lux);

//! @}

#endif // _BC_OPT3001_H
