#include <application.h>

// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

// Led strip instance;
twr_led_strip_t led_strip;

// HC-SR04 instance
twr_hc_sr04_t hc_sr04;

float distance;

void hc_sr04_event_handler(twr_hc_sr04_t *self, twr_hc_sr04_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_HC_SR04_EVENT_UPDATE)
    {
        twr_hc_sr04_get_distance_millimeter(self, &distance);

        int centimeters = round(distance / 10.);

        if (centimeters > 144)
        {
            centimeters = 144;
        }

        for (int i = 0; i < 144; i++)
        {
            if (centimeters > (i + 1))
            {
            	twr_led_strip_set_pixel(&led_strip, i, 0xff000000);
            }
            else
            {
            	twr_led_strip_set_pixel(&led_strip, i, 0);
            }
        }

        twr_led_strip_write(&led_strip);
    }
}

void application_init(void)
{
    twr_module_power_init();

    twr_led_strip_init(&led_strip, twr_module_power_get_led_strip_driver(), &twr_module_power_led_strip_buffer_rgbw_144);

    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Sensor modul:
    // Echo channel B
    // Trig channel A
    twr_hc_sr04_init(&hc_sr04, TWR_HC_SR04_ECHO_P5, TWR_GPIO_P4);
    twr_hc_sr04_set_event_handler(&hc_sr04, hc_sr04_event_handler, NULL);
    twr_hc_sr04_set_update_interval(&hc_sr04, 100);
}

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_led_pulse(&led, 100);
        twr_hc_sr04_measure(&hc_sr04);
    }
}
