#ifndef _TWR_SSD1306_H
#define _TWR_SSD1306_H

#include <twr_gfx.h>
#include <twr_scheduler.h>
#include <twr_i2c.h>

//! @addtogroup twr_ssd1306 twr_ssd1306
//! @brief Driver for SSD1306 Monochrome OLED graphic display
//! @{

#define TWR_SSD1306_ADDRESS_I2C_ADDRESS_DEFAULT 0x3C
#define TWR_SSD1306_ADDRESS_I2C_ADDRESS_ALTERNATE 0x3D

#define TWR_SSD1306_FRAMEBUFFER(NAME, WIDTH, HEIGHT) \
    uint8_t NAME##_buffer[WIDTH * HEIGHT / 8]; \
    twr_ssd1306_framebuffer_t NAME = { \
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

} twr_ssd1306_framebuffer_t;

//! @brief Instance

typedef struct
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    const twr_ssd1306_framebuffer_t *_framebuffer;
    bool _initialized;

} twr_ssd1306_t;

//! @brief Initialize lcd driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address
//! @param[in] framebuffer

bool twr_ssd1306_init(twr_ssd1306_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address, const twr_ssd1306_framebuffer_t *framebuffer);

//! @brief Get capabilities
//! @param[in] self Instance

twr_gfx_caps_t twr_ssd1306_get_caps(twr_ssd1306_t *self);

//! @brief Check if lcd is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool twr_ssd1306_is_ready(twr_ssd1306_t *self);

//! @brief Clear
//! @param[in] self Instance

void twr_ssd1306_clear(twr_ssd1306_t *self);

//! @brief Lcd draw pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

void twr_ssd1306_draw_pixel(twr_ssd1306_t *self, int x, int y, uint32_t color);

//! @brief Lcd get pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

uint32_t twr_ssd1306_get_pixel(twr_ssd1306_t *self, int x, int y);

//! @brief Lcd update, send data
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool twr_ssd1306_update(twr_ssd1306_t *self);

//! @brief Get Lcd driver

const twr_gfx_driver_t *twr_ssd1306_get_driver(void);

//! @}

#endif //_TWR_SSD1306_H
