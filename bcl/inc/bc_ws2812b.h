#ifndef _BC_WS2812B_H
#define _BC_WS2812B_H

#include <bc_led_strip.h>

//! @addtogroup bc_ws2812b bc_ws2812b
//! @brief Driver for led strip ws2812b
//! @{

//! @cond

typedef enum
{
    BC_WS2812B_SEND_DONE = 0,

} bc_ws2812b_event_t;

bool bc_ws2812b_init(bc_led_strip_buffer_t *led_strip);

void bc_ws2812b_set_event_handler(void (*event_handler)(bc_ws2812b_event_t, void *), void *event_param);

void bc_ws2812b_set_pixel_from_rgb(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

void bc_ws2812b_set_pixel_from_uint32(int position, uint32_t color);

bool bc_ws2812b_write(void);

//! @}

#endif // _BC_WS2812B_H
