#include <application.h>

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;

// Temperature Tag instance
bc_tag_temperature_t temperature_tag;

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_tag_temperature_init(&temperature_tag, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    bc_tag_temperature_set_update_interval(&temperature_tag, 10000);
    bc_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    // Initialize radio
    bc_radio_init();
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enroll_to_gateway();

        bc_led_pulse(&led, 1000);
    }
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    float temperature;

    if (bc_tag_temperature_get_temperature_celsius(self, &temperature))
    {
        bc_radio_pub_thermometer(&temperature);
    }
    else
    {
        bc_radio_pub_thermometer(NULL);
    }
}
