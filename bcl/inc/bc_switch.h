#ifndef BC_SWITCH_H
#define BC_SWITCH_H

#include <bc_gpio.h>
#include <bc_tick.h>
#include <bc_scheduler.h>

//! @addtogroup bc_switch bc_switch
//! @brief Driver for switch
//! @{

#define BC_SWITCH_OPEN false
#define BC_SWITCH_CLOSE true

//! @brief Callback events

typedef enum
{
    //! @brief Event Opened
    BC_SWITCH_EVENT_OPENED = 0,

    //! @brief Event Closed
    BC_SWITCH_EVENT_CLOSED = 1

} bc_switch_event_t;

//! @brief Type

typedef enum
{
    //! @brief Type Normally Open
    BC_SWITCH_TYPE_NO = 0,

    //!  @brief Type Normally Closed
    BC_SWITCH_TYPE_NC = 1

} bc_switch_type_t;

//! @brief Pull

typedef enum
{
    //! @brief Pull None
    BC_SWITCH_PULL_NONE = 0,

    //! @brief Pull Up
    BC_SWITCH_PULL_UP = 1,

    //! @brief Pull Up dynamic (Turns pull only for measurement)
    BC_SWITCH_PULL_UP_DYNAMIC = 2,

    //! @brief Pull Down
    BC_SWITCH_PULL_DOWN = 3,

    //! @brief Pull Down dynamic (Turns pull only for measurement)
    BC_SWITCH_PULL_DOWN_DYNAMIC = 4,

} bc_switch_pull_t;

//! @brief State

typedef struct bc_switch_t bc_switch_t;

//! @cond

typedef enum
{
    BC_SWITCH_TASK_STATE_MEASURE = 0,
    BC_SWITCH_TASK_STATE_SET_PULL = 1,

} bc_switch_task_state_t;

struct bc_switch_t
{
    bc_gpio_channel_t _channel;
    bc_switch_type_t _type;
    bc_switch_pull_t _pull;
    void (*_event_handler)(bc_switch_t *, bc_switch_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bc_switch_task_state_t _task_state;
    int _pin_state;
    bc_scheduler_task_id_t _task_id;
    bc_tick_t _scan_interval;
    bc_tick_t _debounce_time;
    bc_tick_t _tick_debounce;
    uint16_t _pull_advance_time;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] type Type
//! @param[in] pull Pull

void bc_switch_init(bc_switch_t *self, bc_gpio_channel_t channel, bc_switch_type_t type, bc_switch_pull_t pull);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_switch_set_event_handler(bc_switch_t *self, void (*event_handler)(bc_switch_t *,bc_switch_event_t, void*), void *event_param);

//! @brief Get state
//! @param[in] self Instance
//! @return true If Close
//! @return false If Open

bool bc_switch_get_state(bc_switch_t *self);

//! @brief Set scan interval (period of button input sampling), default 50ms
//! @param[in] self Instance
//! @param[in] scan_interval Desired scan interval in ticks

void bc_switch_set_scan_interval(bc_switch_t *self, bc_tick_t scan_interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state), default 20ms
//! @param[in] self Instance
//! @param[in] debounce_time Desired debounce time in ticks

void bc_switch_set_debounce_time(bc_switch_t *self, bc_tick_t debounce_time);

//! @brief Set pull advance time (work only with dynamic pull up/down), default 50us, for time bigger than 1000us is used scheduler
//! @param[in] self Instance
//! @param[in] pull_advance_time Pull advance time in us

void bc_switch_set_pull_advance_time(bc_switch_t *self, uint16_t pull_advance_time_us);

//! @}

#endif // BC_SWITCH_H
