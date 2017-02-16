#include <application.h>
#include <usb_talk.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

void application_init(void)
{
    usb_talk_init();

    bc_module_power_init();

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize radio
    bc_radio_init();
    bc_radio_set_event_handler(radio_event_handler, NULL);
    bc_radio_listen();
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        usb_talk_publish_push_button("base/", &event_count);

        event_count++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enrollment_start();

        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
}

void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_RADIO_EVENT_PAIR_SUCCESS)
    {
        bc_radio_enrollment_stop();

        bc_led_pulse(&led, 1000);

        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
}

void bc_radio_on_push_button(uint16_t *event_count)
{
    bc_led_pulse(&led, 10);

    usb_talk_publish_push_button("remote/", event_count);
}

void bc_radio_on_thermometer(float *temperature)
{ 
    bc_led_pulse(&led, 10);

    if (temperature != NULL && *temperature >= 27.0)
    {
        bc_module_power_led_strip_test();
    }
    
    usb_talk_publish_thermometer("remote/", temperature);
}