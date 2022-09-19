#include <application.h>

// LED instance
twr_led_t led;

// PIR Module instance
twr_module_pir_t pir_module;

void application_init(void)
{
    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);

    // Initialize PIR Module
    twr_module_pir_init(&pir_module);
    twr_module_pir_set_event_handler(&pir_module, pir_module_event_handler, NULL);
    twr_module_pir_set_sensitivity(&pir_module, TWR_MODULE_PIR_SENSITIVITY_MEDIUM);
}

void pir_module_event_handler(twr_module_pir_t *self, twr_module_pir_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    // If event is motion...
    if (event == TWR_MODULE_PIR_EVENT_MOTION)
    {
        // Generate 1 second LED pulse
        twr_led_pulse(&led, 1000);
    }
    // If event is error...
    else if (event == TWR_MODULE_PIR_EVENT_ERROR)
    {
        // Indicate sensor error by LED blinking
        twr_led_set_mode(&led, TWR_LED_MODE_BLINK_FAST);
    }
}
