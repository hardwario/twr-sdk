#include <bc_module_power.h>
#include <bc_scheduler.h>
#include <bc_gpio.h>
#include <bc_ws2812b.h>

#define BC_MODULE_POWER_PIN_RELAY BC_GPIO_P0

static uint32_t _bc_module_power_led_strip_dma_buffer_rgbw_144[144 * 4 * 2];
static uint32_t _bc_module_power_led_strip_dma_buffer_rgb_150[150 * 3 * 2];

const bc_led_strip_buffer_t bc_module_power_led_strip_buffer_rgbw_144 =
{
    .type = BC_LED_STRIP_TYPE_RGBW,
    .count = 144,
    .buffer = _bc_module_power_led_strip_dma_buffer_rgbw_144
};

const bc_led_strip_buffer_t bc_module_power_led_strip_buffer_rgb_150 =
{
    .type = BC_LED_STRIP_TYPE_RGB,
    .count = 150,
    .buffer = _bc_module_power_led_strip_dma_buffer_rgb_150
};

const bc_led_strip_driver_t bc_module_power_led_strip_driver =
{
    .init = bc_ws2812b_init,
    .write = bc_ws2812b_write,
    .set_pixel = bc_ws2812b_set_pixel_from_uint32,
    .set_pixel_rgbw = bc_ws2812b_set_pixel_from_rgb
};

static struct
{
    struct
    {
        bool on;

    } relay;

} _bc_module_power;

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

const bc_led_strip_driver_t *bc_module_power_get_led_strip_driver(void)
{
    return &bc_module_power_led_strip_driver;
}
