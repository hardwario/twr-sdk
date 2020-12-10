#ifndef _TWR_TMP112_H
#define _TWR_TMP112_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_tmp112 twr_tmp112
//! @brief Driver for TMP112 temperature sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_TMP112_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_TMP112_EVENT_UPDATE = 1

} twr_tmp112_event_t;

//! @brief TMP112 instance

typedef struct twr_tmp112_t twr_tmp112_t;

//! @cond

typedef enum
{
    TWR_TMP112_STATE_ERROR = -1,
    TWR_TMP112_STATE_INITIALIZE = 0,
    TWR_TMP112_STATE_MEASURE = 1,
    TWR_TMP112_STATE_READ = 2,
    TWR_TMP112_STATE_UPDATE = 3

} twr_tmp112_state_t;

struct twr_tmp112_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_tmp112_t *, twr_tmp112_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_tmp112_state_t _state;
    twr_tick_t _tick_ready;
    bool _temperature_valid;
    uint16_t _reg_temperature;
};

//! @endcond

//! @brief Initialize TMP112
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_tmp112_init(twr_tmp112_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);


//! @brief Deinitialize TMP112
//! @param[in] self Instance

void twr_tmp112_deinit(twr_tmp112_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_tmp112_set_event_handler(twr_tmp112_t *self, void (*event_handler)(twr_tmp112_t *, twr_tmp112_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_tmp112_set_update_interval(twr_tmp112_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_tmp112_measure(twr_tmp112_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tmp112_get_temperature_raw(twr_tmp112_t *self, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[out] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tmp112_get_temperature_celsius(twr_tmp112_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[out] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tmp112_get_temperature_fahrenheit(twr_tmp112_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[out] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tmp112_get_temperature_kelvin(twr_tmp112_t *self, float *kelvin);

//! @}

#endif // _TWR_TMP112_H
