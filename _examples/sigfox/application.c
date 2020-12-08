#include <application.h>

// LED instance
twr_led_t led;

// Button instance
twr_button_t button;

// Temperature Tag instance
twr_tag_temperature_t temperature_tag;

// SigFox Module instance
twr_module_sigfox_t sigfox_module;

// Variable holding temperature in centigrades
float temperature;

void application_init(void)
{
    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize Temperature Tag
    twr_tag_temperature_init(&temperature_tag, TWR_I2C_I2C0, TWR_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    twr_tag_temperature_set_update_interval(&temperature_tag, 120000);
    twr_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    // Initialize SigFox Module
    twr_module_sigfox_init(&sigfox_module, TWR_MODULE_SIGFOX_REVISION_R2);
    twr_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);
}

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        if (twr_module_sigfox_is_ready(&sigfox_module))
        {
            uint8_t buffer[] = { 0x01 };

            twr_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
        }
    }
}

void temperature_tag_event_handler(twr_tag_temperature_t *self, twr_tag_temperature_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (twr_module_sigfox_is_ready(&sigfox_module))
        {
            // Read temperature
            twr_tag_temperature_get_temperature_celsius(self, &temperature);

            uint8_t buffer[5];

            // First byte 0x02 says we are sending temperature
            buffer[0] = 0x02;
            // Variable temperature is converted from float to uint32_t
            // and then shifted. The decimal part is lost.
            // If you need decimal part, first multiply float by * 100 and then do the casting to uint32_t
            buffer[1] = (uint32_t) temperature;
            buffer[2] = (uint32_t) temperature >> 8;
            buffer[3] = (uint32_t) temperature >> 16;
            buffer[4] = (uint32_t) temperature >> 24;

            twr_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
        }
    }
    else if (event == TWR_TAG_TEMPERATURE_EVENT_ERROR)
    {
    }
}

void sigfox_module_event_handler(twr_module_sigfox_t *self, twr_module_sigfox_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    // If event is start of transmission...
    if (event == TWR_MODULE_SIGFOX_EVENT_ERROR)
    {
        twr_led_set_mode(&led, TWR_LED_MODE_BLINK);
    }
    // If event is start of transmission...
    if (event == TWR_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START)
    {
        twr_led_set_mode(&led, TWR_LED_MODE_ON);
    }
    // If event is end of transmission...
    else if (event == TWR_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        twr_led_set_mode(&led, TWR_LED_MODE_OFF);
    }
}
