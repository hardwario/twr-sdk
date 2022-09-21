#include <application.h>

twr_button_t button;
twr_led_t lcdLed;

twr_gfx_t *pgfx;

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_led_pulse(&lcdLed, 1500);

        char hello[6] = "Hello";
        twr_gfx_draw_string(pgfx, 10, 5, hello, true);
        twr_gfx_draw_line(pgfx, 0, 21, 128, 23, true);

        twr_gfx_update(pgfx);
    }
}

void application_init(void)
{
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    const twr_led_driver_t* driver = twr_module_lcd_get_led_driver();
    twr_led_init_virtual(&lcdLed, TWR_MODULE_LCD_LED_BLUE, driver, 1);

    twr_module_lcd_init();
    pgfx = twr_module_lcd_get_gfx();
    twr_gfx_set_font(pgfx, &twr_font_ubuntu_15);
}
