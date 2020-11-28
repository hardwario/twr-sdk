#ifndef _HIO_OPT3001_H
#define _HIO_OPT3001_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_opt3001 hio_opt3001
//! @brief Driver for OPT3001 ambient light sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_OPT3001_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_OPT3001_EVENT_UPDATE = 1

} hio_opt3001_event_t;

//! @brief OPT3001 instance

typedef struct hio_opt3001_t hio_opt3001_t;

//! @cond

typedef enum
{
    HIO_OPT3001_STATE_ERROR = -1,
    HIO_OPT3001_STATE_INITIALIZE = 0,
    HIO_OPT3001_STATE_MEASURE = 1,
    HIO_OPT3001_STATE_READ = 2,
    HIO_OPT3001_STATE_UPDATE = 3

} hio_opt3001_state_t;

struct hio_opt3001_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_opt3001_t *, hio_opt3001_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    hio_tick_t _update_interval;
    hio_opt3001_state_t _state;
    hio_tick_t _tick_ready;
    bool _illuminance_valid;
    uint16_t _reg_result;
};

//! @endcond

//! @brief Initialize OPT3001 driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void hio_opt3001_init(hio_opt3001_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_opt3001_set_event_handler(hio_opt3001_t *self, void (*event_handler)(hio_opt3001_t *, hio_opt3001_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_opt3001_set_update_interval(hio_opt3001_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_opt3001_measure(hio_opt3001_t *self);

//! @brief Get measured illuminance as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_opt3001_get_illuminance_raw(hio_opt3001_t *self, uint16_t *raw);

//! @brief Get measured illuminance in lux
//! @param[in] self Instance
//! @param[in] lux Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_opt3001_get_illuminance_lux(hio_opt3001_t *self, float *lux);

//! @}

#endif // _HIO_OPT3001_H
