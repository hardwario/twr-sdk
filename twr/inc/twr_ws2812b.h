#ifndef _TWR_WS2812B_H
#define _TWR_WS2812B_H

#include <twr_led_strip.h>

//! @addtogroup twr_ws2812b twr_ws2812b
//! @brief Driver for led strip ws2812b
//! @{

//! @cond

typedef enum
{
    TWR_WS2812B_SEND_DONE = 0,

} twr_ws2812b_event_t;

bool twr_ws2812b_init(const twr_led_strip_buffer_t *led_strip);

void twr_ws2812b_set_event_handler(void (*event_handler)(twr_ws2812b_event_t, void *), void *event_param);

void twr_ws2812b_set_pixel_from_rgb(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

void twr_ws2812b_set_pixel_from_uint32(int position, uint32_t color);

void twr_ws2812b_set_pixel_from_rgb_swap_rg(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

void twr_ws2812b_set_pixel_from_uint32_swap_rg(int position, uint32_t color);

bool twr_ws2812b_write(void);

bool twr_ws2812b_is_ready(void);

//! @}

#endif // _TWR_WS2812B_H
