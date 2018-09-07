#ifndef _BC_BUTTON_H
#define _BC_BUTTON_H

#include <bc_gpio.h>
#include <bc_tick.h>
#include <bc_scheduler.h>

//! @addtogroup bc_button bc_button
//! @brief Driver for generic button
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Event button pressed
    BC_BUTTON_EVENT_PRESS = 0,

    //! @brief Event button released
    BC_BUTTON_EVENT_RELEASE = 1,

    //! @brief Event button clicked (pressed and released within certain time)
    BC_BUTTON_EVENT_CLICK = 2,

    //! @brief Event button hold (pressed for longer time)
    BC_BUTTON_EVENT_HOLD = 3

} bc_button_event_t;

//! @brief Button instance

typedef struct bc_button_t bc_button_t;

//! @brief Button driver interface

typedef struct
{
    //! @brief Callback for initialization
    void (*init)(bc_button_t *self);

    //! @brief Callback for reading input state
    int (*get_input)(bc_button_t *self);

} bc_button_driver_t;

//! @cond

typedef union
{
    bc_gpio_channel_t gpio;
    int virtual;

} bc_button_channel_t;

struct bc_button_t
{
    bc_button_channel_t _channel;
    const bc_button_driver_t *_driver;
    bc_gpio_pull_t _gpio_pull;
    int _idle_state;
    void (*_event_handler)(bc_button_t *, bc_button_event_t, void *);
    void *_event_param;
    bc_tick_t _scan_interval;
    bc_tick_t _debounce_time;
    bc_tick_t _click_timeout;
    bc_tick_t _hold_time;
    bc_tick_t _tick_debounce;
    bc_tick_t _tick_click_timeout;
    bc_tick_t _tick_hold_threshold;
    int _state;
    bool _hold_signalized;
    bc_scheduler_task_id_t _task_id;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel button is connected to
//! @param[in] gpio_pull GPIO pull-up/pull-down setting
//! @param[in] idle_state GPIO pin idle state (when button is not pressed)

void bc_button_init(bc_button_t *self, bc_gpio_channel_t gpio_channel, bc_gpio_pull_t gpio_pull, int idle_state);

//! @brief Initialize virtual button
//! @param[in] self Instance
//! @param[in] channel Virtual channel button is connected to
//! @param[in] driver Virtual channel button driver
//! @param[in] idle_state Virtual pin idle state (when button is not pressed)

void bc_button_init_virtual(bc_button_t *self, int channel, const bc_button_driver_t *driver, int idle_state);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_button_set_event_handler(bc_button_t *self, void (*event_handler)(bc_button_t *, bc_button_event_t, void *), void *event_param);

//! @brief Set scan interval (period of button input sampling)
//! @param[in] self Instance
//! @param[in] scan_interval Desired scan interval in ticks

void bc_button_set_scan_interval(bc_button_t *self, bc_tick_t scan_interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state)
//! @param[in] self Instance
//! @param[in] debounce_time Desired debounce time in ticks

void bc_button_set_debounce_time(bc_button_t *self, bc_tick_t debounce_time);

//! @brief Set click timeout (maximum interval within which button has to be released to recognize click event)
//! @param[in] self Instance
//! @param[in] click_timeout Desired click timeout in ticks

void bc_button_set_click_timeout(bc_button_t *self, bc_tick_t click_timeout);

//! @brief Set hold time (interval after which hold event is recognized when button is steadily pressed)
//! @param[in] self Instance
//! @param[in] hold_time Desired hold time in ticks

void bc_button_set_hold_time(bc_button_t *self, bc_tick_t hold_time);

//! @}

#endif // _BC_BUTTON_H
