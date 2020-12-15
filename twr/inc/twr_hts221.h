#ifndef _TWR_HTS221_H
#define _TWR_HTS221_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_hts221 twr_hts221
//! @brief Driver for HTS221 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_HTS221_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_HTS221_EVENT_UPDATE = 1

} twr_hts221_event_t;

//! @brief HTS221 instance

typedef struct twr_hts221_t twr_hts221_t;

//! @cond

typedef enum
{
    TWR_HTS221_STATE_ERROR = -1,
    TWR_HTS221_STATE_INITIALIZE = 0,
    TWR_HTS221_STATE_MEASURE = 1,
    TWR_HTS221_STATE_READ = 2,
    TWR_HTS221_STATE_UPDATE = 3

} twr_hts221_state_t;

struct twr_hts221_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_hts221_t *, twr_hts221_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_hts221_state_t _state;
    twr_tick_t _tick_ready;
    bool _humidity_valid;
    int16_t _reg_humidity;
    int16_t _h0_rh;
    int16_t _h0_t0_out;
    float _h_grad;
};

//! @endcond

//! @brief Initialize HTS221
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_hts221_init(twr_hts221_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deinitialize HTS221
//! @param[in] self Instance

void twr_hts221_deinit(twr_hts221_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_hts221_set_event_handler(twr_hts221_t *self, void (*event_handler)(twr_hts221_t *, twr_hts221_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_hts221_set_update_interval(twr_hts221_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_hts221_measure(twr_hts221_t *self);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_hts221_get_humidity_percentage(twr_hts221_t *self, float *percentage);

//! @}

#endif // _TWR_HTS221_H
