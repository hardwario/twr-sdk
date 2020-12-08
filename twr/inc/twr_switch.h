#ifndef TWR_SWITCH_H
#define TWR_SWITCH_H

#include <twr_gpio.h>
#include <twr_tick.h>
#include <twr_scheduler.h>

//! @addtogroup twr_switch twr_switch
//! @brief Driver for switch
//! @{

#define TWR_SWITCH_OPEN false
#define TWR_SWITCH_CLOSE true

//! @brief Callback events

typedef enum
{
    //! @brief Event Opened
    TWR_SWITCH_EVENT_OPENED = 0,

    //! @brief Event Closed
    TWR_SWITCH_EVENT_CLOSED = 1

} twr_switch_event_t;

//! @brief Type

typedef enum
{
    //! @brief Type Normally Open
    TWR_SWITCH_TYPE_NO = 0,

    //!  @brief Type Normally Closed
    TWR_SWITCH_TYPE_NC = 1

} twr_switch_type_t;

//! @brief Pull

typedef enum
{
    //! @brief Pull None
    TWR_SWITCH_PULL_NONE = 0,

    //! @brief Pull Up
    TWR_SWITCH_PULL_UP = 1,

    //! @brief Pull Up dynamic (Turns pull only for measurement)
    TWR_SWITCH_PULL_UP_DYNAMIC = 2,

    //! @brief Pull Down
    TWR_SWITCH_PULL_DOWN = 3,

    //! @brief Pull Down dynamic (Turns pull only for measurement)
    TWR_SWITCH_PULL_DOWN_DYNAMIC = 4,

} twr_switch_pull_t;

//! @brief State

typedef struct twr_switch_t twr_switch_t;

//! @cond

typedef enum
{
    TWR_SWITCH_TASK_STATE_MEASURE = 0,
    TWR_SWITCH_TASK_STATE_SET_PULL = 1,

} twr_switch_task_state_t;

struct twr_switch_t
{
    twr_gpio_channel_t _channel;
    twr_switch_type_t _type;
    twr_switch_pull_t _pull;
    void (*_event_handler)(twr_switch_t *, twr_switch_event_t, void *);
    void *_event_param;
    twr_tick_t _update_interval;
    twr_switch_task_state_t _task_state;
    int _pin_state;
    twr_scheduler_task_id_t _task_id;
    twr_tick_t _scan_interval;
    twr_tick_t _debounce_time;
    twr_tick_t _tick_debounce;
    uint16_t _pull_advance_time;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] type Type
//! @param[in] pull Pull

void twr_switch_init(twr_switch_t *self, twr_gpio_channel_t channel, twr_switch_type_t type, twr_switch_pull_t pull);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_switch_set_event_handler(twr_switch_t *self, void (*event_handler)(twr_switch_t *,twr_switch_event_t, void*), void *event_param);

//! @brief Get state
//! @param[in] self Instance
//! @return true If Close
//! @return false If Open

bool twr_switch_get_state(twr_switch_t *self);

//! @brief Set scan interval (period of button input sampling), default 50ms
//! @param[in] self Instance
//! @param[in] scan_interval Desired scan interval in ticks

void twr_switch_set_scan_interval(twr_switch_t *self, twr_tick_t scan_interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state), default 20ms
//! @param[in] self Instance
//! @param[in] debounce_time Desired debounce time in ticks

void twr_switch_set_debounce_time(twr_switch_t *self, twr_tick_t debounce_time);

//! @brief Set pull advance time (work only with dynamic pull up/down), default 50us, for time bigger than 1000us is used scheduler
//! @param[in] self Instance
//! @param[in] pull_advance_time Pull advance time in us

void twr_switch_set_pull_advance_time(twr_switch_t *self, uint16_t pull_advance_time_us);

//! @}

#endif // TWR_SWITCH_H
