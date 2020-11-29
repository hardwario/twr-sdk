#include <application.h>

// LED instance
hio_led_t led;

// Button instance
hio_button_t button;

// Temperature Tag instance
hio_tag_temperature_t temperature_tag;

// SigFox Module instance
hio_module_sigfox_t sigfox_module;

// Variable holding temperature in centigrades
float temperature;

void application_init(void)
{
    // Initialize LED
    hio_led_init(&led, HIO_GPIO_LED, false, false);

    // Initialize button
    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize Temperature Tag
    hio_tag_temperature_init(&temperature_tag, HIO_I2C_I2C0, HIO_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    hio_tag_temperature_set_update_interval(&temperature_tag, 120000);
    hio_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    // Initialize SigFox Module
    hio_module_sigfox_init(&sigfox_module, HIO_MODULE_SIGFOX_REVISION_R2);
    hio_module_sigfox_set_event_handler(&sigfox_module, sigfox_module_event_handler, NULL);
}

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        if (hio_module_sigfox_is_ready(&sigfox_module))
        {
            uint8_t buffer[] = { 0x01 };

            hio_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
        }
    }
}

void temperature_tag_event_handler(hio_tag_temperature_t *self, hio_tag_temperature_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (hio_module_sigfox_is_ready(&sigfox_module))
        {
            // Read temperature
            hio_tag_temperature_get_temperature_celsius(self, &temperature);

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

            hio_module_sigfox_send_rf_frame(&sigfox_module, buffer, sizeof(buffer));
        }
    }
    else if (event == HIO_TAG_TEMPERATURE_EVENT_ERROR)
    {
    }
}

void sigfox_module_event_handler(hio_module_sigfox_t *self, hio_module_sigfox_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    // If event is start of transmission...
    if (event == HIO_MODULE_SIGFOX_EVENT_ERROR)
    {
        hio_led_set_mode(&led, HIO_LED_MODE_BLINK);
    }
    // If event is start of transmission...
    if (event == HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START)
    {
        hio_led_set_mode(&led, HIO_LED_MODE_ON);
    }
    // If event is end of transmission...
    else if (event == HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE)
    {
        hio_led_set_mode(&led, HIO_LED_MODE_OFF);
    }
}
