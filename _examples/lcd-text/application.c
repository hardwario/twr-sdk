#include <application.h>

#define BLACK true

void application_init(void)
{
    // Initialize LCD
    // The parameter is internal buffer in SDK, no need to define it
    hio_module_lcd_init();

    // Init default font, this is necessary
    // See other fonts in sdk/bcl/inc/hio_font_common.h
    hio_module_lcd_set_font(&hio_font_ubuntu_15);

    // Draw string at X, Y location
    hio_module_lcd_draw_string(10, 5, "Hello world!", BLACK);

    hio_module_lcd_draw_line(5, 20, 115, 20, BLACK);

    // Use big font
    hio_module_lcd_set_font(&hio_font_ubuntu_24);
    hio_module_lcd_draw_string(10, 40, "Big", BLACK);

    // Set back default font
    hio_module_lcd_set_font(&hio_font_ubuntu_15);
    hio_module_lcd_draw_string(60, 50, "world", BLACK);

    hio_module_lcd_draw_line(10, 65, 100, 75, BLACK);

    // Don't forget to update
    hio_module_lcd_update();
}
