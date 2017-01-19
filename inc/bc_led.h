#ifndef _BC_LED_H
#define _BC_LED_H

#include <bc_common.h>
#include <bc_gpio.h>
#include <bc_scheduler.h>

//! @addtogroup bc_led bc_led
//! @brief Driver for pcb LED
//! @section example Blink LED example
//! @code
//! bc_led_t led;
//!
//! bc_led_init(&led, BC_GPIO_LED, false, false);
//! bc_led_set_mode(&led, BC_LED_MODE_BLINK);
//! @endcode
//!
//!
//! @{

//! @brief LED modes
typedef enum
{
    BC_LED_MODE_OFF = 0,        //!< Keep LED off
    BC_LED_MODE_ON = 1,         //!< Keep LED on
    BC_LED_MODE_BLINK = 2,      //!< Blink LED
    BC_LED_MODE_BLINK_SLOW = 3, //!< Blink LED slow
    BC_LED_MODE_BLINK_FAST = 4, //!< Blink LED fast
    BC_LED_MODE_FLASH = 5       //!< Repeatedly flash LED

} bc_led_mode_t;

typedef struct bc_led_t bc_led_t;

//! @brief LED instance

struct bc_led_t
{
    bc_gpio_channel_t _gpio_channel;
    bool _open_drain_output;
    bool _idle_state;
    bc_tick_t _slot_interval;
    uint32_t _pattern;
    uint32_t _selector;
    bool _pulse_active;
    bc_scheduler_task_id_t _pulse_task_id;
};

//! @brief LED init function
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel which is connected to the LED
//! @param[in] open_drain_output Enable open drain output
//! @param[in] idle_state Set idle state when the LED is off

void bc_led_init(bc_led_t *self, bc_gpio_channel_t gpio_channel, bool open_drain_output, bool idle_state);

//! @brief Set LED slot interval for pattern
//! @param[in] self LED instance
//! @param[in] interval Desired slot interval in ticks

void bc_led_set_slot_interval(bc_led_t *self, bc_tick_t interval);

//! @brief Set LED mode
//! @param[in] self LED instance
//! @param[in] mode Desired LED mode

void bc_led_set_mode(bc_led_t *self, bc_led_mode_t mode);

//! @brief Set custom blinking pattern
//! @param[in] self LED instance
//! @param[in] pattern Blinking pattern (bits with log. 1 represent active slot)

void bc_led_set_pattern(bc_led_t *self, uint32_t pattern);

//! @brief Turn on LED for the specified duration of time
//! @param[in] self LED instance
//! @param[in] duration Number of ticks for which LED will be turned on

void bc_led_pulse(bc_led_t *self, bc_tick_t duration);

//! @}

#endif /* _BC_LED_H */
