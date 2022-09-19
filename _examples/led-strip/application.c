#include <application.h>
#include <twr.h>

#define COUNT 144

twr_led_t led;
twr_led_strip_t led_strip;
uint32_t color;
int effect = -1;

static uint32_t _dma_buffer[COUNT * 4 * 2]; // count * type * 2

const twr_led_strip_buffer_t _led_strip_buffer =
{
    .type = TWR_LED_STRIP_TYPE_RGBW,
    .count = COUNT,
    .buffer = _dma_buffer
};

void application_init(void)
{

    twr_led_init(&led, TWR_GPIO_LED, false, false);
    twr_led_set_mode(&led, TWR_LED_MODE_ON);


    static twr_button_t button;
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_module_power_init();
    twr_led_strip_init(&led_strip, twr_module_power_get_led_strip_driver(), &_led_strip_buffer);
    twr_led_strip_set_event_handler(&led_strip, led_strip_event_handler, NULL);

    twr_led_strip_fill(&led_strip, 0x10000000);
    twr_led_strip_write(&led_strip);

    twr_led_set_mode(&led, TWR_LED_MODE_OFF);
}

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        effect++;

        if (effect == 0)
        {
            twr_led_strip_effect_rainbow(&led_strip, 25);
        }
        else if (effect == 1)
        {
            twr_led_strip_effect_rainbow_cycle(&led_strip, 25);
        }
        else if (effect == 2)
        {
            color = 0xff000000;
            twr_led_strip_fill(&led_strip, 0x00000000);
            twr_led_strip_effect_color_wipe(&led_strip, color, 50);
        }
        else if (effect == 3)
        {
            twr_led_strip_effect_theater_chase(&led_strip, color, 45);
        }
        else if (effect == 4)
        {
            twr_led_strip_effect_theater_chase_rainbow(&led_strip, 45);
        }
        else if (effect == 5)
        {
            effect = -1;
            twr_led_strip_effect_stop(&led_strip);
            twr_led_strip_fill(&led_strip, 0x00000000);
            twr_led_strip_write(&led_strip);
        }

    }
    else if (event == TWR_BUTTON_EVENT_RELEASE)
    {

    }
    else if (event == TWR_BUTTON_EVENT_HOLD)
    {
        twr_led_strip_effect_test(&led_strip);
    }

}

void led_strip_event_handler(twr_led_strip_t *self, twr_led_strip_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_LED_STRIP_EVENT_EFFECT_DONE)
    {
        if (effect == 2)
        {
            color >>= 8;
            if (color == 0x00000000)
            {
                color = 0xff000000;
            }
            twr_led_strip_effect_color_wipe(self, color, 50);
        }

    }
}
