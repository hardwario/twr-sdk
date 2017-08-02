#ifndef BCL_INC_BC_PULSE_COUNTER_H_
#define BCL_INC_BC_PULSE_COUNTER_H_

#include <bc_gpio.h>
#include <bc_scheduler.h>
#include <bc_exti.h>
#include <bc_module_sensor.h>

//! @addtogroup bc_pulse_counter bc_pulse_counter
//! @brief Driver for generic pulse counter
//! @{

//! @brief Pulse counter active edges

typedef enum
{
    //! @brief Rise edge is active
    BC_PULSE_COUNTER_EDGE_RISE = BC_EXTI_EDGE_RISING,

    //! @brief Fall edge is active
    BC_PULSE_COUNTER_EDGE_FALL = BC_EXTI_EDGE_FALLING,

    //! @brief Rise and fall edges are active
    BC_PULSE_COUNTER_EDGE_RISE_FALL = BC_EXTI_EDGE_RISING_AND_FALLING

} bc_pulse_counter_edge_t;

//! @brief Pulse counter event

typedef enum
{
    //! @brief Update event
    BC_PULSE_COUNTER_EVENT_UPDATE,

    //! @brief Overflow
    BC_PULSE_COUNTER_EVENT_OVERFLOW

} bc_pulse_counter_event_t;

//! @brief Initialize pulse counter
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] edge Active edge

void bc_pulse_counter_init(bc_module_sensor_channel_t channel, bc_pulse_counter_edge_t edge);

//! @brief Set callback function
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_pulse_counter_set_event_handler(bc_module_sensor_channel_t channel, void (*event_handler)(bc_module_sensor_channel_t, bc_pulse_counter_event_t, void *), void *event_param);

//! @brief Set update interval
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] interval Update interval

void bc_pulse_counter_set_update_interval(bc_module_sensor_channel_t channel, bc_tick_t interval);

//! @brief Set count
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] count Count to be set

void bc_pulse_counter_set(bc_module_sensor_channel_t channel, unsigned int count);

//! @brief Get count
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @return Counter count

unsigned int bc_pulse_counter_get(bc_module_sensor_channel_t channel);

//! @brief Set count to zero
//! @param[in] channel Sensor Module channel pulse counter is connected to

void bc_pulse_counter_reset(bc_module_sensor_channel_t channel);

//! @}

#endif // BCL_INC_BC_PULSE_COUNTER_H_
