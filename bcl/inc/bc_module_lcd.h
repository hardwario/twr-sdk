#ifndef _BC_MODULE_LCD
#define _BC_MODULE_LCD

#include "bc_common.h"

//! @addtogroup bc_module_lcd bc_module_lcd
//! @brief Driver for lcd
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Done event
    BC_MODULE_LCD_EVENT_DONE = 0,

} bc_module_lcd_event_t;

//! @brief Initialize lcd

void bc_module_lcd_init(void);
void bc_module_lcd_on(void);
void bc_module_lcd_off(void);
void bc_module_lcd_clear(void);
void bc_module_lcd_draw(const uint8_t *frame, uint8_t width, uint8_t height); // In pixels
void bc_module_lcd_printf(uint8_t line, /*uint8_t size, font, */const uint8_t *string/*, ...*/);
void bc_module_lcd_update(void);

//! @}

#endif /* _BC_MODULE_LCD */
