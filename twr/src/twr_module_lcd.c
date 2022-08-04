#include <stm32l0xx.h>
#include <twr_module_lcd.h>
#include <twr_tca9534a.h>
#include <twr_scheduler.h>
#include <twr_ls013b7dh03.h>

enum
{
    _TWR_MODULE_LCD_BACKLIGHT_PIN = TWR_TCA9534A_PIN_P0,
    _TWR_MODULE_LCD_BUTTON1_PIN = TWR_TCA9534A_PIN_P1,
    _TWR_MODULE_LCD_DISP_ON_PIN = TWR_TCA9534A_PIN_P2,
    _TWR_MODULE_LCD_BUTTON2_PIN = TWR_TCA9534A_PIN_P3,
    _TWR_MODULE_LCD_LED_GREEN_PIN = TWR_TCA9534A_PIN_P4,
    _TWR_MODULE_LCD_LED_RED_PIN = TWR_TCA9534A_PIN_P5,
    _TWR_MODULE_LCD_LED_BLUE_PIN = TWR_TCA9534A_PIN_P6,
    _TWR_MODULE_LCD_LED_DISP_CS_PIN = TWR_TCA9534A_PIN_P7
};

#define _TWR_MODULE_LCD_INITIALIZED ((1 << _TWR_MODULE_LCD_BACKLIGHT_PIN) | (1 << _TWR_MODULE_LCD_LED_DISP_CS_PIN) | (1 << _TWR_MODULE_LCD_DISP_ON_PIN) | (1 << _TWR_MODULE_LCD_LED_GREEN_PIN) | (1 << _TWR_MODULE_LCD_LED_RED_PIN) | (1 << _TWR_MODULE_LCD_LED_BLUE_PIN))

typedef struct twr_module_lcd_t
{
    void (*event_handler)(twr_module_lcd_event_t, void *);
    void *event_param;
    bool is_tca9534a_initialized;
    twr_tca9534a_t tca9534a;
    twr_ls013b7dh03_t ls013b7dh03;
    twr_gfx_t gfx;

    twr_button_t button_left;
    twr_button_t button_right;

} twr_module_lcd_t;

twr_module_lcd_t _twr_module_lcd;

static twr_tca9534a_pin_t _twr_module_lcd_led_pin_lut[3] =
{
        [TWR_MODULE_LCD_LED_RED] = TWR_TCA9534A_PIN_P5,
        [TWR_MODULE_LCD_LED_GREEN] = TWR_TCA9534A_PIN_P4,
        [TWR_MODULE_LCD_LED_BLUE] = TWR_TCA9534A_PIN_P6
};

static twr_tca9534a_pin_t _twr_module_lcd_button_pin_lut[2] =
{
        [TWR_MODULE_LCD_BUTTON_LEFT] = TWR_TCA9534A_PIN_P3,
        [TWR_MODULE_LCD_BUTTON_RIGHT] = TWR_TCA9534A_PIN_P1
};

static bool _twr_module_lcd_tca9534a_init(void);

static bool _twr_module_lcd_cs_pin_set(bool state);

static void _twr_module_lcd_led_init(twr_led_t *self);

static void _twr_module_lcd_led_on(twr_led_t *self);

static void _twr_module_lcd_led_off(twr_led_t *self);

static void _twr_module_lcd_button_init(twr_button_t *self);

static int _twr_module_lcd_button_get_input(twr_button_t *self);

void twr_module_lcd_init()
{
	_twr_module_lcd_tca9534a_init();

	twr_ls013b7dh03_init(&_twr_module_lcd.ls013b7dh03, _twr_module_lcd_cs_pin_set);

	twr_gfx_init(&_twr_module_lcd.gfx, &_twr_module_lcd.ls013b7dh03, twr_ls013b7dh03_get_driver());

	twr_gfx_clear(&_twr_module_lcd.gfx);
}

twr_gfx_t *twr_module_lcd_get_gfx()
{
    return &_twr_module_lcd.gfx;
}

bool twr_module_lcd_on(void)
{
    return twr_tca9534a_write_pin(&_twr_module_lcd.tca9534a, (twr_tca9534a_pin_t) _TWR_MODULE_LCD_DISP_ON_PIN, 1);
}

bool twr_module_lcd_off(void)
{
    return twr_tca9534a_write_pin(&_twr_module_lcd.tca9534a, (twr_tca9534a_pin_t) _TWR_MODULE_LCD_DISP_ON_PIN, 0);
}

bool twr_module_lcd_is_present(void)
{
    return _twr_module_lcd_tca9534a_init();
}

bool twr_module_lcd_is_ready(void)
{
	return _twr_module_lcd_tca9534a_init() && twr_gfx_display_is_ready(&_twr_module_lcd.gfx);
}

void twr_module_lcd_clear(void)
{
    twr_gfx_clear(&_twr_module_lcd.gfx);
}

void twr_module_lcd_draw_pixel(int x, int y, bool value)
{
    twr_gfx_draw_pixel(&_twr_module_lcd.gfx, x, y, value);
}

int twr_module_lcd_draw_char(int left, int top, uint8_t ch, bool color)
{
    return twr_gfx_draw_char(&_twr_module_lcd.gfx, left, top, ch, color);
}

int twr_module_lcd_draw_string(int left, int top, char *str, bool color)
{
    return twr_gfx_draw_string(&_twr_module_lcd.gfx, left, top, str, color);
}


void twr_module_lcd_draw(const uint8_t *frame, uint8_t width, uint8_t height) // In pixels
{
    (void)frame;
    (void)width;
    (void)height;
}

void twr_module_lcd_printf(uint8_t line, /*uint8_t size, font, */const uint8_t *string/*, ...*/)
{
    (void) line;
    (void) string;
}

void twr_module_lcd_draw_line(int x0, int y0, int x1, int y1, bool color)
{
    twr_gfx_draw_line(&_twr_module_lcd.gfx, x0, y0, x1, y1, color);
}

void twr_module_lcd_draw_rectangle(int x0, int y0, int x1, int y1, bool color)
{
    twr_gfx_draw_rectangle(&_twr_module_lcd.gfx, x0, y0, x1, y1, color);
}

void twr_module_lcd_draw_circle(int x0, int y0, int radius, bool color)
{
    twr_gfx_draw_circle(&_twr_module_lcd.gfx, x0, y0, radius, color);
}

void twr_module_lcd_draw_image(int left, int top, const twr_image_t *img)
{
    uint8_t line;
	uint8_t row;
    uint8_t bytes_per_row = img->width / 8;

    if(img->width % 8 != 0)
    {
        bytes_per_row++;
    }

    twr_tick_t start = twr_tick_get();

        for (row = 0; row < img->height; row++) {
            for (line = 0; line < img->width; line++) {
                uint32_t byte_offset = line / 8 + row * bytes_per_row;
                uint32_t bit = line % 8;
                twr_gfx_draw_pixel(&_twr_module_lcd.gfx, line + left, row + top, (img->data[byte_offset]) & (1 << bit));
			}
		}
    volatile twr_tick_t duration = twr_tick_get() - start;
    (void)duration;

}

bool twr_module_lcd_update(void)
{
    return twr_gfx_update(&_twr_module_lcd.gfx);
}

void twr_module_lcd_set_font(const twr_font_t *font)
{
    twr_gfx_set_font(&_twr_module_lcd.gfx, font);
}

static void _twr_module_lcd_button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) event_param;

    if (self == &_twr_module_lcd.button_left)
    {
        if (event == TWR_BUTTON_EVENT_PRESS)
        {
            _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_LEFT_PRESS, _twr_module_lcd.event_param);
        }
        if (event == TWR_BUTTON_EVENT_RELEASE)
        {
            _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_LEFT_RELEASE, _twr_module_lcd.event_param);
        }
        if (event == TWR_BUTTON_EVENT_CLICK)
        {
            _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_LEFT_CLICK, _twr_module_lcd.event_param);
        }
        if (event == TWR_BUTTON_EVENT_HOLD)
        {
            if (_twr_module_lcd.button_right._state)
            {
                _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_BOTH_HOLD, _twr_module_lcd.event_param);
                // Force _hold_signalized to true, so the hold event of the second button won't trigger which would cause event duplication
                _twr_module_lcd.button_right._hold_signalized = true;
            }
            else
            {
                _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_LEFT_HOLD, _twr_module_lcd.event_param);
            }
        }
    }

    if (self == &_twr_module_lcd.button_right)
    {
        if (event == TWR_BUTTON_EVENT_PRESS)
        {
            _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_RIGHT_PRESS, _twr_module_lcd.event_param);
        }
        if (event == TWR_BUTTON_EVENT_RELEASE)
        {
            _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_RIGHT_RELEASE, _twr_module_lcd.event_param);
        }
        if (event == TWR_BUTTON_EVENT_CLICK)
        {
            _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_RIGHT_CLICK, _twr_module_lcd.event_param);
        }
        if (event == TWR_BUTTON_EVENT_HOLD)
        {
            if (_twr_module_lcd.button_left._state)
            {
                _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_BOTH_HOLD, _twr_module_lcd.event_param);
                // Force _hold_signalized to true, so the hold event of the second button won't trigger which would cause event duplication
                _twr_module_lcd.button_left._hold_signalized = true;
            }
            else
            {
                _twr_module_lcd.event_handler(TWR_MODULE_LCD_EVENT_RIGHT_HOLD, _twr_module_lcd.event_param);
            }
        }
    }
}

void twr_module_lcd_set_event_handler(void (*event_handler)(twr_module_lcd_event_t, void *), void *event_param)
{
    _twr_module_lcd.event_handler = event_handler;
    _twr_module_lcd.event_param = event_param;

    const twr_button_driver_t* lcdButtonDriver =  twr_module_lcd_get_button_driver();

    twr_button_init_virtual(&_twr_module_lcd.button_left, 0, lcdButtonDriver, 0);
    twr_button_init_virtual(&_twr_module_lcd.button_right, 1, lcdButtonDriver, 0);

    twr_button_set_event_handler(&_twr_module_lcd.button_left, _twr_module_lcd_button_event_handler, (int*)0);
    twr_button_set_event_handler(&_twr_module_lcd.button_right, _twr_module_lcd_button_event_handler, (int*)1);
}

void twr_module_lcd_set_button_hold_time(twr_tick_t hold_time)
{
    twr_button_set_hold_time(&_twr_module_lcd.button_left, hold_time);
    twr_button_set_hold_time(&_twr_module_lcd.button_right, hold_time);
}

void twr_module_lcd_set_button_scan_interval(twr_tick_t scan_interval)
{
    twr_button_set_scan_interval(&_twr_module_lcd.button_left, scan_interval);
    twr_button_set_scan_interval(&_twr_module_lcd.button_right, scan_interval);
}

void twr_module_lcd_set_button_debounce_time(twr_tick_t debounce_time)
{
    twr_button_set_debounce_time(&_twr_module_lcd.button_left, debounce_time);
    twr_button_set_debounce_time(&_twr_module_lcd.button_right, debounce_time);
}

void twr_module_lcd_set_button_click_timeout(twr_tick_t click_timeout)
{
    twr_button_set_click_timeout(&_twr_module_lcd.button_left, click_timeout);
    twr_button_set_click_timeout(&_twr_module_lcd.button_right, click_timeout);
}

void twr_module_lcd_set_rotation(twr_module_lcd_rotation_t rotation)
{
    twr_gfx_set_rotation(&_twr_module_lcd.gfx, (twr_gfx_rotation_t) rotation);
}

twr_module_lcd_rotation_t twr_module_lcd_get_rotation(void)
{
    return (twr_module_lcd_rotation_t) twr_gfx_get_rotation(&_twr_module_lcd.gfx);
}

const twr_led_driver_t *twr_module_lcd_get_led_driver(void)
{
    static const twr_led_driver_t twr_module_lcd_led_driver =
    {
        .init = _twr_module_lcd_led_init,
        .on = _twr_module_lcd_led_on,
        .off = _twr_module_lcd_led_off,
    };

    return &twr_module_lcd_led_driver;
}

const twr_button_driver_t *twr_module_lcd_get_button_driver(void)
{
    static const twr_button_driver_t twr_module_lcd_button_driver =
    {
        .init = _twr_module_lcd_button_init,
        .get_input = _twr_module_lcd_button_get_input,
    };

    return &twr_module_lcd_button_driver;
}

static bool _twr_module_lcd_tca9534a_init(void)
{
	if (!_twr_module_lcd.is_tca9534a_initialized)
	{
		if (!twr_tca9534a_init(&_twr_module_lcd.tca9534a, TWR_I2C_I2C0, 0x3c))
		{
			return false;
		}

		if (!twr_tca9534a_write_port(&_twr_module_lcd.tca9534a, _TWR_MODULE_LCD_INITIALIZED))
		{
			return false;
		}

		if (!twr_tca9534a_set_port_direction(&_twr_module_lcd.tca9534a, (1 << _TWR_MODULE_LCD_BUTTON1_PIN) | (1 << _TWR_MODULE_LCD_BUTTON2_PIN)))
		{
			return false;
		}

		_twr_module_lcd.is_tca9534a_initialized = true;
	}

	return true;
}

static bool _twr_module_lcd_cs_pin_set(bool state)
{
    if (!_twr_module_lcd_tca9534a_init())
    {
        return false;
    }

    if (!twr_tca9534a_write_pin(&_twr_module_lcd.tca9534a, (twr_tca9534a_pin_t) _TWR_MODULE_LCD_LED_DISP_CS_PIN, state))
    {
        _twr_module_lcd.is_tca9534a_initialized = false;

        return false;
    }

    return true;
}

static void _twr_module_lcd_led_init(twr_led_t *self)
{
    (void) self;

    _twr_module_lcd_tca9534a_init();
}

static void _twr_module_lcd_led_on(twr_led_t *self)
{
    if (!twr_tca9534a_write_pin(&_twr_module_lcd.tca9534a, _twr_module_lcd_led_pin_lut[self->_channel.virtual], self->_idle_state ? 0 : 1))
    {
    	_twr_module_lcd.is_tca9534a_initialized = false;
    }
}

static void _twr_module_lcd_led_off(twr_led_t *self)
{
    if (!twr_tca9534a_write_pin(&_twr_module_lcd.tca9534a, _twr_module_lcd_led_pin_lut[self->_channel.virtual], self->_idle_state ? 1 : 0))
    {
    	_twr_module_lcd.is_tca9534a_initialized = false;
    }
}

static void _twr_module_lcd_button_init(twr_button_t *self)
{
    (void) self;

    _twr_module_lcd_tca9534a_init();

    twr_gpio_set_mode(TWR_GPIO_BUTTON, TWR_GPIO_MODE_INPUT);
    twr_gpio_init(TWR_GPIO_BUTTON);
}

static int _twr_module_lcd_button_get_input(twr_button_t *self)
{
    if (twr_gpio_get_input(TWR_GPIO_BUTTON) == 0)
    {
        return 0;
    }

    int state;

    if (!twr_tca9534a_read_pin(&_twr_module_lcd.tca9534a, _twr_module_lcd_button_pin_lut[self->_channel.virtual], &state))
    {
    	_twr_module_lcd.is_tca9534a_initialized = false;
    	return 0;
    }

    return state;
}
