#ifndef _BC_WS2812B_H
#define _BC_WS2812B_H

#include <bc_common.h>

//! @addtogroup bc_ws2812b bc_ws2812b
//! @brief Driver for led strip ws2812b
//! @{

//! @cond

typedef enum
{
    BC_WS2812B_TYPE_RGBW = 4,
    BC_WS2812B_TYPE_RGB = 3

} bc_ws2812b_type_t;


bool bc_ws2812b_init(void *dma_bit_buffer, bc_ws2812b_type_t type, uint16_t count);

void bc_ws2812b_set_pixel(uint16_t column, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

bool bc_ws2812b_send(void);

//! @}

#endif // _BC_WS2812B_H
