#ifndef _HIO_TMP112_H
#define _HIO_TMP112_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_tmp112 hio_tmp112
//! @brief Driver for TMP112 temperature sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_TMP112_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_TMP112_EVENT_UPDATE = 1

} hio_tmp112_event_t;

//! @brief TMP112 instance

typedef struct hio_tmp112_t hio_tmp112_t;

//! @cond

typedef enum
{
    HIO_TMP112_STATE_ERROR = -1,
    HIO_TMP112_STATE_INITIALIZE = 0,
    HIO_TMP112_STATE_MEASURE = 1,
    HIO_TMP112_STATE_READ = 2,
    HIO_TMP112_STATE_UPDATE = 3

} hio_tmp112_state_t;

struct hio_tmp112_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_tmp112_t *, hio_tmp112_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    hio_tick_t _update_interval;
    hio_tmp112_state_t _state;
    hio_tick_t _tick_ready;
    bool _temperature_valid;
    uint16_t _reg_temperature;
};

//! @endcond

//! @brief Initialize TMP112
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void hio_tmp112_init(hio_tmp112_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);


//! @brief Deinitialize TMP112
//! @param[in] self Instance

void hio_tmp112_deinit(hio_tmp112_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_tmp112_set_event_handler(hio_tmp112_t *self, void (*event_handler)(hio_tmp112_t *, hio_tmp112_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_tmp112_set_update_interval(hio_tmp112_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_tmp112_measure(hio_tmp112_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tmp112_get_temperature_raw(hio_tmp112_t *self, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[out] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tmp112_get_temperature_celsius(hio_tmp112_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[out] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tmp112_get_temperature_fahrenheit(hio_tmp112_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[out] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tmp112_get_temperature_kelvin(hio_tmp112_t *self, float *kelvin);

//! @}

#endif // _HIO_TMP112_H
