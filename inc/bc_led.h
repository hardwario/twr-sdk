#ifndef _BC_LED_H
#define _BC_LED_H

#include <bc_common.h>
#include <bc_gpio.h>

typedef enum
{
    BC_LED_MODE_OFF = 0,
    BC_LED_MODE_ON = 1,
    BC_LED_MODE_BLINK = 2

} bc_led_mode_t;

typedef struct bc_led_t bc_led_t;

struct bc_led_t
{
    bc_gpio_channel_t _gpio_channel;
    bool _open_drain_output;
    bool _idle_state;
    uint32_t _pattern;
    uint32_t _selector;
};

void bc_led_init(bc_led_t *self, bc_gpio_channel_t gpio_channel, bool open_drain_output, bool idle_state);
void bc_led_set_mode(bc_led_t *self, bc_led_mode_t mode);

#endif /* _BC_LED_H */
