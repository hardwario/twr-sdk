#ifndef _BC_OPT3001_H
#define _BC_OPT3001_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_opt3001 bc_opt3001
//! @brief Driver for OPT3001 ambient light sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_OPT3001_EVENT_ERROR = 0,

    //! @brief Update event
    BC_OPT3001_EVENT_UPDATE = 1

} bc_opt3001_event_t;

//! @brief OPT3001 instance

typedef struct bc_opt3001_t bc_opt3001_t;

//! @cond

typedef enum
{
    BC_OPT3001_STATE_ERROR = -1,
    BC_OPT3001_STATE_INITIALIZE = 0,
    BC_OPT3001_STATE_MEASURE = 1,
    BC_OPT3001_STATE_READ = 2,
    BC_OPT3001_STATE_UPDATE = 3

} bc_opt3001_state_t;

struct bc_opt3001_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_opt3001_t *, bc_opt3001_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_opt3001_state_t _state;
    bc_tick_t _tick_ready;
    bool _illuminance_valid;
    uint16_t _reg_result;
};

//! @endcond

//! @brief Initialize OPT3001 driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_opt3001_init(bc_opt3001_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_opt3001_set_event_handler(bc_opt3001_t *self, void (*event_handler)(bc_opt3001_t *, bc_opt3001_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_opt3001_set_update_interval(bc_opt3001_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_opt3001_measure(bc_opt3001_t *self);

//! @brief Get measured illuminance as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_opt3001_get_illuminance_raw(bc_opt3001_t *self, uint16_t *raw);

//! @brief Get measured illuminance in lux
//! @param[in] self Instance
//! @param[in] lux Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_opt3001_get_illuminance_lux(bc_opt3001_t *self, float *lux);

//! @}

#endif // _BC_OPT3001_H
