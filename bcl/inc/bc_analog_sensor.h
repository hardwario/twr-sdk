#ifndef _BC_ANALOG_SENSOR_H
#define _BC_ANALOG_SENSOR_H

#include <bc_adc.h>
#include <bc_scheduler.h>

//! @addtogroup bc_analog_sensor bc_analog_sensor
//! @brief Driver for generic analog sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_ANALOG_SENSOR_EVENT_ERROR = 0,

    //! @brief Update event
    BC_ANALOG_SENSOR_EVENT_UPDATE = 1

} bc_analog_sensor_event_t;

//! @brief Analog sensor instance

typedef struct bc_analog_sensor_t bc_analog_sensor_t;

//! @brief Analog sensor driver interface

typedef struct
{
    //! @brief Callback for initialization
    void (*init)(bc_analog_sensor_t *self);

    //! @brief Callback for enabling analog sensor
    void (*enable)(bc_analog_sensor_t *self);

    //! @brief Callback for disabling analog sensor
    void (*disable)(bc_analog_sensor_t *self);

    //! @brief Callback for getting settling interval
    bc_tick_t (*get_settling_interval)(bc_analog_sensor_t *self);

} bc_analog_sensor_driver_t;

//! @cond

typedef enum
{
    BC_ANALOG_SENSOR_STATE_ERROR = -1,
    BC_ANALOG_SENSOR_STATE_ENABLE = 0,
    BC_ANALOG_SENSOR_STATE_MEASURE = 1,
    BC_ANALOG_SENSOR_STATE_DISABLE = 2,
    BC_ANALOG_SENSOR_STATE_UPDATE = 3

} bc_analog_sensor_state_t;

struct bc_analog_sensor_t
{
    bc_adc_channel_t _adc_channel;
    const bc_analog_sensor_driver_t *_driver;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_analog_sensor_t *, bc_analog_sensor_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_analog_sensor_state_t _state;
    uint16_t _value;
};

//! @endcond

//! @brief Initialize generic analog sensor
//! @param[in] self Instance
//! @param[in] adc_channel ADC channel
//! @param[in] adc_format ADC result format
//! @param[in] driver Optional driver interface (can be NULL)

void bc_analog_sensor_init(bc_analog_sensor_t *self, bc_adc_channel_t adc_channel, const bc_analog_sensor_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_analog_sensor_set_event_handler(bc_analog_sensor_t *self, void (*event_handler)(bc_analog_sensor_t *, bc_analog_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_analog_sensor_set_update_interval(bc_analog_sensor_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_analog_sensor_measure(bc_analog_sensor_t *self);

//! @brief Get measurement result
//! @param[in] self Instance
//! @param[out] result Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_analog_sensor_get_result(bc_analog_sensor_t *self, void *result);

//! @}

#endif // _BC_ANALOG_SENSOR_H
