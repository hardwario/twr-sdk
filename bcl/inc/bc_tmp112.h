#ifndef _BC_TMP112_H
#define _BC_TMP112_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_tmp112 bc_tmp112
//! @brief Driver for TMP112 temperature sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_TMP112_EVENT_ERROR = 0,

    //! @brief Update event
    BC_TMP112_EVENT_UPDATE = 1

} bc_tmp112_event_t;

//! @brief TMP112 instance

typedef struct bc_tmp112_t bc_tmp112_t;

//! @cond

typedef enum
{
    BC_TMP112_STATE_ERROR = -1,
    BC_TMP112_STATE_INITIALIZE = 0,
    BC_TMP112_STATE_MEASURE = 1,
    BC_TMP112_STATE_READ = 2,
    BC_TMP112_STATE_UPDATE = 3

} bc_tmp112_state_t;

struct bc_tmp112_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_tmp112_t *, bc_tmp112_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_tmp112_state_t _state;
    bc_tick_t _tick_ready;
    bool _temperature_valid;
    uint16_t _reg_temperature;
};

//! @endcond

//! @brief Initialize TMP112
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_tmp112_init(bc_tmp112_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);


//! @brief Deinitialize TMP112
//! @param[in] self Instance

void bc_tmp112_deinit(bc_tmp112_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_tmp112_set_event_handler(bc_tmp112_t *self, void (*event_handler)(bc_tmp112_t *, bc_tmp112_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_tmp112_set_update_interval(bc_tmp112_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_tmp112_measure(bc_tmp112_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tmp112_get_temperature_raw(bc_tmp112_t *self, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[out] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tmp112_get_temperature_celsius(bc_tmp112_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[out] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tmp112_get_temperature_fahrenheit(bc_tmp112_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[out] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tmp112_get_temperature_kelvin(bc_tmp112_t *self, float *kelvin);

//! @}

#endif // _BC_TMP112_H
