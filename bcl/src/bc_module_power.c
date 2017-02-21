#include <bc_module_power.h>
#include <bc_scheduler.h>
#include <bc_gpio.h>
#include <bc_ws2812b.h>

#define BC_MODULE_POWER_PIN_RELAY BC_GPIO_P0

static struct
{
    struct
    {
        bool on;

    } relay;

    struct
    {
        bool initialized;
        bool on;
        bc_led_strip_type_t type;
        int count;
        uint8_t *framebuffer;

    } led_strip;

    struct
    {
        bool test_is_on;
        int led;
        int step;

    } test;

} _bc_module_power;

static void _bc_module_power_led_strip_task(void *param);

void bc_module_power_init(void)
{
    memset(&_bc_module_power, 0, sizeof(_bc_module_power));

    bc_gpio_init(BC_MODULE_POWER_PIN_RELAY);
    bc_gpio_set_mode(BC_MODULE_POWER_PIN_RELAY, BC_GPIO_MODE_OUTPUT);
}

void bc_module_power_relay_set_state(bool state)
{
    _bc_module_power.relay.on = state;

    bc_gpio_set_output(BC_MODULE_POWER_PIN_RELAY, _bc_module_power.relay.on);
}

bool bc_module_power_relay_get_state(void)
{
    return _bc_module_power.relay.on;
}

void bc_module_power_led_strip_init(const bc_led_strip_t *led_strip)
{

    _bc_module_power.led_strip.initialized = true;
    _bc_module_power.led_strip.on = true;
    _bc_module_power.led_strip.type = led_strip->type;
    _bc_module_power.led_strip.count = led_strip->count;
    _bc_module_power.led_strip.framebuffer = led_strip->framebuffer;

    memset(_bc_module_power.led_strip.framebuffer, 0x00, led_strip->count * led_strip->type);

    bc_ws2812b_init(led_strip);

    bc_scheduler_register(_bc_module_power_led_strip_task, &_bc_module_power, 10);
}

void bc_module_power_led_strip_off(void)
{
    for (int i = 0; i < _bc_module_power.led_strip.count; i++)
    {
        bc_module_power_led_strip_set_pixel_from_uint32(i, 0x00000000);
    }

    bc_ws2812b_write();
}

void bc_module_power_led_strip_write(void)
{
    bc_ws2812b_write();
}

bool bc_module_power_led_strip_set_framebuffer(const uint8_t *framebuffer, size_t length)
{
    if (!_bc_module_power.led_strip.initialized)
    {
        return false;
    }

    if (length > (size_t) (_bc_module_power.led_strip.type * _bc_module_power.led_strip.count))
    {
        return false;
    }

    int position = 0;

    if (_bc_module_power.led_strip.type == BC_LED_STRIP_TYPE_RGBW)
    {
        for (size_t i = 0; i < length; i += _bc_module_power.led_strip.type)
        {
            bc_module_power_led_strip_set_pixel_from_rgb(position++, framebuffer[i], framebuffer[i + 1], framebuffer[i + 2], framebuffer[i + 3]);
        }
    }
    else
    {
        for (size_t i = 0; i < length; i += _bc_module_power.led_strip.count)
        {
            bc_module_power_led_strip_set_pixel_from_rgb(position++, framebuffer[i], framebuffer[i + 1], framebuffer[i + 2], 0);
        }
    }

    return true;
}

void bc_module_power_led_strip_set_pixel_from_rgb(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    if (!_bc_module_power.led_strip.initialized)
    {
        return;
    }

    int i = position * _bc_module_power.led_strip.type;

    _bc_module_power.led_strip.framebuffer[i] = red;
    _bc_module_power.led_strip.framebuffer[i++] = green;
    _bc_module_power.led_strip.framebuffer[i++] = blue;
    if (_bc_module_power.led_strip.type == BC_LED_STRIP_TYPE_RGBW)
    {
        _bc_module_power.led_strip.framebuffer[i] = white;
    }

    bc_ws2812b_set_pixel_from_rgb(position, red, green, blue, white);
}

void bc_module_power_led_strip_set_pixel_from_uint32(int position, uint32_t color)
{
    if (!_bc_module_power.led_strip.initialized)
    {
        return;
    }

    int i = position * _bc_module_power.led_strip.type;

    _bc_module_power.led_strip.framebuffer[i] = color >> 24;
    _bc_module_power.led_strip.framebuffer[i++] = color >> 16;
    _bc_module_power.led_strip.framebuffer[i++] = color >> 8;
    if (_bc_module_power.led_strip.type == BC_LED_STRIP_TYPE_RGBW)
    {
        _bc_module_power.led_strip.framebuffer[i] = color;
    }

    bc_ws2812b_set_pixel_from_uint32(position, color);
}

void bc_module_power_led_strip_test(void)
{
    if (!_bc_module_power.led_strip.initialized)
    {
        return;
    }

    _bc_module_power.test.led = 0;
    _bc_module_power.test.step = 0;
    _bc_module_power.test.test_is_on = true;
}

static void _bc_module_power_led_strip_task(void *param)
{
    (void) param;

    if (_bc_module_power.test.test_is_on && bc_ws2812b_write())
    {
        uint8_t intensity = 255 * (_bc_module_power.test.led + 1) / (_bc_module_power.led_strip.count + 1);

        if (_bc_module_power.test.step == 0)
        {
            bc_ws2812b_set_pixel_from_rgb(_bc_module_power.test.led, intensity, 0, 0, 0);
        }
        else if (_bc_module_power.test.step == 1)
        {
            bc_ws2812b_set_pixel_from_rgb(_bc_module_power.test.led, 0, intensity, 0, 0);
        }
        else if (_bc_module_power.test.step == 2)
        {
            bc_ws2812b_set_pixel_from_rgb(_bc_module_power.test.led, 0, 0, intensity, 0);
        }
        else if (_bc_module_power.test.step == 3)
        {
            if (_bc_module_power.led_strip.type == BC_LED_STRIP_TYPE_RGBW)
            {
                    bc_ws2812b_set_pixel_from_rgb(_bc_module_power.test.led, 0, 0, 0, intensity);
            }
            else
            {
                bc_ws2812b_set_pixel_from_rgb(_bc_module_power.test.led, intensity, intensity, intensity, 0);
            }
        }
        else
        {
            bc_ws2812b_set_pixel_from_rgb(_bc_module_power.test.led, 0, 0, 0, 0);
        }

        _bc_module_power.test.led++;

        if (_bc_module_power.test.led == _bc_module_power.led_strip.count)
        {
            _bc_module_power.test.led = 0;

            _bc_module_power.test.step++;

            if (_bc_module_power.test.step == 5)
            {
                _bc_module_power.test.test_is_on = false;
            }
        }
    }

    bc_scheduler_plan_current_relative(10);
}
