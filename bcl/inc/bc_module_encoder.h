#ifndef _BC_MODULE_ENCODER_H
#define _BC_MODULE_ENCODER_H

#include <bc_gpio.h>
#include <bc_button.h>
#include <bc_tick.h>

//! @addtogroup bc_module_encoder bc_module_encoder
//! @brief Driver for encoder module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Event button pressed
    BC_MODULE_ENCODER_EVENT_PRESS = 0,

    //! @brief Event button released
    BC_MODULE_ENCODER_EVENT_RELEASE = 1,

    //! @brief Event button clicked (pressed and released within certain time)
    BC_MODULE_ENCODER_EVENT_CLICK = 2,

    //! @brief Event button hold (pressed for longer time)
    BC_MODULE_ENCODER_EVENT_HOLD = 3,

    BC_MODULE_ENCODER_EVENT_UP = 4,

    BC_MODULE_ENCODER_EVENT_DOWN = 5

} bc_module_encoder_event_t;

//! @brief Button instance

typedef struct bc_module_encoder_t bc_module_encoder_t;

//! @cond

struct bc_module_encoder_t
{
    bc_button_t _button;
    uint8_t _encoder_last_state;
    void (*_event_handler)(bc_module_encoder_t *, bc_module_encoder_event_t, void *);
    void *_event_param;

    uint8_t value;
    uint8_t real;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel button is connected to
//! @param[in] gpio_pull GPIO pull-up/pull-down setting
//! @param[in] idle_state GPIO pin idle state (when button is not pressed)

void bc_module_encoder_init(bc_module_encoder_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_encoder_set_event_handler(bc_module_encoder_t *self, void (*event_handler)(bc_module_encoder_t *, bc_module_encoder_event_t, void *), void *event_param);

//! @brief Set scan interval (period of button input sampling)
//! @param[in] self Instance
//! @param[in] scan_interval Desired scan interval in ticks

void bc_module_encoder_set_scan_interval(bc_module_encoder_t *self, bc_tick_t scan_interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state)
//! @param[in] self Instance
//! @param[in] debounce_time Desired debounce time in ticks

void bc_module_encoder_set_debounce_time(bc_module_encoder_t *self, bc_tick_t debounce_time);

//! @brief Set click timeout (maximum interval within which button has to be released to recognize click event)
//! @param[in] self Instance
//! @param[in] click_timeout Desired click timeout in ticks

void bc_module_encoder_set_click_timeout(bc_module_encoder_t *self, bc_tick_t click_timeout);

//! @brief Set hold time (interval after which hold event is recognized when button is steadily pressed)
//! @param[in] self Instance
//! @param[in] hold_time Desired hold time in ticks

void bc_module_encoder_set_hold_time(bc_module_encoder_t *self, bc_tick_t hold_time);

//! @}

#endif // _BC_MODULE_ENCODER_H
