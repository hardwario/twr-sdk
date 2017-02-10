#include <application.h>

// LED instance
bc_led_t led;

// PIR Module instance
bc_module_pir_t pir_module;

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize PIR Module
    bc_module_pir_init(&pir_module);
    bc_module_pir_set_event_handler(&pir_module, pir_module_event_handler, NULL);
    bc_module_pir_set_sensitivity(&pir_module, BC_MODULE_PIR_SENSITIVITY_MEDIUM);
}

void pir_module_event_handler(bc_module_pir_t *self, bc_module_pir_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    // If event is motion...
    if (event == BC_MODULE_PIR_EVENT_MOTION)
    {
        // Generate 1 second LED pulse
        bc_led_pulse(&led, 1000);
    }
    // If event is error...
    else if (event == BC_MODULE_PIR_EVENT_ERROR)
    {
        // Indicate sensor error by LED blinking
        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
}
