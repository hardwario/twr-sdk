#ifndef _BC_HC_SR04_H
#define _BC_HC_SR04_H

#include <bc_tick.h>
#include <bc_gpio.h>
#include <bc_scheduler.h>

//! @addtogroup bc_hc_sr04 bc_hc_sr04
//! @brief Driver for HC-SR04 ultrasonic range sensor
//! @{

typedef enum
{
    //! @brief Error event
    BC_HC_SR04_EVENT_ERROR = 0,

    //! @brief Update event
    BC_HC_SR04_EVENT_UPDATE = 1

} bc_hc_sr04_event_t;

typedef enum
{
    BC_HC_SR04_ECHO_P5 = BC_GPIO_P5,
    BC_HC_SR04_ECHO_P8 = BC_GPIO_P8,

} bc_hc_sr04_echo_t;


//! @brief HC-SR04 instance

typedef struct bc_hc_sr04_t bc_hc_sr04_t;

//! @cond

struct bc_hc_sr04_t
{
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_notify;
    void (*_event_handler)(bc_hc_sr04_t *, bc_hc_sr04_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bool _measurement_active;
    bool _measurement_valid;
    uint16_t _echo_duration;
    bc_hc_sr04_echo_t _echo;
    bc_gpio_channel_t _trig;
};

//! @endcond

//! @brief Initialize HC-SR04 for sensor module
//! @param[in] self Instance

void bc_hc_sr04_init_sensor_module(bc_hc_sr04_t *self);

//! @brief Initialize HC-SR04
//! @param[in] self Instance
//! @param[in] echo Pin
//! @param[in] trig Pin

void bc_hc_sr04_init(bc_hc_sr04_t *self, bc_gpio_channel_t trig, bc_hc_sr04_echo_t echo);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_hc_sr04_set_event_handler(bc_hc_sr04_t *self, void (*event_handler)(bc_hc_sr04_t *, bc_hc_sr04_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_hc_sr04_set_update_interval(bc_hc_sr04_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_hc_sr04_measure(bc_hc_sr04_t *self);

//! @brief Get measured distance in millimeters
//! @param[in] self Instance
//! @param[out] millimeter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_hc_sr04_get_distance_millimeter(bc_hc_sr04_t *self, float *millimeter);

//! @}

#endif // _BC_HC_SR04_H
