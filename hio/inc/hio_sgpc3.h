#ifndef _HIO_SGPC3_H
#define _HIO_SGPC3_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_sgpc3 hio_sgpc3
//! @brief Driver for SGPC3 VOC gas sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_SGPC3_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_SGPC3_EVENT_UPDATE = 1

} hio_sgpc3_event_t;

//! @brief SGPC3 instance

typedef struct hio_sgpc3_t hio_sgpc3_t;

//! @cond

typedef enum
{
    HIO_SGPC3_STATE_ERROR = -1,
    HIO_SGPC3_STATE_INITIALIZE = 0,
    HIO_SGPC3_STATE_GET_FEATURE_SET = 1,
    HIO_SGPC3_STATE_READ_FEATURE_SET = 2,
    HIO_SGPC3_STATE_SET_POWER_MODE = 3,
    HIO_SGPC3_STATE_INIT_AIR_QUALITY = 4,
    HIO_SGPC3_STATE_SET_HUMIDITY = 5,
    HIO_SGPC3_STATE_MEASURE_AIR_QUALITY = 6,
    HIO_SGPC3_STATE_READ_AIR_QUALITY = 7

} hio_sgpc3_state_t;

struct hio_sgpc3_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_sgpc3_t *, hio_sgpc3_event_t, void *);
    void *_event_param;
    hio_tick_t _update_interval;
    hio_sgpc3_state_t _state;
    hio_tick_t _tick_ready;
    hio_tick_t _tick_last_measurement;
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

void hio_sgpc3_init(hio_sgpc3_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_sgpc3_set_event_handler(hio_sgpc3_t *self, void (*event_handler)(hio_sgpc3_t *, hio_sgpc3_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_sgpc3_set_update_interval(hio_sgpc3_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_sgpc3_measure(hio_sgpc3_t *self);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_sgpc3_get_tvoc_ppb(hio_sgpc3_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float hio_sgpc3_set_compensation(hio_sgpc3_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _HIO_SGPC3_H
