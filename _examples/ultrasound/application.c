#include <application.h>

#include <bc_hc_sr04.h>
#include <bc_gpio.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Led strip instance;
bc_led_strip_t led_strip;

float distance;

void hc_sr04_event_handler(bc_hc_sr04_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_HC_SR04_EVENT_UPDATE)
    {
        bc_hc_sr04_get_distance_millimeter(&distance);

        int centimeters = round(distance / 10.);

        if (centimeters > 144)
        {
            centimeters = 144;
        }

        for (int i = 0; i < 144; i++)
        {
            if (centimeters > (i + 1))
            {
            	bc_led_strip_set_pixel(&led_strip, i, 0xff000000);
            }
            else
            {
            	bc_led_strip_set_pixel(&led_strip, i, 0);
            }
        }

        bc_led_strip_write(&led_strip);
    }
}

void application_init(void)
{
    bc_module_power_init();

    bc_led_strip_init(&led_strip, bc_module_power_get_led_strip_driver(), &bc_module_power_led_strip_buffer_rgbw_144);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_hc_sr04_init();
    bc_hc_sr04_set_event_handler(hc_sr04_event_handler, NULL);
    bc_hc_sr04_set_update_interval(100);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);
        bc_hc_sr04_measure();
    }
}
