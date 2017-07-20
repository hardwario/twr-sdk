#ifndef _BC_MODULE_LCD
#define _BC_MODULE_LCD


#include <bc_font_common.h>
#include <bc_led.h>
#include <bc_button.h>

//! @addtogroup bc_module_lcd bc_module_lcd
//! @brief Driver for lcd
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Done event
    BC_MODULE_LCD_EVENT_DONE = 0

} bc_module_lcd_event_t;



// See app note https://www.silabs.com/documents/public/application-notes/AN0048.pdf
// Figure 3.1
// 1B mode | 1B addr + 16B data + 1B dummy | 1B dummy END
#define BC_LCD_FRAMEBUFFER_SIZE (1 + ((1+16+1) * 128) + 1)

//! @cond

typedef struct bc_module_lcd_framebuffer_t
{
    uint8_t framebuffer[BC_LCD_FRAMEBUFFER_SIZE];

} bc_module_lcd_framebuffer_t;

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief LCD rotation 0 degrees
    BC_MODULE_LCD_ROTATION_0   = 0,

    //! @brief LCD rotation 90 degrees
    BC_MODULE_LCD_ROTATION_90  = 1,

    //! @brief LCD rotation 180 degrees
    BC_MODULE_LCD_ROTATION_180 = 2,

    //! @brief LCD rotation 270 degrees
    BC_MODULE_LCD_ROTATION_270 = 3

} bc_module_lcd_rotation_t;

//! @brief Virtual LED channels

typedef enum
{
    //! @brief LCD red LED channel
    BC_MODULE_LCD_LED_RED = 0,

    //! @brief LCD green LED channel
    BC_MODULE_LCD_LED_GREEN  = 1,

    //! @brief LCD blue LED channel
    BC_MODULE_LCD_LED_BLUE = 2

} bc_module_lcd_led_t;

//! @brief Virtual button channels

typedef enum
{
    //! @brief LCD left button channel
    BC_MODULE_LCD_BUTTON_LEFT = 0,

    //! @brief LCD right button channel
    BC_MODULE_LCD_BUTTON_RIGHT  = 1

} bc_module_lcd_button_t;

//! @brief LCD frame buffer instance

bc_module_lcd_framebuffer_t _bc_module_lcd_framebuffer;

//! @brief Initialize lcd
//! @param[in] framebuffer

void bc_module_lcd_init(bc_module_lcd_framebuffer_t *framebuffer);

//! @brief Lcd on
//! @return true On success
//! @return false On failure

bool bc_module_lcd_on(void);

//! @brief Lcd off
//! @return true On success
//! @return false On failure

bool bc_module_lcd_off(void);

//! @brief Check if lcd is ready for commands
//! @return true If ready
//! @return false If not ready

bool bc_module_lcd_is_ready(void);

//! @brief Lcd clear

void bc_module_lcd_clear(void);

//! @brief Lcd draw pixel
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] value Pixels state

void bc_module_lcd_draw_pixel(int left, int top, bool value);

//! @brief Lcd draw char
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] ch Char to be printed
//! @return Width of printed character

int bc_module_lcd_draw_char(int left, int top, uint8_t ch, bool color);

//! @brief Lcd draw string
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] *str String to be printed
//! @return Width of printed string

int bc_module_lcd_draw_string(int left, int top, char *str, bool color);

//void bc_module_lcd_draw(const uint8_t *frame, uint8_t width, uint8_t height); // In pixels
//void bc_module_lcd_printf(uint8_t line, /*uint8_t size, font, */const uint8_t *string/*, ...*/);

//! @brief Lcd update, send data
//! @return true On success
//! @return false On failure

bool bc_module_lcd_update(void);

//! @brief Send Lcd clear memory command
//! @return true On success
//! @return false On failure

bool bc_module_lcd_clear_memory_command(void);

//! @brief Lcd set font
//! @param[in] *font Font

void bc_module_lcd_set_font(const bc_font_t *font);

//! @brief Lcd set rotation
//! @param[in] rotation Rotation of diplay

void bc_module_lcd_set_rotation(bc_module_lcd_rotation_t rotation);

//! @brief Lcd get rotation
//! @return Rotation of display

bc_module_lcd_rotation_t bc_module_lcd_get_rotation(void);

//! @brief Lcd get led driver
//! @return Driver for onboard led

const bc_led_driver_t *bc_module_lcd_get_led_driver(void);

//! @brief Lcd get button driver
//! @return Driver for onboard button

const bc_button_driver_t *bc_module_lcd_get_button_driver(void);

//! @}

#endif /* _BC_MODULE_LCD */
