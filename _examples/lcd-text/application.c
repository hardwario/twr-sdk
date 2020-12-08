#include <application.h>

#define BLACK true

void application_init(void)
{
    // Initialize LCD
    // The parameter is internal buffer in SDK, no need to define it
    twr_module_lcd_init();

    // Init default font, this is necessary
    // See other fonts in sdk/bcl/inc/twr_font_common.h
    twr_module_lcd_set_font(&twr_font_ubuntu_15);

    // Draw string at X, Y location
    twr_module_lcd_draw_string(10, 5, "Hello world!", BLACK);

    twr_module_lcd_draw_line(5, 20, 115, 20, BLACK);

    // Use big font
    twr_module_lcd_set_font(&twr_font_ubuntu_24);
    twr_module_lcd_draw_string(10, 40, "Big", BLACK);

    // Set back default font
    twr_module_lcd_set_font(&twr_font_ubuntu_15);
    twr_module_lcd_draw_string(60, 50, "world", BLACK);

    twr_module_lcd_draw_line(10, 65, 100, 75, BLACK);

    // Don't forget to update
    twr_module_lcd_update();
}
