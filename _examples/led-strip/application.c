#include <application.h>
#include <bcl.h>

bc_led_t led;
bc_led_strip_t led_strip;


void application_init(void)
{
    usb_talk_init();
    bc_rtc_init();

    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);


    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);


    bc_module_power_init();
    bc_module_power_led_strip_init(&led_strip, &bc_module_power_led_strip_buffer_rgbw_144);

    bc_led_set_mode(&led, BC_LED_MODE_OFF);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_strip_effect_rainbow_cycle(&led_strip);
    }
    else if (event == BC_BUTTON_EVENT_RELEASE)
    {

    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_led_strip_test(&led_strip);
    }

}
