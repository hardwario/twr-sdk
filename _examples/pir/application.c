#include <application.h>

// LED instance
hio_led_t led;

// PIR Module instance
hio_module_pir_t pir_module;

void application_init(void)
{
    // Initialize LED
    hio_led_init(&led, HIO_GPIO_LED, false, false);

    // Initialize PIR Module
    hio_module_pir_init(&pir_module);
    hio_module_pir_set_event_handler(&pir_module, pir_module_event_handler, NULL);
    hio_module_pir_set_sensitivity(&pir_module, HIO_MODULE_PIR_SENSITIVITY_MEDIUM);
}

void pir_module_event_handler(hio_module_pir_t *self, hio_module_pir_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    // If event is motion...
    if (event == HIO_MODULE_PIR_EVENT_MOTION)
    {
        // Generate 1 second LED pulse
        hio_led_pulse(&led, 1000);
    }
    // If event is error...
    else if (event == HIO_MODULE_PIR_EVENT_ERROR)
    {
        // Indicate sensor error by LED blinking
        hio_led_set_mode(&led, HIO_LED_MODE_BLINK_FAST);
    }
}
