#ifndef _BC_FLOOD_DETECTOR_H
#define _BC_FLOOD_DETECTOR_H

#include <bc_tick.h>
#include <bc_module_sensor.h>
#include <bc_scheduler.h>
#include <bc_gpio.h>

//! @addtogroup bc_flood_detector bc_flood_detector
//! @brief Driver flood detector
//! @{

//! @brief Type sensor

typedef enum
{
    BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A,
    BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B

} bc_flood_detector_type_t;

//! @brief Callback events

typedef enum
{
    BC_FLOOD_DETECTOR_EVENT_ERROR,
    BC_FLOOD_DETECTOR_EVENT_UPDATE,

} bc_flood_detector_event_t;

//! @brief Instance

typedef struct bc_flood_detector_t bc_flood_detector_t;

//! @cond

typedef enum
{
    BC_FLOOD_DETECTOR_STATE_ERROR = -1,
    BC_FLOOD_DETECTOR_STATE_INITIALIZE = 0,
    BC_FLOOD_DETECTOR_STATE_READY = 1,
    BC_FLOOD_DETECTOR_STATE_MEASURE = 2,

} bc_flood_detector_state_t;

struct bc_flood_detector_t
{
    bc_flood_detector_type_t _type;
    void (*_event_handler)(bc_flood_detector_t *, bc_flood_detector_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bc_flood_detector_state_t _state;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    bool _measurement_active;
    bool _alarm;

};

//! @endcond

//! @brief Initialize flood detector
//! @param[in] self Instance
//! @param[in] type senzor

void bc_flood_detector_init(bc_flood_detector_t *self, bc_flood_detector_type_t type);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_flood_detector_set_event_handler(bc_flood_detector_t *self, void (*event_handler)(bc_flood_detector_t *,bc_flood_detector_event_t, void*), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_flood_detector_set_update_interval(bc_flood_detector_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_flood_detector_measure(bc_flood_detector_t *self);

//! @brief Is alarm
//! @param[in] self Instance
//! @return true
//! @return false

bool bc_flood_detector_is_alarm(bc_flood_detector_t *self);

//! @}

#endif // _BC_FLOOD_DETECTOR_H
