#ifndef _HIO_HTS221_H
#define _HIO_HTS221_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_hts221 hio_hts221
//! @brief Driver for HTS221 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_HTS221_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_HTS221_EVENT_UPDATE = 1

} hio_hts221_event_t;

//! @brief HTS221 instance

typedef struct hio_hts221_t hio_hts221_t;

//! @cond

typedef enum
{
    HIO_HTS221_STATE_ERROR = -1,
    HIO_HTS221_STATE_INITIALIZE = 0,
    HIO_HTS221_STATE_MEASURE = 1,
    HIO_HTS221_STATE_READ = 2,
    HIO_HTS221_STATE_UPDATE = 3

} hio_hts221_state_t;

struct hio_hts221_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_hts221_t *, hio_hts221_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    hio_tick_t _update_interval;
    hio_hts221_state_t _state;
    hio_tick_t _tick_ready;
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

void hio_hts221_init(hio_hts221_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_hts221_set_event_handler(hio_hts221_t *self, void (*event_handler)(hio_hts221_t *, hio_hts221_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_hts221_set_update_interval(hio_hts221_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_hts221_measure(hio_hts221_t *self);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_hts221_get_humidity_percentage(hio_hts221_t *self, float *percentage);

//! @}

#endif // _HIO_HTS221_H
