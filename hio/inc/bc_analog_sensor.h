#ifndef _HIO_ANALOG_SENSOR_H
#define _HIO_ANALOG_SENSOR_H

#include <hio_adc.h>
#include <hio_scheduler.h>

//! @addtogroup hio_analog_sensor hio_analog_sensor
//! @brief Driver for generic analog sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_ANALOG_SENSOR_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_ANALOG_SENSOR_EVENT_UPDATE = 1

} hio_analog_sensor_event_t;

//! @brief Analog sensor instance

typedef struct hio_analog_sensor_t hio_analog_sensor_t;

//! @brief Analog sensor driver interface

typedef struct
{
    //! @brief Callback for initialization
    void (*init)(hio_analog_sensor_t *self);

    //! @brief Callback for enabling analog sensor
    void (*enable)(hio_analog_sensor_t *self);

    //! @brief Callback for disabling analog sensor
    void (*disable)(hio_analog_sensor_t *self);

    //! @brief Callback for getting settling interval
    hio_tick_t (*get_settling_interval)(hio_analog_sensor_t *self);

} hio_analog_sensor_driver_t;

//! @cond

typedef enum
{
    HIO_ANALOG_SENSOR_STATE_ERROR = -1,
    HIO_ANALOG_SENSOR_STATE_ENABLE = 0,
    HIO_ANALOG_SENSOR_STATE_MEASURE = 1,
    HIO_ANALOG_SENSOR_STATE_DISABLE = 2,
    HIO_ANALOG_SENSOR_STATE_UPDATE = 3

} hio_analog_sensor_state_t;

struct hio_analog_sensor_t
{
    hio_adc_channel_t _adc_channel;
    const hio_analog_sensor_driver_t *_driver;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_analog_sensor_t *, hio_analog_sensor_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    hio_tick_t _update_interval;
    hio_analog_sensor_state_t _state;
    uint16_t _value;
};

//! @endcond

//! @brief Initialize generic analog sensor
//! @param[in] self Instance
//! @param[in] adc_channel ADC channel
//! @param[in] adc_format ADC result format
//! @param[in] driver Optional driver interface (can be NULL)

void hio_analog_sensor_init(hio_analog_sensor_t *self, hio_adc_channel_t adc_channel, const hio_analog_sensor_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_analog_sensor_set_event_handler(hio_analog_sensor_t *self, void (*event_handler)(hio_analog_sensor_t *, hio_analog_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_analog_sensor_set_update_interval(hio_analog_sensor_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_analog_sensor_measure(hio_analog_sensor_t *self);

//! @brief Get measurement result
//! @param[in] self Instance
//! @param[out] result Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_analog_sensor_get_result(hio_analog_sensor_t *self, void *result);

//! @}

#endif // _HIO_ANALOG_SENSOR_H
