#ifndef _HIO_SHT20_H
#define _HIO_SHT20_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_sht20 hio_sht20
//! @brief Driver for SHT20 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_SHT20_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_SHT20_EVENT_UPDATE = 1

} hio_sht20_event_t;

//! @brief SHT20 instance

typedef struct hio_sht20_t hio_sht20_t;

//! @cond

typedef enum
{
    HIO_SHT20_STATE_ERROR = -1,
    HIO_SHT20_STATE_INITIALIZE = 0,
    HIO_SHT20_STATE_MEASURE_RH = 1,
    HIO_SHT20_STATE_READ_RH = 2,
    HIO_SHT20_STATE_MEASURE_T = 3,
    HIO_SHT20_STATE_READ_T = 4,
    HIO_SHT20_STATE_UPDATE = 5

} hio_sht20_state_t;

struct hio_sht20_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_sht20_t *, hio_sht20_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    hio_tick_t _update_interval;
    hio_sht20_state_t _state;
    hio_tick_t _tick_ready;
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

void hio_sht20_init(hio_sht20_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deinitialize SHT20
//! @param[in] self Instance
void hio_sht20_deinit(hio_sht20_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_sht20_set_event_handler(hio_sht20_t *self, void (*event_handler)(hio_sht20_t *, hio_sht20_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_sht20_set_update_interval(hio_sht20_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_sht20_measure(hio_sht20_t *self);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_sht20_get_humidity_raw(hio_sht20_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_sht20_get_humidity_percentage(hio_sht20_t *self, float *percentage);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_sht20_get_temperature_raw(hio_sht20_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_sht20_get_temperature_celsius(hio_sht20_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[out] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_sht20_get_temperature_fahrenheit(hio_sht20_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[out] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_sht20_get_temperature_kelvin(hio_sht20_t *self, float *kelvin);

//! @}

#endif // _HIO_SHT20_H
