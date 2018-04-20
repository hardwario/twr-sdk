#include <application.h>

#define BLACK true

void application_init(void)
{
    // Initialize LCD
    // The parameter is internal buffer in SDK, no need to define it
    bc_module_lcd_init();

    // Init default font, this is necessary
    // See other fonts in sdk/bcl/inc/bc_font_common.h
    bc_module_lcd_set_font(&bc_font_ubuntu_15);

    // Draw string at X, Y location
    bc_module_lcd_draw_string(10, 5, "Hello world!", BLACK);

    bc_module_lcd_draw_line(5, 20, 115, 20, BLACK);

    // Use big font
    bc_module_lcd_set_font(&bc_font_ubuntu_24);
    bc_module_lcd_draw_string(10, 40, "Big", BLACK);

    // Set back default font
    bc_module_lcd_set_font(&bc_font_ubuntu_15);
    bc_module_lcd_draw_string(60, 50, "world", BLACK);

    bc_module_lcd_draw_line(10, 65, 100, 75, BLACK);

    // Don't forget to update
    bc_module_lcd_update();
}
