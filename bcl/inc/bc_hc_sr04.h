#ifndef _BC_HC_SR04_H
#define _BC_HC_SR04_H

#include <bc_tick.h>

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

void bc_hc_sr04_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_hc_sr04_set_event_handler(void (*event_handler)(bc_hc_sr04_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] interval Measurement interval

void bc_hc_sr04_set_update_interval(bc_tick_t interval);

//! @brief Start measurement manually
//! @return true On success
//! @return false When other measurement is in progress

bool bc_hc_sr04_measure(void);

//! @brief Get measured distance in millimeters
//! @param[in] millimeter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_hc_sr04_get_distance_millimeter(float *millimeter);

//! @}

#endif // _BC_HC_SR04_H
