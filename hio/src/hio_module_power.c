#include <hio_module_power.h>
#include <hio_scheduler.h>
#include <hio_gpio.h>
#include <hio_ws2812b.h>

#define HIO_MODULE_POWER_PIN_RELAY HIO_GPIO_P0

static uint32_t _hio_module_power_led_strip_dma_buffer_rgbw_144[144 * 4 * 2];
static uint32_t _hio_module_power_led_strip_dma_buffer_rgb_150[150 * 3 * 2];

const hio_led_strip_buffer_t hio_module_power_led_strip_buffer_rgbw_144 =
{
    .type = HIO_LED_STRIP_TYPE_RGBW,
    .count = 144,
    .buffer = _hio_module_power_led_strip_dma_buffer_rgbw_144
};

const hio_led_strip_buffer_t hio_module_power_led_strip_buffer_rgb_150 =
{
    .type = HIO_LED_STRIP_TYPE_RGB,
    .count = 150,
    .buffer = _hio_module_power_led_strip_dma_buffer_rgb_150
};

#if LED_STRIP_SWAP_RG == 0

const hio_led_strip_driver_t hio_module_power_led_strip_driver =
{
    .init = hio_ws2812b_init,
    .write = hio_ws2812b_write,
    .set_pixel = hio_ws2812b_set_pixel_from_uint32,
    .set_pixel_rgbw = hio_ws2812b_set_pixel_from_rgb,
    .is_ready = hio_ws2812b_is_ready
};

#else

const hio_led_strip_driver_t hio_module_power_led_strip_driver =
{
    .init = hio_ws2812b_init,
    .write = hio_ws2812b_write,
    .set_pixel = hio_ws2812b_set_pixel_from_uint32_swap_rg,
    .set_pixel_rgbw = hio_ws2812b_set_pixel_from_rgb_swap_rg,
    .is_ready = hio_ws2812b_is_ready
};

#endif

static struct
{
    struct
    {
        bool on;

    } relay;

} _hio_module_power;

void hio_module_power_init(void)
{
    memset(&_hio_module_power, 0, sizeof(_hio_module_power));

    hio_gpio_init(HIO_MODULE_POWER_PIN_RELAY);
    hio_gpio_set_mode(HIO_MODULE_POWER_PIN_RELAY, HIO_GPIO_MODE_OUTPUT);
}

void hio_module_power_relay_set_state(bool state)
{
    _hio_module_power.relay.on = state;

    hio_gpio_set_output(HIO_MODULE_POWER_PIN_RELAY, _hio_module_power.relay.on ? 1 : 0);
}

bool hio_module_power_relay_get_state(void)
{
    return _hio_module_power.relay.on;
}

const hio_led_strip_driver_t *hio_module_power_get_led_strip_driver(void)
{
    return &hio_module_power_led_strip_driver;
}
