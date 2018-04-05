#ifndef _BC_SSD1306_H
#define _BC_SSD1306_H

#include <bc_gfx.h>
#include <bc_scheduler.h>
#include <bc_i2c.h>

//! @addtogroup bc_ssd1306 bc_ssd1306
//! @brief Driver for SSD1306 Monochrome OLED graphic display
//! @{

#define BC_SSD1306_ADDRESS_I2C_ADDRESS_DEFAULT 0x3C
#define BC_SSD1306_ADDRESS_I2C_ADDRESS_ALTERNATE 0x3D

#define BC_SSD1306_FRAMEBUFFER(NAME, WIDTH, HEIGHT) \
    uint8_t NAME##_buffer[WIDTH * HEIGHT / 8]; \
    bc_ssd1306_framebuffer_t NAME = { \
            .buffer = NAME##_buffer, \
            .width = WIDTH, \
            .height = HEIGHT, \
            .length = sizeof(NAME##_buffer), \
            .pages = HEIGHT / 8 \
    };

typedef struct
{
    int width;
    int height;
    uint8_t *buffer;
    size_t length;
    int pages;

} bc_ssd1306_framebuffer_t;

//! @brief Instance

typedef struct
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    const bc_ssd1306_framebuffer_t *_framebuffer;
    bool _initialized;

} bc_ssd1306_t;

//! @brief Initialize lcd driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address
//! @param[in] framebuffer

bool bc_ssd1306_init(bc_ssd1306_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address, const bc_ssd1306_framebuffer_t *framebuffer);

//! @brief Get capabilities
//! @param[in] self Instance

bc_gfx_caps_t bc_ssd1306_get_caps(bc_ssd1306_t *self);

//! @brief Check if lcd is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool bc_ssd1306_is_ready(bc_ssd1306_t *self);

//! @brief Clear
//! @param[in] self Instance

void bc_ssd1306_clear(bc_ssd1306_t *self);

//! @brief Lcd draw pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

void bc_ssd1306_draw_pixel(bc_ssd1306_t *self, int x, int y, uint32_t color);

//! @brief Lcd get pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

uint32_t bc_ssd1306_get_pixel(bc_ssd1306_t *self, int x, int y);

//! @brief Lcd update, send data
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool bc_ssd1306_update(bc_ssd1306_t *self);

//! @brief Get Lcd driver

const bc_gfx_driver_t *bc_ssd1306_get_driver(void);

//! @}

#endif //_BC_SSD1306_H
