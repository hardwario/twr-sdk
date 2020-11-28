#ifndef HIO_SWITCH_H
#define HIO_SWITCH_H

#include <hio_gpio.h>
#include <hio_tick.h>
#include <hio_scheduler.h>

//! @addtogroup hio_switch hio_switch
//! @brief Driver for switch
//! @{

#define HIO_SWITCH_OPEN false
#define HIO_SWITCH_CLOSE true

//! @brief Callback events

typedef enum
{
    //! @brief Event Opened
    HIO_SWITCH_EVENT_OPENED = 0,

    //! @brief Event Closed
    HIO_SWITCH_EVENT_CLOSED = 1

} hio_switch_event_t;

//! @brief Type

typedef enum
{
    //! @brief Type Normally Open
    HIO_SWITCH_TYPE_NO = 0,

    //!  @brief Type Normally Closed
    HIO_SWITCH_TYPE_NC = 1

} hio_switch_type_t;

//! @brief Pull

typedef enum
{
    //! @brief Pull None
    HIO_SWITCH_PULL_NONE = 0,

    //! @brief Pull Up
    HIO_SWITCH_PULL_UP = 1,

    //! @brief Pull Up dynamic (Turns pull only for measurement)
    HIO_SWITCH_PULL_UP_DYNAMIC = 2,

    //! @brief Pull Down
    HIO_SWITCH_PULL_DOWN = 3,

    //! @brief Pull Down dynamic (Turns pull only for measurement)
    HIO_SWITCH_PULL_DOWN_DYNAMIC = 4,

} hio_switch_pull_t;

//! @brief State

typedef struct hio_switch_t hio_switch_t;

//! @cond

typedef enum
{
    HIO_SWITCH_TASK_STATE_MEASURE = 0,
    HIO_SWITCH_TASK_STATE_SET_PULL = 1,

} hio_switch_task_state_t;

struct hio_switch_t
{
    hio_gpio_channel_t _channel;
    hio_switch_type_t _type;
    hio_switch_pull_t _pull;
    void (*_event_handler)(hio_switch_t *, hio_switch_event_t, void *);
    void *_event_param;
    hio_tick_t _update_interval;
    hio_switch_task_state_t _task_state;
    int _pin_state;
    hio_scheduler_task_id_t _task_id;
    hio_tick_t _scan_interval;
    hio_tick_t _debounce_time;
    hio_tick_t _tick_debounce;
    uint16_t _pull_advance_time;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] type Type
//! @param[in] pull Pull

void hio_switch_init(hio_switch_t *self, hio_gpio_channel_t channel, hio_switch_type_t type, hio_switch_pull_t pull);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_switch_set_event_handler(hio_switch_t *self, void (*event_handler)(hio_switch_t *,hio_switch_event_t, void*), void *event_param);

//! @brief Get state
//! @param[in] self Instance
//! @return true If Close
//! @return false If Open

bool hio_switch_get_state(hio_switch_t *self);

//! @brief Set scan interval (period of button input sampling), default 50ms
//! @param[in] self Instance
//! @param[in] scan_interval Desired scan interval in ticks

void hio_switch_set_scan_interval(hio_switch_t *self, hio_tick_t scan_interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state), default 20ms
//! @param[in] self Instance
//! @param[in] debounce_time Desired debounce time in ticks

void hio_switch_set_debounce_time(hio_switch_t *self, hio_tick_t debounce_time);

//! @brief Set pull advance time (work only with dynamic pull up/down), default 50us, for time bigger than 1000us is used scheduler
//! @param[in] self Instance
//! @param[in] pull_advance_time Pull advance time in us

void hio_switch_set_pull_advance_time(hio_switch_t *self, uint16_t pull_advance_time_us);

//! @}

#endif // HIO_SWITCH_H
