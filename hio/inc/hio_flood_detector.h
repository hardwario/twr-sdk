#ifndef _HIO_FLOOD_DETECTOR_H
#define _HIO_FLOOD_DETECTOR_H

#include <hio_tick.h>
#include <hio_module_sensor.h>
#include <hio_scheduler.h>
#include <hio_gpio.h>

//! @addtogroup hio_flood_detector hio_flood_detector
//! @brief Driver flood detector
//! @{

//! @brief Type sensor

typedef enum
{
    HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A,
    HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B

} hio_flood_detector_type_t;

//! @brief Callback events

typedef enum
{
    HIO_FLOOD_DETECTOR_EVENT_ERROR,
    HIO_FLOOD_DETECTOR_EVENT_UPDATE,

} hio_flood_detector_event_t;

//! @brief Instance

typedef struct hio_flood_detector_t hio_flood_detector_t;

//! @cond

typedef enum
{
    HIO_FLOOD_DETECTOR_STATE_ERROR = -1,
    HIO_FLOOD_DETECTOR_STATE_INITIALIZE = 0,
    HIO_FLOOD_DETECTOR_STATE_READY = 1,
    HIO_FLOOD_DETECTOR_STATE_MEASURE = 2,

} hio_flood_detector_state_t;

struct hio_flood_detector_t
{
    hio_flood_detector_type_t _type;
    void (*_event_handler)(hio_flood_detector_t *, hio_flood_detector_event_t, void *);
    void *_event_param;
    hio_tick_t _update_interval;
    hio_flood_detector_state_t _state;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    bool _measurement_active;
    bool _alarm;

};

//! @endcond

//! @brief Initialize flood detector
//! @param[in] self Instance
//! @param[in] type senzor

void hio_flood_detector_init(hio_flood_detector_t *self, hio_flood_detector_type_t type);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_flood_detector_set_event_handler(hio_flood_detector_t *self, void (*event_handler)(hio_flood_detector_t *,hio_flood_detector_event_t, void*), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_flood_detector_set_update_interval(hio_flood_detector_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_flood_detector_measure(hio_flood_detector_t *self);

//! @brief Is alarm
//! @param[in] self Instance
//! @return true
//! @return false

bool hio_flood_detector_is_alarm(hio_flood_detector_t *self);

//! @}

#endif // _HIO_FLOOD_DETECTOR_H
