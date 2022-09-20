#include <application.h>

twr_led_t led;
twr_button_t button;

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_led_set_mode(&led, TWR_LED_MODE_OFF);
    } else if (event == TWR_BUTTON_EVENT_HOLD ) {
        twr_led_set_mode(&led,  TWR_LED_MODE_BLINK_FAST);
    }
}

void application_init(void)
{
    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_set_mode(&led, TWR_LED_MODE_OFF);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN,0);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Set HOLD time
    twr_button_set_hold_time(&button, 1500);
}

