#ifndef _BC_LED_H
#define _BC_LED_H

#include <bc_common.h>
#include <bc_gpio.h>

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
    BC_LED_MODE_OFF = 0,    //!< Turn LED off
    BC_LED_MODE_ON = 1,     //!< Turn LED on
    BC_LED_MODE_BLINK = 2   //!< Blink the LED

} bc_led_mode_t;

typedef struct bc_led_t bc_led_t;

//! @brief LED instance

struct bc_led_t
{
    bc_gpio_channel_t _gpio_channel;
    bool _open_drain_output;
    bool _idle_state;
    uint32_t _pattern;
    uint32_t _selector;
};

//! @brief LED init function
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel which is connected to the LED
//! @param[in] open_drain_output Enable open drain output
//! @param[in] idle_state Set idle state when the LED is off

void bc_led_init(bc_led_t *self, bc_gpio_channel_t gpio_channel, bool open_drain_output, bool idle_state);

//! @brief Set LED mode
//! @param[in] self Instance
//! @param[in] mode LED mode

void bc_led_set_mode(bc_led_t *self, bc_led_mode_t mode);

//! @}

#endif /* _BC_LED_H */
