#ifndef _BC_HC_SR04_H
#define _BC_HC_SR04_H

#include <bc_scheduler.h>
#include <bc_gpio.h>

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

typedef struct bc_hc_sr04_t bc_hc_sr04_t;

struct bc_hc_sr04_t
{
    bc_gpio_channel_t _channel_echo;
    bc_gpio_channel_t _channel_trig;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_hc_sr04_t *, bc_hc_sr04_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bool _measurement_active;
    bool _measurement_valid;
    uint16_t _echo_duration;
};

//! @brief Initialize HC-SR04 Ultrasonic Sensor
//! @param[in] self Instance
//! @param[in] channel_echo GPIO channel for Echo PIN
//! @param[in] channel_trig GPIO channel for Trig PIN

void bc_hc_sr04_init(bc_hc_sr04_t *self, bc_gpio_channel_t channel_echo, bc_gpio_channel_t channel_trig);

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
//! @param[in] millimeter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_hc_sr04_get_distance_millimeter(bc_hc_sr04_t *self, float *millimeter);

//! @}

#endif // _BC_HC_SR04_H
