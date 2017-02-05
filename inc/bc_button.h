#ifndef _BC_BUTTON_H
#define _BC_BUTTON_H

#include <bc_gpio.h>
#include <bc_tick.h>

//! @addtogroup bc_button bc_button
//! @brief Driver for buttons
//! @{

//! @brief button callback events

typedef enum
{
    BC_BUTTON_EVENT_PRESS = 0,
    BC_BUTTON_EVENT_CLICK = 1,
    BC_BUTTON_EVENT_HOLD = 2

} bc_button_event_t;

//! @brief Button instance

typedef struct bc_button_t bc_button_t;

//! @cond

struct bc_button_t
{
    bc_gpio_channel_t _gpio_channel;
    bc_gpio_pull_t _gpio_pull;
    bool _idle_state;
    void (*_event_handler)(bc_button_t *, bc_button_event_t, void *);
    void *_event_param;
    bc_tick_t _scan_interval;
    bc_tick_t _debounce_time;
    bc_tick_t _click_timeout;
    bc_tick_t _hold_time;
    bc_tick_t _tick_debounce;
    bc_tick_t _tick_click_timeout;
    bc_tick_t _tick_hold_threshold;
    bool _state;
    bool _hold_signalized;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] bc_gpio_channel_t GPIO channel button is connected to
//! @param[in] bc_gpio_pull_t Enable no-pull, pull-up or pull-down
//! @param[in] idle_state GPIO pin idle state (when button is not pressed)

void bc_button_init(bc_button_t *self, bc_gpio_channel_t gpio_channel, bc_gpio_pull_t gpio_pull, bool idle_state);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_button_set_event_handler(bc_button_t *self, void (*event_handler)(bc_button_t *, bc_button_event_t, void *), void *event_param);

void bc_button_set_scan_interval(bc_button_t *self, bc_tick_t scan_interval);
void bc_button_set_debounce_time(bc_button_t *self, bc_tick_t debounce_time);
void bc_button_set_click_timeout(bc_button_t *self, bc_tick_t click_timeout);
void bc_button_set_hold_time(bc_button_t *self, bc_tick_t hold_time);

//! @}

#endif // _BC_BUTTON_H
