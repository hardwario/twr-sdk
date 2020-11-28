#ifndef _HIO_HC_SR04_H
#define _HIO_HC_SR04_H

#include <hio_tick.h>
#include <hio_gpio.h>
#include <hio_scheduler.h>

//! @addtogroup hio_hc_sr04 hio_hc_sr04
//! @brief Driver for HC-SR04 ultrasonic range sensor
//! @{

typedef enum
{
    //! @brief Error event
    HIO_HC_SR04_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_HC_SR04_EVENT_UPDATE = 1

} hio_hc_sr04_event_t;

typedef enum
{
    HIO_HC_SR04_ECHO_P5 = HIO_GPIO_P5,
    HIO_HC_SR04_ECHO_P8 = HIO_GPIO_P8,

} hio_hc_sr04_echo_t;


//! @brief HC-SR04 instance

typedef struct hio_hc_sr04_t hio_hc_sr04_t;

//! @cond

struct hio_hc_sr04_t
{
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_notify;
    void (*_event_handler)(hio_hc_sr04_t *, hio_hc_sr04_event_t, void *);
    void *_event_param;
    hio_tick_t _update_interval;
    bool _measurement_active;
    bool _measurement_valid;
    uint16_t _echo_duration;
    hio_hc_sr04_echo_t _echo;
    hio_gpio_channel_t _trig;
};

//! @endcond

//! @brief Initialize HC-SR04 for sensor module
//! @param[in] self Instance

void hio_hc_sr04_init_sensor_module(hio_hc_sr04_t *self);

//! @brief Initialize HC-SR04
//! @param[in] self Instance
//! @param[in] echo Pin
//! @param[in] trig Pin

void hio_hc_sr04_init(hio_hc_sr04_t *self, hio_gpio_channel_t trig, hio_hc_sr04_echo_t echo);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_hc_sr04_set_event_handler(hio_hc_sr04_t *self, void (*event_handler)(hio_hc_sr04_t *, hio_hc_sr04_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_hc_sr04_set_update_interval(hio_hc_sr04_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_hc_sr04_measure(hio_hc_sr04_t *self);

//! @brief Get measured distance in millimeters
//! @param[in] self Instance
//! @param[out] millimeter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_hc_sr04_get_distance_millimeter(hio_hc_sr04_t *self, float *millimeter);

//! @}

#endif // _HIO_HC_SR04_H
