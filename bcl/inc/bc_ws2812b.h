#ifndef _BC_WS2812B_H
#define _BC_WS2812B_H

#include <bc_common.h>

//! @addtogroup bc_ws2812b bc_ws2812b
//! @brief Driver for led strip ws2812b
//! @{

//! @cond

#define BC_WS2812_RESET_PERIOD 100

#define BC_WS2812B_PORT GPIOA
#define BC_WS2812B_PIN GPIO_PIN_1

typedef enum
{
    BC_WS2812B_TYPE_RGBW = 4,
    BC_WS2812B_TYPE_RGB = 3

} bc_ws2812b_type_t;

typedef struct ws2812b_t
{
	uint8_t *dma_bit_buffer;
	size_t dma_bit_buffer_size;
	bc_ws2812b_type_t type;
	uint16_t count;
    bool transfer;

} bc_ws2812b_t;


bool bc_ws2812b_init(bc_ws2812b_type_t type, uint16_t count);

void bc_ws2812b_set_pixel(uint16_t column, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

bool bc_ws2812b_send(void);

//! @}

#endif // _BC_WS2812B_H
