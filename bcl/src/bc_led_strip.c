#include <bc_led_strip.h>

static uint8_t _bc_led_strip_framebuffer_rgbw_144[144 * 4];
static uint32_t _bc_led_strip_dma_buffer_rgbw_144[144 * 4 * 2];

const bc_led_strip_t bc_led_strip_rgbw_144 = {
		.type = BC_LED_STRIP_TYPE_RGBW,
		.count = 144,
		.framebuffer = _bc_led_strip_framebuffer_rgbw_144,
		.dma_buffer = _bc_led_strip_dma_buffer_rgbw_144
};


static uint8_t _bc_led_strip_framebuffer_rgb_150[150 * 3];
static uint32_t _bc_led_strip_dma_buffer_rgb_150[150 * 3 * 2];

const bc_led_strip_t bc_led_strip_rgb_150 = {
		.type = BC_LED_STRIP_TYPE_RGB,
		.count = 150,
		.framebuffer = _bc_led_strip_framebuffer_rgb_150,
		.dma_buffer = _bc_led_strip_dma_buffer_rgb_150
};

;
