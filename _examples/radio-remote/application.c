#include <application.h>

// LED instance
hio_led_t led;

// Button instance
hio_button_t button;

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param);

void application_init(void)
{
    // Initialize LED
    hio_led_init(&led, HIO_GPIO_LED, false, false);

    // Initialize button
    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize radio
    hio_radio_init(HIO_RADIO_MODE_NODE_SLEEPING);
}

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        hio_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        hio_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == HIO_BUTTON_EVENT_HOLD)
    {
        hio_radio_pairing_request("demo", "v1.0.0");

        hio_led_pulse(&led, 1000);
    }
}
