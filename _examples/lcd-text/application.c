#include <application.h>

void application_init(void)
{
    // Initialize LCD
    // The parameter is internal buffer in SDK, no need to define it
    bc_module_lcd_init(&_bc_module_lcd_framebuffer);

    // Draw string at X, Y location
    bc_module_lcd_draw_string(10, 5, "Hello world!");

    // Use big font
    bc_module_lcd_set_font(&FontBig);
    bc_module_lcd_draw_string(10, 40, "Big");

    // Set back default font
    bc_module_lcd_set_font(&Font);
    bc_module_lcd_draw_string(60, 50, "world");

    // Don't forget to update
    bc_module_lcd_update();
}
