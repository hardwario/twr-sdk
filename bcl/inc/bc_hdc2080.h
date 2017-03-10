#ifndef _BC_HDC2080_H
#define _BC_HDC2080_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_hdc2080 bc_hdc2080
//! @brief Driver for HDC2080 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_HDC2080_EVENT_ERROR = 0,

    //! @brief Update event
    BC_HDC2080_EVENT_UPDATE = 1

} bc_hdc2080_event_t;

//! @brief HDC2080 instance

typedef struct bc_hdc2080_t bc_hdc2080_t;

//! @cond

typedef enum
{
    BC_HDC2080_STATE_ERROR = -1,
    BC_HDC2080_STATE_INITIALIZE = 0,
    BC_HDC2080_STATE_MEASURE = 1,
    BC_HDC2080_STATE_READ = 2,
    BC_HDC2080_STATE_UPDATE = 3

} bc_hdc2080_state_t;

struct bc_hdc2080_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_hdc2080_t *, bc_hdc2080_event_t, void *);
    void *_event_param;
    bc_scheduler_task_id_t _task_id;
    bc_tick_t _update_interval;
    bc_hdc2080_state_t _state;
    bool _temperature_valid;
    bool _humidity_valid;
    uint16_t _reg_temperature;
    uint16_t _reg_humidity;
};

//! @endcond

//! @brief Initialize HDC2080
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_hdc2080_init(bc_hdc2080_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_hdc2080_set_event_handler(bc_hdc2080_t *self, void (*event_handler)(bc_hdc2080_t *, bc_hdc2080_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_hdc2080_set_update_interval(bc_hdc2080_t *self, bc_tick_t interval);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_hdc2080_get_temperature_raw(bc_hdc2080_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_hdc2080_get_temperature_celsius(bc_hdc2080_t *self, float *celsius);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_hdc2080_get_humidity_raw(bc_hdc2080_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_hdc2080_get_humidity_percentage(bc_hdc2080_t *self, float *percentage);

//! @}

#endif // _BC_HDC2080_H
