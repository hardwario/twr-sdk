#include <application.h>

// LED instance
hio_led_t led;

// Button instance
hio_button_t button;

// Led strip instance;
hio_led_strip_t led_strip;

// HC-SR04 instance
hio_hc_sr04_t hc_sr04;

float distance;

void hc_sr04_event_handler(hio_hc_sr04_t *self, hio_hc_sr04_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_HC_SR04_EVENT_UPDATE)
    {
        hio_hc_sr04_get_distance_millimeter(self, &distance);

        int centimeters = round(distance / 10.);

        if (centimeters > 144)
        {
            centimeters = 144;
        }

        for (int i = 0; i < 144; i++)
        {
            if (centimeters > (i + 1))
            {
            	hio_led_strip_set_pixel(&led_strip, i, 0xff000000);
            }
            else
            {
            	hio_led_strip_set_pixel(&led_strip, i, 0);
            }
        }

        hio_led_strip_write(&led_strip);
    }
}

void application_init(void)
{
    hio_module_power_init();

    hio_led_strip_init(&led_strip, hio_module_power_get_led_strip_driver(), &hio_module_power_led_strip_buffer_rgbw_144);

    // Initialize LED
    hio_led_init(&led, HIO_GPIO_LED, false, false);

    // Initialize button
    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button, button_event_handler, NULL);

    // Sensor modul:
    // Echo channel B
    // Trig channel A
    hio_hc_sr04_init(&hc_sr04, HIO_HC_SR04_ECHO_P5, HIO_GPIO_P4);
    hio_hc_sr04_set_event_handler(&hc_sr04, hc_sr04_event_handler, NULL);
    hio_hc_sr04_set_update_interval(&hc_sr04, 100);
}

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        hio_led_pulse(&led, 100);
        hio_hc_sr04_measure(&hc_sr04);
    }
}
