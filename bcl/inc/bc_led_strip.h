#ifndef _BC_LED_STRIP_H
#define _BC_LED_STRIP_H

#include <bc_common.h>

typedef enum
{
	BC_LED_STRIP_TYPE_RGBW = 4,
	BC_LED_STRIP_TYPE_RGB = 3

} bc_led_strip_type_t;

typedef struct
{
	bc_led_strip_type_t type;
	int count;
	uint8_t *framebuffer;
	uint32_t *dma_buffer;

} bc_led_strip_t;

extern const bc_led_strip_t bc_led_strip_rgbw_144;
extern const bc_led_strip_t bc_led_strip_rgb_150;

#endif // _BC_LED_STRIP_H
