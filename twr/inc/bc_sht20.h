#ifndef _TWR_SHT20_H
#define _TWR_SHT20_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_sht20 twr_sht20
//! @brief Driver for SHT20 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_SHT20_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_SHT20_EVENT_UPDATE = 1

} twr_sht20_event_t;

//! @brief SHT20 instance

typedef struct twr_sht20_t twr_sht20_t;

//! @cond

typedef enum
{
    TWR_SHT20_STATE_ERROR = -1,
    TWR_SHT20_STATE_INITIALIZE = 0,
    TWR_SHT20_STATE_MEASURE_RH = 1,
    TWR_SHT20_STATE_READ_RH = 2,
    TWR_SHT20_STATE_MEASURE_T = 3,
    TWR_SHT20_STATE_READ_T = 4,
    TWR_SHT20_STATE_UPDATE = 5

} twr_sht20_state_t;

struct twr_sht20_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_sht20_t *, twr_sht20_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_sht20_state_t _state;
    twr_tick_t _tick_ready;
    bool _humidity_valid;
    bool _temperature_valid;
    uint16_t _reg_humidity;
    uint16_t _reg_temperature;
};

//! @endcond

//! @brief Initialize SHT20
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_sht20_init(twr_sht20_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deinitialize SHT20
//! @param[in] self Instance
void twr_sht20_deinit(twr_sht20_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_sht20_set_event_handler(twr_sht20_t *self, void (*event_handler)(twr_sht20_t *, twr_sht20_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_sht20_set_update_interval(twr_sht20_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_sht20_measure(twr_sht20_t *self);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sht20_get_humidity_raw(twr_sht20_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sht20_get_humidity_percentage(twr_sht20_t *self, float *percentage);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sht20_get_temperature_raw(twr_sht20_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sht20_get_temperature_celsius(twr_sht20_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[out] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sht20_get_temperature_fahrenheit(twr_sht20_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[out] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sht20_get_temperature_kelvin(twr_sht20_t *self, float *kelvin);

//! @}

#endif // _TWR_SHT20_H
