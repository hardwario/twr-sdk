#include <application.h>

// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param);

void application_init(void)
{
    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize radio
    twr_radio_init(TWR_RADIO_MODE_NODE_SLEEPING);
}

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        twr_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == TWR_BUTTON_EVENT_HOLD)
    {
        twr_radio_pairing_request("demo", "v1.0.0");

        twr_led_pulse(&led, 1000);
    }
}
