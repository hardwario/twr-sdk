#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Temperature Tag instance
bc_tag_temperature_t temperature_tag;

// SigFox Module instance
bc_module_sigfox_t sigfox_module;

// Variable holding temperature in centigrades
float temperature;

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize Temperature Tag
    bc_tag_temperature_init(&temperature_tag, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    bc_tag_temperature_set_update_interval(&temperature_tag, 120000);
    bc_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    // Initialize SigFox Module
    bc_module_sigfox_init(&sigfox_module);
    bc_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        if (bc_module_sigfox_is_ready(&sigfox_module))
        {
            uint8_t buffer[] = { 0x01 };

            bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
        }
    }
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (bc_module_sigfox_is_ready(&sigfox_module))
        {
            // Read temperature
            bc_tag_temperature_get_temperature_celsius(self, &temperature);

            uint8_t buffer[5];

            buffer[0] = 0x02;
            buffer[1] = (uint32_t) temperature;
            buffer[2] = (uint32_t) temperature >> 8;
            buffer[3] = (uint32_t) temperature >> 16;
            buffer[4] = (uint32_t) temperature >> 24;

            bc_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
        }
    }
    else if (event == BC_TAG_TEMPERATURE_EVENT_ERROR)
    {
    }
}

void sigfox_module_event_handler(bc_module_sigfox_t *self, bc_module_sigfox_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    // If event is start of transmission...
    if (event == BC_MODULE_SIGFOX_EVENT_ERROR)
    {
        bc_led_set_mode(&led, BC_LED_MODE_BLINK);
    }
    // If event is start of transmission...
    if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START)
    {
        bc_led_set_mode(&led, BC_LED_MODE_ON);
    }
    // If event is end of transmission...
    else if (event == BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        bc_led_set_mode(&led, BC_LED_MODE_OFF);
    }
}
