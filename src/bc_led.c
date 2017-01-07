#include <bc_led.h>
#include <bc_scheduler.h>

// TODO Replace with parameter
#define LED_REFRESH_INTERVAL 100

static bc_tick_t bc_led_task(void *param);

void bc_led_init(bc_led_t *self, bc_gpio_channel_t gpio_channel, bool open_drain_output, bool idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_gpio_channel = gpio_channel;
    self->_open_drain_output = open_drain_output;
    self->_idle_state = idle_state;

    bc_gpio_init(self->_gpio_channel);

    bc_gpio_set_output(self->_gpio_channel, self->_idle_state);

    if (self->_open_drain_output)
    {
        bc_gpio_set_mode(self->_gpio_channel, BC_GPIO_MODE_OUTPUT_OD);
    }
    else
    {
        bc_gpio_set_mode(self->_gpio_channel, BC_GPIO_MODE_OUTPUT);
    }

    bc_scheduler_register(bc_led_task, self, LED_REFRESH_INTERVAL);
}

static bc_tick_t bc_led_task(void *param)
{
    bc_led_t *self = param;

    if (self->_selector == 0)
    {
        self->_selector = 0x80000000;
    }

    if ((self->_pattern & self->_selector) != 0)
    {
        bc_gpio_set_output(self->_gpio_channel, self->_idle_state ? false : true);
    }
    else
    {
        bc_gpio_set_output(self->_gpio_channel, self->_idle_state ? true : false);
    }

    self->_selector >>= 1;

    return LED_REFRESH_INTERVAL;
}

void bc_led_set_mode(bc_led_t *self, bc_led_mode_t mode)
{
    uint32_t pattern = 0;

    switch (mode)
    {
        case BC_LED_MODE_OFF:
        {
            break;
        }
        case BC_LED_MODE_ON:
        {
            pattern = 0xffffffff;
            break;
        }
        case BC_LED_MODE_BLINK:
        {
            pattern = 0xcccccccc;
            break;
        }
        default:
        {
            break;
        }
    }

    if (self->_pattern != pattern)
    {
        self->_pattern = pattern;
        self->_selector = 0;
    }
}
