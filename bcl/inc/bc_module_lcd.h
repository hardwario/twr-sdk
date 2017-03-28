#ifndef _BC_MODULE_LCD
#define _BC_MODULE_LCD


#include <bc_font_common.h>

//! @addtogroup bc_module_lcd bc_module_lcd
//! @brief Driver for lcd
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Done event
    BC_MODULE_LCD_EVENT_DONE = 0,

} bc_module_lcd_event_t;

extern const tFont Font;
extern const tFont FontBig;

// See app note https://www.silabs.com/documents/public/application-notes/AN0048.pdf
// Figure 3.1
// 1B mode | 1B addr + 16B data + 1B dummy | 1B dummy END
#define BC_LCD_FRAMEBUFFER_SIZE (1 + ((1+16+1) * 128) + 1)

typedef struct bc_module_lcd_framebuffer_t
{
    uint8_t framebuffer[BC_LCD_FRAMEBUFFER_SIZE];

} bc_module_lcd_framebuffer_t;

typedef enum
{
    BC_MODULE_LCD_ROTATION_0   = 0,
    BC_MODULE_LCD_ROTATION_90  = 1,
    BC_MODULE_LCD_ROTATION_180 = 2,
    BC_MODULE_LCD_ROTATION_270 = 3,

} bc_module_lcd_rotation_t;

bc_module_lcd_framebuffer_t _bc_module_lcd_framebuffer;

//! @brief Initialize lcd
//! @param[in] bc_module_lcd_framebuffer_t framebuffer

void bc_module_lcd_init(bc_module_lcd_framebuffer_t *framebuffer);

//! @brief Lcd on

void bc_module_lcd_on(void);

//! @brief Lcd off

void bc_module_lcd_off(void);

//! @brief Lcd clear

void bc_module_lcd_clear(void);

//! @brief Lcd draw pixel
//! @param[in] int left
//! @param[in] int top
//! @param[in] bool value

void bc_module_lcd_draw_pixel(int left, int top, bool value);

//! @brief Lcd draw char
//! @param[in] int left
//! @param[in] int top
//! @param[in] uint8_t ch
//! @return width of printed character

int bc_module_lcd_draw_char(int left, int top, uint8_t ch);

//! @brief Lcd draw string
//! @param[in] int left
//! @param[in] int top
//! @param[in] char *str
//! @return width of printed characters

int bc_module_lcd_draw_string(int left, int top, char *str);

//void bc_module_lcd_draw(const uint8_t *frame, uint8_t width, uint8_t height); // In pixels
//void bc_module_lcd_printf(uint8_t line, /*uint8_t size, font, */const uint8_t *string/*, ...*/);

//! @brief Lcd update, send data

void bc_module_lcd_update(void);

//! @brief Lcd set font
//! @param[in] tFont *font

void bc_module_lcd_set_font(const tFont *font);

//! @brief Lcd set rotation
//! @param[in] bc_module_lcd_rotation_t rotation

void bc_module_lcd_set_rotation(bc_module_lcd_rotation_t rotation);

//! @brief Lcd get rotation
//! @return bc_module_lcd_rotation_t

bc_module_lcd_rotation_t bc_module_lcd_get_rotation(void);

//! @}

#endif /* _BC_MODULE_LCD */
