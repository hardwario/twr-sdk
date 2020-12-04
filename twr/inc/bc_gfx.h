#ifndef _BC_GFX_H
#define _BC_GFX_H

#include <bc_common.h>
#include <bc_font_common.h>

//! @addtogroup bc_gfx bc_gfx
//! @brief Graphics library
//! @{

//! @brief Display size

typedef struct
{
    int width;
    int height;

} bc_gfx_caps_t;

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
    bc_gfx_caps_t (*get_caps)(void *self);

} bc_gfx_driver_t;

//! @brief Rotation

typedef enum
{
    //! @brief Rotation 0 degrees
    BC_GFX_ROTATION_0   = 0,

    //! @brief Rotation 90 degrees
    BC_GFX_ROTATION_90  = 1,

    //! @brief Rotation 180 degrees
    BC_GFX_ROTATION_180 = 2,

    //! @brief Rotation 270 degrees
    BC_GFX_ROTATION_270 = 3

} bc_gfx_rotation_t;

typedef enum
{
    //! @brief Round corner right top
    BC_GFX_ROUND_CORNER_RIGHT_TOP =    0x01,

    //! @brief Round corner right bottom
    BC_GFX_ROUND_CORNER_RIGHT_BOTTOM = 0x02,

    //! @brief Round corner left bottom
    BC_GFX_ROUND_CORNER_LEFT_BOTTOM = 0x04,

    //! @brief Round corner left top
    BC_GFX_ROUND_CORNER_LEFT_TOP = 0x08

} bc_gfx_round_corner_t;

//! @brief Instance

typedef struct
{
    void *_display;
    const bc_gfx_driver_t *_driver;
    bc_gfx_rotation_t _rotation;
    const bc_font_t *_font;
    bc_gfx_caps_t _caps;

} bc_gfx_t;

//! @brief Initialize button
//! @param[in] self Instance

void bc_gfx_init(bc_gfx_t *self, void *display, const bc_gfx_driver_t *driver);

//! @brief Check if display driver is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool bc_gfx_display_is_ready(bc_gfx_t *self);

//! @brief Get Display capabilities
//! @param[in] self Instance

bc_gfx_caps_t bc_gfx_get_caps(bc_gfx_t *self);

//! @brief Clear
//! @param[in] self Instance

void bc_gfx_clear(bc_gfx_t *self);

//! @brief Set font
//! @param[in] self Instance
//! @param[in] *font Font

void bc_gfx_set_font(bc_gfx_t *self, const bc_font_t *font);

//! @brief Set rotation
//! @param[in] self Instance
//! @param[in] rotation Rotation of diplay

void bc_gfx_set_rotation(bc_gfx_t *self, bc_gfx_rotation_t rotation);

//! @brief Get rotation
//! @param[in] self Instance
//! @return Rotation of display

bc_gfx_rotation_t bc_gfx_get_rotation(bc_gfx_t *self);

//! @brief Draw pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color

void bc_gfx_draw_pixel(bc_gfx_t *self, int x, int y, uint32_t color);

//! @brief Display draw char
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] ch Char to be printed
//! @param[in] color
//! @return Width of printed character

int bc_gfx_draw_char(bc_gfx_t *self, int left, int top, uint8_t ch, uint32_t color);

//! @brief Calc width character
//! @param[in] self Instance
//! @return Width of printed character

int bc_gfx_calc_char_width(bc_gfx_t *self, uint8_t ch);

//! @brief Display draw string
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] *str String to be printed
//! @param[in] color
//! @return Width of printed string

int bc_gfx_draw_string(bc_gfx_t *self, int left, int top, char *str, uint32_t color);

//! @brief Calc width string
//! @param[in] self Instance
//! @param[in] *str String to be printed
//! @return Width of printed string

int bc_gfx_calc_string_width(bc_gfx_t *self,  char *str);

//! @brief Display string
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments
//! @return Width of printed string

int bc_gfx_printf(bc_gfx_t *self, int left, int top, uint32_t color, char *format, ...);

//! @brief Display draw line
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color

void bc_gfx_draw_line(bc_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Display draw rectangle
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color

void bc_gfx_draw_rectangle(bc_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Display draw fill rectangle
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color

void bc_gfx_draw_fill_rectangle(bc_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Display draw fill rectangle with a dithering pattern defined in the color parameter
//! @param[in] self Instance
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color Dithering mask, 16 bits define bit pattern in the 4 by 4 area

void bc_gfx_draw_fill_rectangle_dithering(bc_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color);

//! @brief Lcd draw circle, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] radius In pixels
//! @param[in] color

void bc_gfx_draw_circle(bc_gfx_t *self, int x0, int y0, int radius, uint32_t color);

//! @brief Lcd draw fill circle, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] radius In pixels
//! @param[in] color

void bc_gfx_draw_fill_circle(bc_gfx_t *self, int x0, int y0, int radius, uint32_t color);

//! @brief Lcd draw round corner, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] corner position
//! @param[in] radius In pixels
//! @param[in] color

void bc_gfx_draw_round_corner(bc_gfx_t *self, int x0, int y0, int radius, bc_gfx_round_corner_t corner, uint32_t color);

//! @brief Lcd draw fill round corner, using Midpoint circle algorithm
//! @param[in] self Instance
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] corner position
//! @param[in] radius In pixels
//! @param[in] color

void bc_gfx_draw_fill_round_corner(bc_gfx_t *self, int x0, int y0, int radius, bc_gfx_round_corner_t corner, uint32_t color);

//! @brief Display update, send data
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool bc_gfx_update(bc_gfx_t *self);

//! @}

#endif // _BC_GFX_H
