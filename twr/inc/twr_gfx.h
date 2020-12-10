#ifndef _TWR_GFX_H
#define _TWR_GFX_H

#include <twr_common.h>
#include <twr_font_common.h>

//! @addtogroup twr_gfx twr_gfx
//! @brief Graphics library
//! @{

//! @brief Display size

typedef struct
{
    int width;
    int height;

} twr_gfx_caps_t;

//! @brief Display driver interface

typedef struct
{
    //! @brief Callback for is ready
    bool (*is_ready)(void *self);

    //! @brief Callback for clear display
    void (*clear)(void *self);

    //! @brief Callback for draw pifex
    void (*draw_pixel)(void *self, int left, int top, uint32_t color);

    //! @brief Callback for draw pifex
    uint32_t (*get_pixel)(void *self, int left, int top);

    //! @brief Callback for update
    bool (*update)(void *self);

    //! @brief Callback for get capabilities
    twr_gfx_caps_t (*get_caps)(void *self);

} twr_gfx_driver_t;

//! @brief Rotation

typedef enum
{
    //! @brief Rotation 0 degrees
    TWR_GFX_ROTATION_0   = 0,

    //! @brief Rotation 90 degrees
    TWR_GFX_ROTATION_90  = 1,

    //! @brief Rotation 180 degrees
    TWR_GFX_ROTATION_180 = 2,

    //! @brief Rotation 270 degrees
    TWR_GFX_ROTATION_270 = 3

} twr_gfx_rotation_t;

typedef enum
{
    //! @brief Round corner right top
    TWR_GFX_ROUND_CORNER_RIGHT_TOP =    0x01,

    //! @brief Round corner right bottom
    TWR_GFX_ROUND_CORNER_RIGHT_BOTTOM = 0x02,

    //! @brief Round corner left bottom
    TWR_GFX_ROUND_CORNER_LEFT_BOTTOM = 0x04,

    //! @brief Round corner left top
    TWR_GFX_ROUND_CORNER_LEFT_TOP = 0x08

} twr_gfx_round_corner_t;

//! @brief Instance

typedef struct
{
    void *_display;
    const twr_gfx_driver_t *_driver;
    twr_gfx_rotation_t _rotation;
    const twr_font_t *_font;
    twr_gfx_caps_t _caps;

} twr_gfx_t;

//! @brief Initialize button
//! @param[in] self Instance

void twr_gfx_init(twr_gfx_t *self, void *display, const twr_gfx_driver_t *driver);

//! @brief Check if display driver is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool twr_gfx_display_is_ready(twr_gfx_t *self);

//! @brief Get Display capabilities
//! @param[in] self Instance

twr_gfx_caps_t twr_gfx_get_caps(twr_gfx_t *self);

//! @brief Clear
//! @param[in] self Instance

void twr_gfx_clear(twr_gfx_t *self);

//! @brief Set font
//! @param[in] self Instance
//! @param[in] *font Font

void twr_gfx_set_font(twr_gfx_t *self, const twr_font_t *font);

//! @brief Set rotation
//! @param[in] self Instance
//! @param[in] rotation Rotation of diplay

void twr_gfx_set_rotation(twr_gfx_t *self, twr_gfx_rotation_t rotation);

//! @brief Get rotation
//! @param[in] self Instance
//! @return Rotation of display

twr_gfx_rotation_t twr_gfx_get_rotation(twr_gfx_t *self);

//! @brief Draw pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color

void twr_gfx_draw_pixel(twr_gfx_t *self, int x, int y, uint32_t color);

//! @brief Display draw char
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] ch Char to be printed
//! @param[in] color
//! @return Width of printed character

int twr_gfx_draw_char(twr_gfx_t *self, int left, int top, uint8_t ch, uint32_t color);

//! @brief Calc width character
//! @param[in] self Instance
//! @return Width of printed character

int twr_gfx_calc_char_width(twr_gfx_t *self, uint8_t ch);

//! @brief Display draw string
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] *str String to be printed
//! @param[in] color
//! @return Width of printed string

int twr_gfx_draw_string(twr_gfx_t *self, int left, int top, char *str, uint32_t color);

//! @brief Calc width string
//! @param[in] self Instance
//! @param[in] *str String to be printed
//! @return Width of printed string

int twr_gfx_calc_string_width(twr_gfx_t *self,  char *str);

//! @brief Display string
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments
//! @return Width of printed string

int twr_gfx_printf(twr_gfx_t *self, int left, int top, uint32_t color, char *format, ...);

//! @brief Display draw line
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color

void twr_gfx_draw_line(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Display draw rectangle
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color

void twr_gfx_draw_rectangle(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Display draw fill rectangle
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color

void twr_gfx_draw_fill_rectangle(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Display draw fill rectangle with a dithering pattern defined in the color parameter
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color Dithering mask, 16 bits define bit pattern in the 4 by 4 area

void twr_gfx_draw_fill_rectangle_dithering(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Lcd draw circle, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] radius In pixels
//! @param[in] color

void twr_gfx_draw_circle(twr_gfx_t *self, int x0, int y0, int radius, uint32_t color);

//! @brief Lcd draw fill circle, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] radius In pixels
//! @param[in] color

void twr_gfx_draw_fill_circle(twr_gfx_t *self, int x0, int y0, int radius, uint32_t color);

//! @brief Lcd draw round corner, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] corner position
//! @param[in] radius In pixels
//! @param[in] color

void twr_gfx_draw_round_corner(twr_gfx_t *self, int x0, int y0, int radius, twr_gfx_round_corner_t corner, uint32_t color);

//! @brief Lcd draw fill round corner, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] corner position
//! @param[in] radius In pixels
//! @param[in] color

void twr_gfx_draw_fill_round_corner(twr_gfx_t *self, int x0, int y0, int radius, twr_gfx_round_corner_t corner, uint32_t color);

//! @brief Display update, send data
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool twr_gfx_update(twr_gfx_t *self);

//! @}

#endif // _TWR_GFX_H
