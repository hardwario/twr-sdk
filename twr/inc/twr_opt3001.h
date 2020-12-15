#ifndef _TWR_OPT3001_H
#define _TWR_OPT3001_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_opt3001 twr_opt3001
//! @brief Driver for OPT3001 ambient light sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_OPT3001_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_OPT3001_EVENT_UPDATE = 1

} twr_opt3001_event_t;

//! @brief OPT3001 instance

typedef struct twr_opt3001_t twr_opt3001_t;

//! @cond

typedef enum
{
    TWR_OPT3001_STATE_ERROR = -1,
    TWR_OPT3001_STATE_INITIALIZE = 0,
    TWR_OPT3001_STATE_MEASURE = 1,
    TWR_OPT3001_STATE_READ = 2,
    TWR_OPT3001_STATE_UPDATE = 3

} twr_opt3001_state_t;

struct twr_opt3001_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_opt3001_t *, twr_opt3001_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_opt3001_state_t _state;
    twr_tick_t _tick_ready;
    bool _illuminance_valid;
    uint16_t _reg_result;
};

//! @endcond

//! @brief Initialize OPT3001 driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_opt3001_init(twr_opt3001_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deinitialize OPT3001 driver
//! @param[in] self Instance

void twr_opt3001_deinit(twr_opt3001_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_opt3001_set_event_handler(twr_opt3001_t *self, void (*event_handler)(twr_opt3001_t *, twr_opt3001_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_opt3001_set_update_interval(twr_opt3001_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_opt3001_measure(twr_opt3001_t *self);

//! @brief Get measured illuminance as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_opt3001_get_illuminance_raw(twr_opt3001_t *self, uint16_t *raw);

//! @brief Get measured illuminance in lux
//! @param[in] self Instance
//! @param[in] lux Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_opt3001_get_illuminance_lux(twr_opt3001_t *self, float *lux);

//! @}

#endif // _TWR_OPT3001_H
