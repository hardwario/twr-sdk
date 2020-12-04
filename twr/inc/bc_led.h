#ifndef _BC_LED_H
#define _BC_LED_H

#include <bc_gpio.h>
#include <bc_scheduler.h>

//! @addtogroup bc_led bc_led
//! @brief Driver for generic LED
//! @section example Example: Blink with on-board LED
//! @code
//! #include <bc_led.h>
//!
//! bc_led_t led;
//!
//! void application_init(void)
//! {
//!     bc_led_init(&led, BC_GPIO_LED, false, false);
//!     bc_led_set_mode(&led, BC_LED_MODE_BLINK);
//! }
//! @endcode
//! @{

//! @brief LED modes

typedef enum
{
    //! @brief LED toggles between on/off state (this has no effect while processing alternating patterns)
    BC_LED_MODE_TOGGLE = 0,

    //! @brief LED has steady off state
    BC_LED_MODE_OFF = 1,

    //! @brief LED has steady on state
    BC_LED_MODE_ON = 2,

    //! @brief LED blinks
    BC_LED_MODE_BLINK = 3,

    //! @brief LED blinks slowly
    BC_LED_MODE_BLINK_SLOW = 4,

    //! @brief LED blinks quickly
    BC_LED_MODE_BLINK_FAST = 5,

    //! @brief LED flashes repeatedly
    BC_LED_MODE_FLASH = 6

} bc_led_mode_t;

//! @brief LED instance

typedef struct bc_led_t bc_led_t;

//! @brief LED driver interface

typedef struct
{
    //! @brief Callback for initialization
    void (*init)(bc_led_t *self);

    //! @brief Callback for setting LED on
    void (*on)(bc_led_t *self);

    //! @brief Callback for setting LED off
    void (*off)(bc_led_t *self);

} bc_led_driver_t;

//! @cond

typedef union
{
    bc_gpio_channel_t gpio;
    int virtual;

} bc_led_channel_t;

struct bc_led_t
{
    bc_led_channel_t _channel;
    const bc_led_driver_t *_driver;
    bool _open_drain_output;
    int _idle_state;
    bc_tick_t _slot_interval;
    uint32_t _pattern;
    uint32_t _selector;
    int _count;
    bool _pulse_active;
    bc_scheduler_task_id_t _task_id;
};

//! @endcond

//! @brief Initialize LED
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel LED is connected to
//! @param[in] open_drain_output Determines if LED is driven by open-drain output
//! @param[in] idle_state GPIO pin idle state (when LED is supposed to be off)

void bc_led_init(bc_led_t *self, bc_gpio_channel_t gpio_channel, bool open_drain_output, int idle_state);

//! @brief Initialize virtual LED
//! @param[in] self Instance
//! @param[in] channel Virtual channel LED is connected to
//! @param[in] driver Virtual channel LED driver
//! @param[in] idle_state Virtual pin idle state (when LED is supposed to be off)

void bc_led_init_virtual(bc_led_t *self, int channel, const bc_led_driver_t *driver, int idle_state);

//! @brief Set slot interval for pattern processing
//! @param[in] self Instance
//! @param[in] interval Desired slot interval in ticks

void bc_led_set_slot_interval(bc_led_t *self, bc_tick_t interval);

//! @brief Set LED mode
//! @param[in] self Instance
//! @param[in] mode Desired LED mode

void bc_led_set_mode(bc_led_t *self, bc_led_mode_t mode);

//! @brief Set custom blinking pattern
//! @param[in] self Instance
//! @param[in] pattern Blinking pattern (bits with log. 1 represent active slot)

void bc_led_set_pattern(bc_led_t *self, uint32_t pattern);

//! @brief Set count for blinking pattern executed
//! @param[in] self Instance
//! @param[in] count Count for blinking pattern executed, negative value means infinity

void bc_led_set_count(bc_led_t *self, int count);

//! @brief LED blink
//! @param[in] self Instance
//! @param[in] count Count of blink

void bc_led_blink(bc_led_t *self, int count);

//! @brief Turn on LED for the specified duration of time
//! @param[in] self Instance
//! @param[in] duration Duration for which LED will be turned on

void bc_led_pulse(bc_led_t *self, bc_tick_t duration);

//! @brief Check if there is ongoing LED pulse
//! @param[in] self Instance
//! @return true If pulse is now active
//! @return false If pulse is now inactive

bool bc_led_is_pulse(bc_led_t *self);

//! @}

#endif // _BC_LED_H
