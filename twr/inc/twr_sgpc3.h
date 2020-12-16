#ifndef _TWR_SGPC3_H
#define _TWR_SGPC3_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_sgpc3 twr_sgpc3
//! @brief Driver for SGPC3 VOC gas sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_SGPC3_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_SGPC3_EVENT_UPDATE = 1

} twr_sgpc3_event_t;

//! @brief SGPC3 instance

typedef struct twr_sgpc3_t twr_sgpc3_t;

//! @cond

typedef enum
{
    TWR_SGPC3_STATE_ERROR = -1,
    TWR_SGPC3_STATE_INITIALIZE = 0,
    TWR_SGPC3_STATE_GET_FEATURE_SET = 1,
    TWR_SGPC3_STATE_READ_FEATURE_SET = 2,
    TWR_SGPC3_STATE_SET_POWER_MODE = 3,
    TWR_SGPC3_STATE_INIT_AIR_QUALITY = 4,
    TWR_SGPC3_STATE_SET_HUMIDITY = 5,
    TWR_SGPC3_STATE_MEASURE_AIR_QUALITY = 6,
    TWR_SGPC3_STATE_READ_AIR_QUALITY = 7

} twr_sgpc3_state_t;

struct twr_sgpc3_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_sgpc3_t *, twr_sgpc3_event_t, void *);
    void *_event_param;
    twr_tick_t _update_interval;
    twr_sgpc3_state_t _state;
    twr_tick_t _tick_ready;
    twr_tick_t _tick_last_measurement;
    bool _hit_error;
    bool _measurement_valid;
    uint16_t _tvoc;
    uint16_t _ah_scaled;
};

//! @endcond

//! @brief Initialize SGPC3
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_sgpc3_init(twr_sgpc3_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deinitialize SGPC3
//! @param[in] self Instance

void twr_sgpc3_deinit(twr_sgpc3_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_sgpc3_set_event_handler(twr_sgpc3_t *self, void (*event_handler)(twr_sgpc3_t *, twr_sgpc3_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_sgpc3_set_update_interval(twr_sgpc3_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_sgpc3_measure(twr_sgpc3_t *self);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sgpc3_get_tvoc_ppb(twr_sgpc3_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float twr_sgpc3_set_compensation(twr_sgpc3_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _TWR_SGPC3_H
