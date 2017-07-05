#ifndef BCL_INC_BC_PULSE_COUNTER_H_
#define BCL_INC_BC_PULSE_COUNTER_H_

#include <bc_gpio.h>
#include <bc_scheduler.h>

//! @addtogroup bc_pulse_counter bc_pulse_counter
//! @brief Driver for generic pulse counter
//! @{

//! @brief Pulse counter active edges

typedef enum
{
    //! @brief Rise edge is active
	BC_COUNTER_EDGE_RISE,

    //! @brief Fall edge is active
	BC_COUNTER_EDGE_FALL,

    //! @brief Rise and fall edges are active
	BC_COUNTER_EDGE_RISE_FALL

} bc_pulse_counter_edge_t;

//! @brief Pulse counter event

typedef enum
{
    //! @brief Update event
    BC_COUNTER_EVENT_UPDATE

} bc_pulse_counter_event_t;

//! @brief Pulse counter instance

typedef struct bc_pulse_counter_t bc_pulse_counter_t;

//! @cond

typedef enum
{
    BC_COUNTER_STATE_IDLE,
    BC_COUNTER_STATE_DEBOUNCE

} bc_pulse_counter_state_t;

typedef union
{
    bc_gpio_channel_t gpio;
    int virtual;

} bc_pulse_counter_channel_t;

typedef struct
{
    void (*init)(bc_pulse_counter_t *self);
    int (*get_input)(bc_pulse_counter_t *self);

} bc_pulse_counter_driver_t;

struct bc_pulse_counter_t
{
    bc_pulse_counter_channel_t _channel;
    const bc_pulse_counter_driver_t *_driver;
    int _count;
    bc_pulse_counter_edge_t _edge;
    bc_tick_t _debounce_time;
    bc_tick_t _update_interval;
    void (*_event_handler)(bc_pulse_counter_t *, bc_pulse_counter_event_t, void *);
    void *_event_param;
    bc_scheduler_task_id_t _task_id_interval;
	bc_pulse_counter_state_t _state;
	int _idle;
	int _changed;
};

//! @endcond

//! @brief Initialize pulse counter
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel pulse counter is connected to
//! @param[in] edge Active edge

void bc_pulse_counter_init(bc_pulse_counter_t *self, bc_gpio_channel_t gpio_channel, bc_pulse_counter_edge_t edge);

//! @brief Initialize virtual pulse counter
//! @param[in] self Instance
//! @param[in] channel Virtual channel pulse counter is connected to
//! @param[in] driver Virtual pulse counter driver
//! @param[in] edge Active edge

void bc_pulse_counter_init_virtual(bc_pulse_counter_t *self, int channel, const bc_pulse_counter_driver_t *driver, bc_pulse_counter_edge_t edge);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_pulse_counter_set_event_handler(bc_pulse_counter_t *self, void (*event_handler)(bc_pulse_counter_t *, bc_pulse_counter_event_t, void *), void *event_param);

//! @brief Set update interval
//! @param[in] self Instance
//! @param[in] interval Update interval

void bc_pulse_counter_set_update_interval(bc_pulse_counter_t *self, bc_tick_t interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state)
//! @param[in] self Instance
//! @param[in] debounce_time Desired debounce time in ticks

void bc_pulse_counter_set_debounce_time(bc_pulse_counter_t *self, bc_tick_t debounce_time);

//! @brief Set count
//! @param[in] self Instance
//! @param[in] count Count to be set

void bc_pulse_counter_set(bc_pulse_counter_t *self, int count);

//! @brief Get count
//! @param[in] self Instance
//! @return Counter count

int bc_pulse_counter_get(bc_pulse_counter_t *self);

//! @brief Set count to zero
//! @param[in] self Instance

void bc_pulse_counter_reset(bc_pulse_counter_t *self);

//! @}

#endif /* BCL_INC_BC_PULSE_COUNTER_H_ */
