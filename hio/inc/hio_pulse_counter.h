#ifndef BCL_INC_HIO_PULSE_COUNTER_H_
#define BCL_INC_HIO_PULSE_COUNTER_H_

#include <hio_gpio.h>
#include <hio_scheduler.h>
#include <hio_exti.h>
#include <hio_module_sensor.h>

//! @addtogroup hio_pulse_counter hio_pulse_counter
//! @brief Driver for generic pulse counter
//! @{

//! @brief Pulse counter active edges

typedef enum
{
    //! @brief Rise edge is active
    HIO_PULSE_COUNTER_EDGE_RISE = HIO_EXTI_EDGE_RISING,

    //! @brief Fall edge is active
    HIO_PULSE_COUNTER_EDGE_FALL = HIO_EXTI_EDGE_FALLING,

    //! @brief Rise and fall edges are active
    HIO_PULSE_COUNTER_EDGE_RISE_FALL = HIO_EXTI_EDGE_RISING_AND_FALLING

} hio_pulse_counter_edge_t;

//! @brief Pulse counter event

typedef enum
{
    //! @brief Update event
    HIO_PULSE_COUNTER_EVENT_UPDATE,

    //! @brief Overflow
    HIO_PULSE_COUNTER_EVENT_OVERFLOW

} hio_pulse_counter_event_t;

//! @brief Initialize pulse counter
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] edge Active edge

void hio_pulse_counter_init(hio_module_sensor_channel_t channel, hio_pulse_counter_edge_t edge);

//! @brief Set callback function
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_pulse_counter_set_event_handler(hio_module_sensor_channel_t channel, void (*event_handler)(hio_module_sensor_channel_t, hio_pulse_counter_event_t, void *), void *event_param);

//! @brief Set update interval
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] interval Update interval

void hio_pulse_counter_set_update_interval(hio_module_sensor_channel_t channel, hio_tick_t interval);

//! @brief Set count
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @param[in] count Count to be set

void hio_pulse_counter_set(hio_module_sensor_channel_t channel, unsigned int count);

//! @brief Get count
//! @param[in] channel Sensor Module channel pulse counter is connected to
//! @return Counter count

unsigned int hio_pulse_counter_get(hio_module_sensor_channel_t channel);

//! @brief Set count to zero
//! @param[in] channel Sensor Module channel pulse counter is connected to

void hio_pulse_counter_reset(hio_module_sensor_channel_t channel);

//! @}

#endif // BCL_INC_HIO_PULSE_COUNTER_H_
