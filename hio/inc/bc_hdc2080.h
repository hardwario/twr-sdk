#ifndef _HIO_HDC2080_H
#define _HIO_HDC2080_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_hdc2080 hio_hdc2080
//! @brief Driver for HDC2080 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_HDC2080_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_HDC2080_EVENT_UPDATE = 1

} hio_hdc2080_event_t;

//! @brief HDC2080 instance

typedef struct hio_hdc2080_t hio_hdc2080_t;

//! @cond

typedef enum
{
    HIO_HDC2080_STATE_ERROR = -1,
    HIO_HDC2080_STATE_INITIALIZE = 0,
    HIO_HDC2080_STATE_MEASURE = 1,
    HIO_HDC2080_STATE_READ = 2,
    HIO_HDC2080_STATE_UPDATE = 3

} hio_hdc2080_state_t;

struct hio_hdc2080_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_hdc2080_t *, hio_hdc2080_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    hio_tick_t _update_interval;
    hio_hdc2080_state_t _state;
    hio_tick_t _tick_ready;
    bool _humidity_valid;
    bool _temperature_valid;
    uint16_t _reg_humidity;
    uint16_t _reg_temperature;
};

//! @endcond

//! @brief Initialize HDC2080
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void hio_hdc2080_init(hio_hdc2080_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_hdc2080_set_event_handler(hio_hdc2080_t *self, void (*event_handler)(hio_hdc2080_t *, hio_hdc2080_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_hdc2080_set_update_interval(hio_hdc2080_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_hdc2080_measure(hio_hdc2080_t *self);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_hdc2080_get_humidity_raw(hio_hdc2080_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_hdc2080_get_humidity_percentage(hio_hdc2080_t *self, float *percentage);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_hdc2080_get_temperature_raw(hio_hdc2080_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_hdc2080_get_temperature_celsius(hio_hdc2080_t *self, float *celsius);

//! @}

#endif // _HIO_HDC2080_H
