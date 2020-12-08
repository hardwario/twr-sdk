#ifndef _TWR_HC_SR04_H
#define _TWR_HC_SR04_H

#include <twr_tick.h>
#include <twr_gpio.h>
#include <twr_scheduler.h>

//! @addtogroup twr_hc_sr04 twr_hc_sr04
//! @brief Driver for HC-SR04 ultrasonic range sensor
//! @{

typedef enum
{
    //! @brief Error event
    TWR_HC_SR04_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_HC_SR04_EVENT_UPDATE = 1

} twr_hc_sr04_event_t;

typedef enum
{
    TWR_HC_SR04_ECHO_P5 = TWR_GPIO_P5,
    TWR_HC_SR04_ECHO_P8 = TWR_GPIO_P8,

} twr_hc_sr04_echo_t;


//! @brief HC-SR04 instance

typedef struct twr_hc_sr04_t twr_hc_sr04_t;

//! @cond

struct twr_hc_sr04_t
{
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_notify;
    void (*_event_handler)(twr_hc_sr04_t *, twr_hc_sr04_event_t, void *);
    void *_event_param;
    twr_tick_t _update_interval;
    bool _measurement_active;
    bool _measurement_valid;
    uint16_t _echo_duration;
    twr_hc_sr04_echo_t _echo;
    twr_gpio_channel_t _trig;
};

//! @endcond

//! @brief Initialize HC-SR04 for sensor module
//! @param[in] self Instance

void twr_hc_sr04_init_sensor_module(twr_hc_sr04_t *self);

//! @brief Initialize HC-SR04
//! @param[in] self Instance
//! @param[in] echo Pin
//! @param[in] trig Pin

void twr_hc_sr04_init(twr_hc_sr04_t *self, twr_gpio_channel_t trig, twr_hc_sr04_echo_t echo);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_hc_sr04_set_event_handler(twr_hc_sr04_t *self, void (*event_handler)(twr_hc_sr04_t *, twr_hc_sr04_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_hc_sr04_set_update_interval(twr_hc_sr04_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_hc_sr04_measure(twr_hc_sr04_t *self);

//! @brief Get measured distance in millimeters
//! @param[in] self Instance
//! @param[out] millimeter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_hc_sr04_get_distance_millimeter(twr_hc_sr04_t *self, float *millimeter);

//! @}

#endif // _TWR_HC_SR04_H
