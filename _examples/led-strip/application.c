#include <application.h>
#include <bcl.h>

bc_led_t led;
bc_led_strip_t led_strip;
uint32_t color;
int effect = -1;

static uint32_t _dma_buffer_rgb_12[12 * 3 * 2]; // count * type * 2

const bc_led_strip_buffer_t _led_strip_buffer_rgb_12 =
{
    .type = BC_LED_STRIP_TYPE_RGB,
    .count = 12,
    .buffer = _dma_buffer_rgb_12
};

void application_init(void)
{
    usb_talk_init();
    bc_rtc_init();

    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);


    static bc_button_t button;
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);


    bc_module_power_init();
    bc_led_strip_init(&led_strip, bc_module_power_get_led_strip_driver(), &bc_module_power_led_strip_buffer_rgb_150);
    bc_led_strip_set_event_handler(&led_strip, led_strip_event_handler, NULL);

    bc_led_strip_fill(&led_strip, 0x00000000);
    bc_led_strip_write(&led_strip);

    bc_led_set_mode(&led, BC_LED_MODE_OFF);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        effect++;

        if (effect == 0)
        {
            bc_led_strip_effect_rainbow(&led_strip, 25);
        }
        else if (effect == 1)
        {
            bc_led_strip_effect_rainbow_cycle(&led_strip, 25);
        }
        else if (effect == 2)
        {
            color = 0xff000000;
            bc_led_strip_fill(&led_strip, 0x00000000);
            bc_led_strip_effect_color_wipe(&led_strip, color, 50);
        }
        else if (effect == 3)
        {
            bc_led_strip_effect_theater_chase(&led_strip, color, 45);
        }
        else if (effect == 4)
        {
            bc_led_strip_effect_theater_chase_rainbow(&led_strip, 45);
        }
        else if (effect == 5)
        {
            effect = -1;
            bc_led_strip_effect_stop(&led_strip);
            bc_led_strip_fill(&led_strip, 0x00000000);
            bc_led_strip_write(&led_strip);
        }

    }
    else if (event == BC_BUTTON_EVENT_RELEASE)
    {

    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_led_strip_effect_test(&led_strip);
    }

}

void led_strip_event_handler(bc_led_strip_t *self, bc_led_strip_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_LED_STRIP_EVENT_EFFECT_DONE)
    {
        if (effect == 2)
        {
            color >>= 8;
            if (color == 0x00000000)
            {
                color = 0xff000000;
            }
            bc_led_strip_effect_color_wipe(self, color, 50);
        }

    }
}
