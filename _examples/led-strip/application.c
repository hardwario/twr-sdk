#include <application.h>
#include <hio.h>

#define COUNT 144

hio_led_t led;
hio_led_strip_t led_strip;
uint32_t color;
int effect = -1;

static uint32_t _dma_buffer[COUNT * 4 * 2]; // count * type * 2

const hio_led_strip_buffer_t _led_strip_buffer =
{
    .type = HIO_LED_STRIP_TYPE_RGBW,
    .count = COUNT,
    .buffer = _dma_buffer
};

void application_init(void)
{

    hio_led_init(&led, HIO_GPIO_LED, false, false);
    hio_led_set_mode(&led, HIO_LED_MODE_ON);


    static hio_button_t button;
    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button, button_event_handler, NULL);

    hio_module_power_init();
    hio_led_strip_init(&led_strip, hio_module_power_get_led_strip_driver(), &_led_strip_buffer);
    hio_led_strip_set_event_handler(&led_strip, led_strip_event_handler, NULL);

    hio_led_strip_fill(&led_strip, 0x10000000);
    hio_led_strip_write(&led_strip);

    hio_led_set_mode(&led, HIO_LED_MODE_OFF);
}

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        effect++;

        if (effect == 0)
        {
            hio_led_strip_effect_rainbow(&led_strip, 25);
        }
        else if (effect == 1)
        {
            hio_led_strip_effect_rainbow_cycle(&led_strip, 25);
        }
        else if (effect == 2)
        {
            color = 0xff000000;
            hio_led_strip_fill(&led_strip, 0x00000000);
            hio_led_strip_effect_color_wipe(&led_strip, color, 50);
        }
        else if (effect == 3)
        {
            hio_led_strip_effect_theater_chase(&led_strip, color, 45);
        }
        else if (effect == 4)
        {
            hio_led_strip_effect_theater_chase_rainbow(&led_strip, 45);
        }
        else if (effect == 5)
        {
            effect = -1;
            hio_led_strip_effect_stop(&led_strip);
            hio_led_strip_fill(&led_strip, 0x00000000);
            hio_led_strip_write(&led_strip);
        }

    }
    else if (event == HIO_BUTTON_EVENT_RELEASE)
    {

    }
    else if (event == HIO_BUTTON_EVENT_HOLD)
    {
        hio_led_strip_effect_test(&led_strip);
    }

}

void led_strip_event_handler(hio_led_strip_t *self, hio_led_strip_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_LED_STRIP_EVENT_EFFECT_DONE)
    {
        if (effect == 2)
        {
            color >>= 8;
            if (color == 0x00000000)
            {
                color = 0xff000000;
            }
            hio_led_strip_effect_color_wipe(self, color, 50);
        }

    }
}
