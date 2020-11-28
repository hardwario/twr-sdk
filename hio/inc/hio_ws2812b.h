#ifndef _HIO_WS2812B_H
#define _HIO_WS2812B_H

#include <hio_led_strip.h>

//! @addtogroup hio_ws2812b hio_ws2812b
//! @brief Driver for led strip ws2812b
//! @{

//! @cond

typedef enum
{
    HIO_WS2812B_SEND_DONE = 0,

} hio_ws2812b_event_t;

bool hio_ws2812b_init(const hio_led_strip_buffer_t *led_strip);

void hio_ws2812b_set_event_handler(void (*event_handler)(hio_ws2812b_event_t, void *), void *event_param);

void hio_ws2812b_set_pixel_from_rgb(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

void hio_ws2812b_set_pixel_from_uint32(int position, uint32_t color);

void hio_ws2812b_set_pixel_from_rgb_swap_rg(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

void hio_ws2812b_set_pixel_from_uint32_swap_rg(int position, uint32_t color);

bool hio_ws2812b_write(void);

bool hio_ws2812b_is_ready(void);

//! @}

#endif // _HIO_WS2812B_H
