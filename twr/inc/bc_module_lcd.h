#ifndef _BC_MODULE_LCD
#define _BC_MODULE_LCD


#include <bc_font_common.h>
#include <bc_image.h>
#include <bc_led.h>
#include <bc_button.h>
#include <bc_gfx.h>

//! @addtogroup bc_module_lcd bc_module_lcd
//! @brief Driver for lcd
//! @{

//! @brief Callback events

typedef enum
{
    BC_MODULE_LCD_EVENT_LEFT_PRESS = 0x10 | BC_BUTTON_EVENT_PRESS,
    BC_MODULE_LCD_EVENT_LEFT_RELEASE = 0x10 | BC_BUTTON_EVENT_RELEASE,
    BC_MODULE_LCD_EVENT_LEFT_CLICK = 0x10 | BC_BUTTON_EVENT_CLICK,
    BC_MODULE_LCD_EVENT_LEFT_HOLD = 0x10 | BC_BUTTON_EVENT_HOLD,

    BC_MODULE_LCD_EVENT_RIGHT_PRESS = 0x20 | BC_BUTTON_EVENT_PRESS,
    BC_MODULE_LCD_EVENT_RIGHT_RELEASE = 0x20 | BC_BUTTON_EVENT_RELEASE,
    BC_MODULE_LCD_EVENT_RIGHT_CLICK = 0x20 | BC_BUTTON_EVENT_CLICK,
    BC_MODULE_LCD_EVENT_RIGHT_HOLD = 0x20 | BC_BUTTON_EVENT_HOLD,

    BC_MODULE_LCD_EVENT_BOTH_HOLD = 0x30 | BC_BUTTON_EVENT_HOLD,

} bc_module_lcd_event_t;


//! @brief Rotation

typedef enum
{
    //! @brief LCD rotation 0 degrees
    BC_MODULE_LCD_ROTATION_0   = BC_GFX_ROTATION_0,

    //! @brief LCD rotation 90 degrees
    BC_MODULE_LCD_ROTATION_90  = BC_GFX_ROTATION_90,

    //! @brief LCD rotation 180 degrees
    BC_MODULE_LCD_ROTATION_180 = BC_GFX_ROTATION_180,

    //! @brief LCD rotation 270 degrees
    BC_MODULE_LCD_ROTATION_270 = BC_GFX_ROTATION_270

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


//! @brief Initialize lcd

void bc_module_lcd_init();

//! @brief Get gfx instance

bc_gfx_t *bc_module_lcd_get_gfx();

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
//! @param[in] color Pixels state

void bc_module_lcd_draw_pixel(int left, int top, bool color);

//! @brief Lcd draw char
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] ch Char to be printed
//! @param[in] color Pixels state
//! @return Width of printed character

int bc_module_lcd_draw_char(int left, int top, uint8_t ch, bool color);

//! @brief Lcd draw string
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] *str String to be printed
//! @param[in] color Pixels state
//! @return Width of printed string

int bc_module_lcd_draw_string(int left, int top, char *str, bool color);

//! @brief Lcd draw line
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color Pixels state

void bc_module_lcd_draw_line(int x0, int y0, int x1, int y1, bool color);

//! @brief Lcd draw rectangle
//! @param[in] x0 Pixels from left edge
//! @param[in] y0 Pixels from top edge
//! @param[in] x1 Pixels from left edge
//! @param[in] y1 Pixels from top edge
//! @param[in] color Pixels state

void bc_module_lcd_draw_rectangle(int x0, int y0, int x1, int y1, bool color);

//! @brief Lcd draw circle
//! @param[in] x0 Center - pixels from left edge
//! @param[in] y0 Center - pixels from top edge
//! @param[in] radius In pixels
//! @param[in] color Pixels state

void bc_module_lcd_draw_circle(int x0, int y0, int radius, bool color);

//! @brief Lcd draw image
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] img Pointer to the image

void bc_module_lcd_draw_image(int left, int top, const bc_image_t *img);

//void bc_module_lcd_draw(const uint8_t *frame, uint8_t width, uint8_t height); // In pixels
//void bc_module_lcd_printf(uint8_t line, /*uint8_t size, font, */const uint8_t *string/*, ...*/);

//! @brief Lcd update, send data
//! @return true On success
//! @return false On failure

bool bc_module_lcd_update(void);

//! @brief Lcd set font
//! @param[in] *font Font

void bc_module_lcd_set_font(const bc_font_t *font);

//! @brief Lcd set rotation
//! @param[in] rotation Rotation of diplay

void bc_module_lcd_set_rotation(bc_module_lcd_rotation_t rotation);

//! @brief Lcd set event handler for buttons
//! @param[in] event_handler Event handler
//! @param[in] event_param Event parameter

void bc_module_lcd_set_event_handler(void (*event_handler)(bc_module_lcd_event_t, void *), void *event_param);

//! @brief Lcd get rotation
//! @return Rotation of display

bc_module_lcd_rotation_t bc_module_lcd_get_rotation(void);

//! @brief Lcd get led driver
//! @return Driver for onboard led

const bc_led_driver_t *bc_module_lcd_get_led_driver(void);

//! @brief Lcd get button driver
//! @return Driver for onboard button

const bc_button_driver_t *bc_module_lcd_get_button_driver(void);

//! @brief Set hold time (interval after which hold event is recognized when button is steadily pressed)
//! @param[in] hold_time Desired hold time in ticks

void bc_module_lcd_set_button_hold_time(bc_tick_t hold_time);

//! @brief Set scan interval (period of button input sampling)
//! @param[in] scan_interval Desired scan interval in ticks

void bc_module_lcd_set_button_scan_interval(bc_tick_t scan_interval);

//! @brief Set debounce time (minimum sampling interval during which input cannot change to toggle its state)
//! @param[in] debounce_time Desired debounce time in ticks

void bc_module_lcd_set_button_debounce_time(bc_tick_t debounce_time);

//! @brief Set click timeout (maximum interval within which button has to be released to recognize click event)
//! @param[in] click_timeout Desired click timeout in ticks

void bc_module_lcd_set_button_click_timeout(bc_tick_t click_timeout);

//! @}

#endif // _BC_MODULE_LCD
