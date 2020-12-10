#ifndef BCL_INC_TWR_PULSE_COUNTER_H_
#define BCL_INC_TWR_PULSE_COUNTER_H_

#include <twr_gpio.h>
#include <twr_scheduler.h>
#include <twr_exti.h>
#include <twr_module_sensor.h>

//! @addtogroup twr_pulse_counter twr_pulse_counter
//! @brief Driver for generic pulse counter
//! @{

//! @brief Pulse counter active edges

typedef enum
{
    //! @brief Rise edge is active
    TWR_PULSE_COUNTER_EDGE_RISE = TWR_EXTI_EDGE_RISING,

    //! @brief Fall edge is active
    TWR_PULSE_COUNTER_EDGE_FALL = TWR_EXTI_EDGE_FALLING,

    //! @brief Rise and fall edges are active
    TWR_PULSE_COUNTER_EDGE_RISE_FALL = TWR_EXTI_EDGE_RISING_AND_FALLING

} twr_pulse_counter_edge_t;

//! @brief Pulse counter event

typedef enum
{
    //! @brief Update event
    TWR_PULSE_COUNTER_EVENT_UPDATE,

    //! @brief Overflow
    TWR_PULSE_COUNTER_EVENT_OVERFLOW

} twr_pulse_counter_event_t;

//! @brief Initialize pulse counter
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] edge Active edge

void twr_pulse_counter_init(twr_module_sensor_channel_t channel, twr_pulse_counter_edge_t edge);

//! @brief Set callback function
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_pulse_counter_set_event_handler(twr_module_sensor_channel_t channel, void (*event_handler)(twr_module_sensor_channel_t, twr_pulse_counter_event_t, void *), void *event_param);

//! @brief Set update interval
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] interval Update interval

void twr_pulse_counter_set_update_interval(twr_module_sensor_channel_t channel, twr_tick_t interval);

//! @brief Set count
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] count Count to be set

void twr_pulse_counter_set(twr_module_sensor_channel_t channel, unsigned int count);

//! @brief Get count
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @return Counter count

unsigned int twr_pulse_counter_get(twr_module_sensor_channel_t channel);

//! @brief Set count to zero
//! @param[in] channel Sensor Module channel pulse counter is connected to

void twr_pulse_counter_reset(twr_module_sensor_channel_t channel);

//! @}

#endif // BCL_INC_TWR_PULSE_COUNTER_H_
