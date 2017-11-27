#include <application.h>

bc_led_t led;
bc_button_t button;

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    } else if (event == BC_BUTTON_EVENT_HOLD ) {
        bc_led_set_mode(&led,  BC_LED_MODE_BLINK_FAST);
    }
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN,0);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Set HOLD time
    bc_button_set_hold_time(&button, 1500);
}

