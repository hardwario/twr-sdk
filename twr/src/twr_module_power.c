#include <twr_module_power.h>
#include <twr_scheduler.h>
#include <twr_gpio.h>
#include <twr_ws2812b.h>

#define TWR_MODULE_POWER_PIN_RELAY TWR_GPIO_P0

static uint32_t _twr_module_power_led_strip_dma_buffer_rgbw_144[144 * 4 * 2];
static uint32_t _twr_module_power_led_strip_dma_buffer_rgb_150[150 * 3 * 2];

const twr_led_strip_buffer_t twr_module_power_led_strip_buffer_rgbw_144 =
{
    .type = TWR_LED_STRIP_TYPE_RGBW,
    .count = 144,
    .buffer = _twr_module_power_led_strip_dma_buffer_rgbw_144
};

const twr_led_strip_buffer_t twr_module_power_led_strip_buffer_rgb_150 =
{
    .type = TWR_LED_STRIP_TYPE_RGB,
    .count = 150,
    .buffer = _twr_module_power_led_strip_dma_buffer_rgb_150
};

#if LED_STRIP_SWAP_RG == 0

const twr_led_strip_driver_t twr_module_power_led_strip_driver =
{
    .init = twr_ws2812b_init,
    .write = twr_ws2812b_write,
    .set_pixel = twr_ws2812b_set_pixel_from_uint32,
    .set_pixel_rgbw = twr_ws2812b_set_pixel_from_rgb,
    .is_ready = twr_ws2812b_is_ready
};

#else

const twr_led_strip_driver_t twr_module_power_led_strip_driver =
{
    .init = twr_ws2812b_init,
    .write = twr_ws2812b_write,
    .set_pixel = twr_ws2812b_set_pixel_from_uint32_swap_rg,
    .set_pixel_rgbw = twr_ws2812b_set_pixel_from_rgb_swap_rg,
    .is_ready = twr_ws2812b_is_ready
};

#endif

static struct
{
    struct
    {
        bool on;

    } relay;

} _twr_module_power;

void twr_module_power_init(void)
{
    memset(&_twr_module_power, 0, sizeof(_twr_module_power));

    twr_gpio_init(TWR_MODULE_POWER_PIN_RELAY);
    twr_gpio_set_mode(TWR_MODULE_POWER_PIN_RELAY, TWR_GPIO_MODE_OUTPUT);
}

void twr_module_power_relay_set_state(bool state)
{
    _twr_module_power.relay.on = state;

    twr_gpio_set_output(TWR_MODULE_POWER_PIN_RELAY, _twr_module_power.relay.on ? 1 : 0);
}

bool twr_module_power_relay_get_state(void)
{
    return _twr_module_power.relay.on;
}

const twr_led_strip_driver_t *twr_module_power_get_led_strip_driver(void)
{
    return &twr_module_power_led_strip_driver;
}
