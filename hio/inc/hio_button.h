#ifndef _HIO_BUTTON_H
#define _HIO_BUTTON_H

#include <hio_gpio.h>
#include <hio_tick.h>
#include <hio_scheduler.h>

//! @addtogroup hio_button hio_button
//! @brief Driver for generic button
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Event button pressed
    HIO_BUTTON_EVENT_PRESS = 0,

    //! @brief Event button released
    HIO_BUTTON_EVENT_RELEASE = 1,

    //! @brief Event button clicked (pressed and released within certain time)
    HIO_BUTTON_EVENT_CLICK = 2,

    //! @brief Event button hold (pressed for longer time)
    HIO_BUTTON_EVENT_HOLD = 3

} hio_button_event_t;

//! @brief Button instance

typedef struct hio_button_t hio_button_t;

//! @brief Button driver interface

typedef struct
{
    //! @brief Callback for initialization
    void (*init)(hio_button_t *self);

    //! @brief Callback for reading input state
    int (*get_input)(hio_button_t *self);

} hio_button_driver_t;

//! @cond

typedef union
{
    hio_gpio_channel_t gpio;
    int virtual;

} hio_button_channel_t;

struct hio_button_t
{
    hio_button_channel_t _channel;
    const hio_button_driver_t *_driver;
    hio_gpio_pull_t _gpio_pull;
    int _idle_state;
    void (*_event_handler)(hio_button_t *, hio_button_event_t, void *);
    void *_event_param;
    hio_tick_t _scan_interval;
    hio_tick_t _debounce_time;
    hio_tick_t _click_timeout;
    hio_tick_t _hold_time;
    hio_tick_t _tick_debounce;
    hio_tick_t _tick_click_timeout;
    hio_tick_t _tick_hold_threshold;
    int _state;
    bool _hold_signalized;
    hio_scheduler_task_id_t _task_id;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel button is connected to
//! @param[in] gpio_pull GPIO pull-up/pull-down setting
//! @param[in] idle_state GPIO pin idle state (when button is not pressed)

void hio_button_init(hio_button_t *self, hio_gpio_channel_t gpio_channel, hio_gpio_pull_t gpio_pull, int idle_state);

//! @brief Initialize virtual button
//! @param[in] self Instance
//! @param[in] channel Virtual channel button is connected to
//! @param[in] driver Virtual channel button driver
//! @param[in] idle_state Virtual pin idle state (when button is not pressed)

void hio_button_init_virtual(hio_button_t *self, int channel, const hio_button_driver_t *driver, int idle_state);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_button_set_event_handler(hio_button_t *self, void (*event_handler)(hio_button_t *, hio_button_event_t, void *), void *event_param);

//! @brief Set scan interval (period of button input sampling)
//! @param[in] self Instance
//! @param[in] scan_interval Desired scan interval in ticks

void hio_button_set_scan_interval(hio_button_t *self, hio_tick_t scan_interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state)
//! @param[in] self Instance
//! @param[in] debounce_time Desired debounce time in ticks

void hio_button_set_debounce_time(hio_button_t *self, hio_tick_t debounce_time);

//! @brief Set click timeout (maximum interval within which button has to be released to recognize click event)
//! @param[in] self Instance
//! @param[in] click_timeout Desired click timeout in ticks

void hio_button_set_click_timeout(hio_button_t *self, hio_tick_t click_timeout);

//! @brief Set hold time (interval after which hold event is recognized when button is steadily pressed)
//! @param[in] self Instance
//! @param[in] hold_time Desired hold time in ticks

void hio_button_set_hold_time(hio_button_t *self, hio_tick_t hold_time);

//! @}

#endif // _HIO_BUTTON_H
