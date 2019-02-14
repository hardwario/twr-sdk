#include <application.h>
#include <bc_apds9960.h>

// LED instance
static bc_led_t led;

// Button instance
static bc_button_t button;

static bc_apds9960_t apds9960;

void apds9960_event_handler(bc_apds9960_t *self, bc_apds9960_event_t event, void *event_param)
{
    if (event == BC_APDS9960_EVENT_UPDATE)
    {
        bc_apds9960_gesture_t gesture;
        bc_apds9960_get_gesture(self, &gesture);
        char tmp[64];
        char *p[] = {
            "NONE",
            "LEFT",
            "RIGHT",
            "UP",
            "DOWN",
            "NEAR",
            "FAR",
            "ALL"
        };

        sprintf(tmp, "gesture %s", p[gesture]);

        bc_module_lcd_clear();

        bc_module_lcd_set_font(&bc_font_ubuntu_13);

        bc_module_lcd_draw_string(5, 5, tmp, 1);

        for (int i = 0; i < 68; i++)
        {
            switch(gesture)
            {
                case BC_APDS9960_GESTURE_UP:
                case BC_APDS9960_GESTURE_DOWN:
                    bc_module_lcd_draw_pixel(64, 30 + i, 1);
                    break;
                case BC_APDS9960_GESTURE_LEFT:
                case BC_APDS9960_GESTURE_RIGHT:
                    bc_module_lcd_draw_pixel(30 + i, 64, 1);
                    break;
                default:
                    break;
            }

            if (i < 40)
            {
                switch(gesture)
                {
                    case BC_APDS9960_GESTURE_UP:
                        bc_module_lcd_draw_pixel(64 - i, 30 + i, 1);
                        bc_module_lcd_draw_pixel(64 + i, 30 + i, 1);
                        break;
                    case BC_APDS9960_GESTURE_DOWN:
                        bc_module_lcd_draw_pixel(64 - i, 98 - i, 1);
                        bc_module_lcd_draw_pixel(64 + i, 98 - i, 1);
                        break;
                    case BC_APDS9960_GESTURE_LEFT:
                        bc_module_lcd_draw_pixel(30 + i, 64 - i, 1);
                        bc_module_lcd_draw_pixel(30 + i, 64 + i, true);
                        break;
                    case BC_APDS9960_GESTURE_RIGHT:
                        bc_module_lcd_draw_pixel(98 - i, 64 - i, 1);
                        bc_module_lcd_draw_pixel(98 - i, 64 + i, 1);
                        break;
                    default:
                        break;
                }
            }
        }

        bc_module_lcd_update();
    }
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);
    }
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_ON);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_module_lcd_init();
    bc_module_lcd_clear();
    bc_module_lcd_update();

    bc_apds9960_init(&apds9960, BC_I2C_I2C0, 0x39);
    bc_apds9960_set_event_handler(&apds9960, apds9960_event_handler, NULL);
}



