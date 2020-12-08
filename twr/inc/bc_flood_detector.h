#ifndef _TWR_FLOOD_DETECTOR_H
#define _TWR_FLOOD_DETECTOR_H

#include <twr_tick.h>
#include <twr_module_sensor.h>
#include <twr_scheduler.h>
#include <twr_gpio.h>

//! @addtogroup twr_flood_detector twr_flood_detector
//! @brief Driver flood detector
//! @{

//! @brief Type sensor

typedef enum
{
    TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A,
    TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B

} twr_flood_detector_type_t;

//! @brief Callback events

typedef enum
{
    TWR_FLOOD_DETECTOR_EVENT_ERROR,
    TWR_FLOOD_DETECTOR_EVENT_UPDATE,

} twr_flood_detector_event_t;

//! @brief Instance

typedef struct twr_flood_detector_t twr_flood_detector_t;

//! @cond

typedef enum
{
    TWR_FLOOD_DETECTOR_STATE_ERROR = -1,
    TWR_FLOOD_DETECTOR_STATE_INITIALIZE = 0,
    TWR_FLOOD_DETECTOR_STATE_READY = 1,
    TWR_FLOOD_DETECTOR_STATE_MEASURE = 2,

} twr_flood_detector_state_t;

struct twr_flood_detector_t
{
    twr_flood_detector_type_t _type;
    void (*_event_handler)(twr_flood_detector_t *, twr_flood_detector_event_t, void *);
    void *_event_param;
    twr_tick_t _update_interval;
    twr_flood_detector_state_t _state;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    bool _measurement_active;
    bool _alarm;

};

//! @endcond

//! @brief Initialize flood detector
//! @param[in] self Instance
//! @param[in] type senzor

void twr_flood_detector_init(twr_flood_detector_t *self, twr_flood_detector_type_t type);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_flood_detector_set_event_handler(twr_flood_detector_t *self, void (*event_handler)(twr_flood_detector_t *,twr_flood_detector_event_t, void*), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_flood_detector_set_update_interval(twr_flood_detector_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_flood_detector_measure(twr_flood_detector_t *self);

//! @brief Is alarm
//! @param[in] self Instance
//! @return true
//! @return false

bool twr_flood_detector_is_alarm(twr_flood_detector_t *self);

//! @}

#endif // _TWR_FLOOD_DETECTOR_H
