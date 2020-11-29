#include <application.h>

// LED instance
hio_led_t led;

// Button instance
hio_button_t button;

void application_init(void)
{
    // Initialize LED
    hio_led_init(&led, HIO_GPIO_LED, false, false);
    hio_led_set_mode(&led, HIO_LED_MODE_ON);

    // Initialize button
    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button, button_event_handler, NULL);
}

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        hio_led_set_mode(&led, HIO_LED_MODE_TOGGLE);
    }
}
