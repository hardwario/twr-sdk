#include <application.h>

hio_led_t led;
hio_button_t button;

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        hio_led_set_mode(&led, HIO_LED_MODE_OFF);
    } else if (event == HIO_BUTTON_EVENT_HOLD ) {
        hio_led_set_mode(&led,  HIO_LED_MODE_BLINK_FAST);
    }
}

void application_init(void)
{
    // Initialize LED
    hio_led_init(&led, HIO_GPIO_LED, false, false);
    hio_led_set_mode(&led, HIO_LED_MODE_OFF);

    // Initialize button
    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN,0);
    hio_button_set_event_handler(&button, button_event_handler, NULL);

    // Set HOLD time
    hio_button_set_hold_time(&button, 1500);
}

