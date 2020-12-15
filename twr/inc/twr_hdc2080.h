#ifndef _TWR_HDC2080_H
#define _TWR_HDC2080_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_hdc2080 twr_hdc2080
//! @brief Driver for HDC2080 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_HDC2080_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_HDC2080_EVENT_UPDATE = 1

} twr_hdc2080_event_t;

//! @brief HDC2080 instance

typedef struct twr_hdc2080_t twr_hdc2080_t;

//! @cond

typedef enum
{
    TWR_HDC2080_STATE_ERROR = -1,
    TWR_HDC2080_STATE_INITIALIZE = 0,
    TWR_HDC2080_STATE_MEASURE = 1,
    TWR_HDC2080_STATE_READ = 2,
    TWR_HDC2080_STATE_UPDATE = 3

} twr_hdc2080_state_t;

struct twr_hdc2080_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_hdc2080_t *, twr_hdc2080_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_hdc2080_state_t _state;
    twr_tick_t _tick_ready;
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

void twr_hdc2080_init(twr_hdc2080_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deinitialize HDC2080
//! @param[in] self Instance

void twr_hdc2080_deinit(twr_hdc2080_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_hdc2080_set_event_handler(twr_hdc2080_t *self, void (*event_handler)(twr_hdc2080_t *, twr_hdc2080_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_hdc2080_set_update_interval(twr_hdc2080_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_hdc2080_measure(twr_hdc2080_t *self);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_hdc2080_get_humidity_raw(twr_hdc2080_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_hdc2080_get_humidity_percentage(twr_hdc2080_t *self, float *percentage);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_hdc2080_get_temperature_raw(twr_hdc2080_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_hdc2080_get_temperature_celsius(twr_hdc2080_t *self, float *celsius);

//! @}

#endif // _TWR_HDC2080_H
