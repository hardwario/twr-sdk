#ifndef _TWR_ANALOG_SENSOR_H
#define _TWR_ANALOG_SENSOR_H

#include <twr_adc.h>
#include <twr_scheduler.h>

//! @addtogroup twr_analog_sensor twr_analog_sensor
//! @brief Driver for generic analog sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_ANALOG_SENSOR_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_ANALOG_SENSOR_EVENT_UPDATE = 1

} twr_analog_sensor_event_t;

//! @brief Analog sensor instance

typedef struct twr_analog_sensor_t twr_analog_sensor_t;

//! @brief Analog sensor driver interface

typedef struct
{
    //! @brief Callback for initialization
    void (*init)(twr_analog_sensor_t *self);

    //! @brief Callback for enabling analog sensor
    void (*enable)(twr_analog_sensor_t *self);

    //! @brief Callback for disabling analog sensor
    void (*disable)(twr_analog_sensor_t *self);

    //! @brief Callback for getting settling interval
    twr_tick_t (*get_settling_interval)(twr_analog_sensor_t *self);

} twr_analog_sensor_driver_t;

//! @cond

typedef enum
{
    TWR_ANALOG_SENSOR_STATE_ERROR = -1,
    TWR_ANALOG_SENSOR_STATE_ENABLE = 0,
    TWR_ANALOG_SENSOR_STATE_MEASURE = 1,
    TWR_ANALOG_SENSOR_STATE_DISABLE = 2,
    TWR_ANALOG_SENSOR_STATE_UPDATE = 3

} twr_analog_sensor_state_t;

struct twr_analog_sensor_t
{
    twr_adc_channel_t _adc_channel;
    const twr_analog_sensor_driver_t *_driver;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_analog_sensor_t *, twr_analog_sensor_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_analog_sensor_state_t _state;
    uint16_t _value;
};

//! @endcond

//! @brief Initialize generic analog sensor
//! @param[in] self Instance
//! @param[in] adc_channel ADC channel
//! @param[in] adc_format ADC result format
//! @param[in] driver Optional driver interface (can be NULL)

void twr_analog_sensor_init(twr_analog_sensor_t *self, twr_adc_channel_t adc_channel, const twr_analog_sensor_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_analog_sensor_set_event_handler(twr_analog_sensor_t *self, void (*event_handler)(twr_analog_sensor_t *, twr_analog_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_analog_sensor_set_update_interval(twr_analog_sensor_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_analog_sensor_measure(twr_analog_sensor_t *self);

//! @brief Get measurement result
//! @param[in] self Instance
//! @param[out] result Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_analog_sensor_get_result(twr_analog_sensor_t *self, void *result);

//! @}

#endif // _TWR_ANALOG_SENSOR_H
