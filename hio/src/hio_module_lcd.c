#include <stm32l0xx.h>
#include <hio_module_lcd.h>
#include <hio_tca9534a.h>
#include <hio_scheduler.h>
#include <hio_ls013b7dh03.h>

enum
{
    _HIO_MODULE_LCD_BACKLIGHT_PIN = HIO_TCA9534A_PIN_P0,
    _HIO_MODULE_LCD_BUTTON1_PIN = HIO_TCA9534A_PIN_P1,
    _HIO_MODULE_LCD_DISP_ON_PIN = HIO_TCA9534A_PIN_P2,
    _HIO_MODULE_LCD_BUTTON2_PIN = HIO_TCA9534A_PIN_P3,
    _HIO_MODULE_LCD_LED_GREEN_PIN = HIO_TCA9534A_PIN_P4,
    _HIO_MODULE_LCD_LED_RED_PIN = HIO_TCA9534A_PIN_P5,
    _HIO_MODULE_LCD_LED_BLUE_PIN = HIO_TCA9534A_PIN_P6,
    _HIO_MODULE_LCD_LED_DISP_CS_PIN = HIO_TCA9534A_PIN_P7
};

#define _HIO_MODULE_LCD_INITIALIZED ((1 << _HIO_MODULE_LCD_BACKLIGHT_PIN) | (1 << _HIO_MODULE_LCD_LED_DISP_CS_PIN) | (1 << _HIO_MODULE_LCD_DISP_ON_PIN) | (1 << _HIO_MODULE_LCD_LED_GREEN_PIN) | (1 << _HIO_MODULE_LCD_LED_RED_PIN) | (1 << _HIO_MODULE_LCD_LED_BLUE_PIN))

typedef struct hio_module_lcd_t
{
    void (*event_handler)(hio_module_lcd_event_t, void *);
    void *event_param;
    bool is_tca9534a_initialized;
    hio_tca9534a_t tca9534a;
    hio_ls013b7dh03_t ls013b7dh03;
    hio_gfx_t gfx;

    hio_button_t button_left;
    hio_button_t button_right;

} hio_module_lcd_t;

hio_module_lcd_t _hio_module_lcd;

static hio_tca9534a_pin_t _hio_module_lcd_led_pin_lut[3] =
{
        [HIO_MODULE_LCD_LED_RED] = HIO_TCA9534A_PIN_P5,
        [HIO_MODULE_LCD_LED_GREEN] = HIO_TCA9534A_PIN_P4,
        [HIO_MODULE_LCD_LED_BLUE] = HIO_TCA9534A_PIN_P6
};

static hio_tca9534a_pin_t _hio_module_lcd_button_pin_lut[2] =
{
        [HIO_MODULE_LCD_BUTTON_LEFT] = HIO_TCA9534A_PIN_P3,
        [HIO_MODULE_LCD_BUTTON_RIGHT] = HIO_TCA9534A_PIN_P1
};

static bool _hio_module_lcd_tca9534a_init(void);

static bool _hio_module_lcd_cs_pin_set(bool state);

static void _hio_module_lcd_led_init(hio_led_t *self);

static void _hio_module_lcd_led_on(hio_led_t *self);

static void _hio_module_lcd_led_off(hio_led_t *self);

static void _hio_module_lcd_button_init(hio_button_t *self);

static int _hio_module_lcd_button_get_input(hio_button_t *self);

void hio_module_lcd_init()
{
	_hio_module_lcd_tca9534a_init();

	hio_ls013b7dh03_init(&_hio_module_lcd.ls013b7dh03, _hio_module_lcd_cs_pin_set);

	hio_gfx_init(&_hio_module_lcd.gfx, &_hio_module_lcd.ls013b7dh03, hio_ls013b7dh03_get_driver());

	hio_gfx_clear(&_hio_module_lcd.gfx);
}

hio_gfx_t *hio_module_lcd_get_gfx()
{
    return &_hio_module_lcd.gfx;
}

bool hio_module_lcd_on(void)
{
    return hio_tca9534a_write_pin(&_hio_module_lcd.tca9534a, _HIO_MODULE_LCD_DISP_ON_PIN, 1);
}

bool hio_module_lcd_off(void)
{
    return hio_tca9534a_write_pin(&_hio_module_lcd.tca9534a, _HIO_MODULE_LCD_DISP_ON_PIN, 0);
}

bool hio_module_lcd_is_ready(void)
{
	return _hio_module_lcd_tca9534a_init() && hio_gfx_display_is_ready(&_hio_module_lcd.gfx);
}

void hio_module_lcd_clear(void)
{
    hio_gfx_clear(&_hio_module_lcd.gfx);
}

void hio_module_lcd_draw_pixel(int x, int y, bool value)
{
    hio_gfx_draw_pixel(&_hio_module_lcd.gfx, x, y, value);
}

int hio_module_lcd_draw_char(int left, int top, uint8_t ch, bool color)
{
    return hio_gfx_draw_char(&_hio_module_lcd.gfx, left, top, ch, color);
}

int hio_module_lcd_draw_string(int left, int top, char *str, bool color)
{
    return hio_gfx_draw_string(&_hio_module_lcd.gfx, left, top, str, color);
}


void hio_module_lcd_draw(const uint8_t *frame, uint8_t width, uint8_t height) // In pixels
{
    (void)frame;
    (void)width;
    (void)height;
}

void hio_module_lcd_printf(uint8_t line, /*uint8_t size, font, */const uint8_t *string/*, ...*/)
{
    (void) line;
    (void) string;
}

void hio_module_lcd_draw_line(int x0, int y0, int x1, int y1, bool color)
{
    hio_gfx_draw_line(&_hio_module_lcd.gfx, x0, y0, x1, y1, color);
}

void hio_module_lcd_draw_rectangle(int x0, int y0, int x1, int y1, bool color)
{
    hio_gfx_draw_rectangle(&_hio_module_lcd.gfx, x0, y0, x1, y1, color);
}

void hio_module_lcd_draw_circle(int x0, int y0, int radius, bool color)
{
    hio_gfx_draw_circle(&_hio_module_lcd.gfx, x0, y0, radius, color);
}

void hio_module_lcd_draw_image(int left, int top, const hio_image_t *img)
{
    uint8_t line;
	uint8_t row;
    uint8_t bytes_per_row = img->width / 8;

    if(img->width % 8 != 0)
    {
        bytes_per_row++;
    }

    hio_tick_t start = hio_tick_get();

        for (row = 0; row < img->height; row++) {
            for (line = 0; line < img->width; line++) {
                uint32_t byte_offset = line / 8 + row * bytes_per_row;
                uint32_t bit = line % 8;
                hio_gfx_draw_pixel(&_hio_module_lcd.gfx, line + left, row + top, (img->data[byte_offset]) & (1 << bit));
			}
		}
    volatile hio_tick_t duration = hio_tick_get() - start;
    (void)duration;

}

bool hio_module_lcd_update(void)
{
    return hio_gfx_update(&_hio_module_lcd.gfx);
}

void hio_module_lcd_set_font(const hio_font_t *font)
{
    hio_gfx_set_font(&_hio_module_lcd.gfx, font);
}

static void _hio_module_lcd_button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) event_param;

    if (self == &_hio_module_lcd.button_left)
    {
        if (event == HIO_BUTTON_EVENT_PRESS)
        {
            _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_LEFT_PRESS, _hio_module_lcd.event_param);
        }
        if (event == HIO_BUTTON_EVENT_RELEASE)
        {
            _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_LEFT_RELEASE, _hio_module_lcd.event_param);
        }
        if (event == HIO_BUTTON_EVENT_CLICK)
        {
            _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_LEFT_CLICK, _hio_module_lcd.event_param);
        }
        if (event == HIO_BUTTON_EVENT_HOLD)
        {
            if (_hio_module_lcd.button_right._state)
            {
                _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_BOTH_HOLD, _hio_module_lcd.event_param);
                // Force _hold_signalized to true, so the hold event of the second button won't trigger which would cause event duplication
                _hio_module_lcd.button_right._hold_signalized = true;
            }
            else
            {
                _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_LEFT_HOLD, _hio_module_lcd.event_param);
            }
        }
    }

    if (self == &_hio_module_lcd.button_right)
    {
        if (event == HIO_BUTTON_EVENT_PRESS)
        {
            _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_RIGHT_PRESS, _hio_module_lcd.event_param);
        }
        if (event == HIO_BUTTON_EVENT_RELEASE)
        {
            _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_RIGHT_RELEASE, _hio_module_lcd.event_param);
        }
        if (event == HIO_BUTTON_EVENT_CLICK)
        {
            _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_RIGHT_CLICK, _hio_module_lcd.event_param);
        }
        if (event == HIO_BUTTON_EVENT_HOLD)
        {
            if (_hio_module_lcd.button_left._state)
            {
                _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_BOTH_HOLD, _hio_module_lcd.event_param);
                // Force _hold_signalized to true, so the hold event of the second button won't trigger which would cause event duplication
                _hio_module_lcd.button_left._hold_signalized = true;
            }
            else
            {
                _hio_module_lcd.event_handler(HIO_MODULE_LCD_EVENT_RIGHT_HOLD, _hio_module_lcd.event_param);
            }
        }
    }
}

void hio_module_lcd_set_event_handler(void (*event_handler)(hio_module_lcd_event_t, void *), void *event_param)
{
    _hio_module_lcd.event_handler = event_handler;
    _hio_module_lcd.event_param = event_param;

    const hio_button_driver_t* lcdButtonDriver =  hio_module_lcd_get_button_driver();

    hio_button_init_virtual(&_hio_module_lcd.button_left, 0, lcdButtonDriver, 0);
    hio_button_init_virtual(&_hio_module_lcd.button_right, 1, lcdButtonDriver, 0);

    hio_button_set_event_handler(&_hio_module_lcd.button_left, _hio_module_lcd_button_event_handler, (int*)0);
    hio_button_set_event_handler(&_hio_module_lcd.button_right, _hio_module_lcd_button_event_handler, (int*)1);
}

void hio_module_lcd_set_button_hold_time(hio_tick_t hold_time)
{
    hio_button_set_hold_time(&_hio_module_lcd.button_left, hold_time);
    hio_button_set_hold_time(&_hio_module_lcd.button_right, hold_time);
}

void hio_module_lcd_set_button_scan_interval(hio_tick_t scan_interval)
{
    hio_button_set_scan_interval(&_hio_module_lcd.button_left, scan_interval);
    hio_button_set_scan_interval(&_hio_module_lcd.button_right, scan_interval);
}

void hio_module_lcd_set_button_debounce_time(hio_tick_t debounce_time)
{
    hio_button_set_debounce_time(&_hio_module_lcd.button_left, debounce_time);
    hio_button_set_debounce_time(&_hio_module_lcd.button_right, debounce_time);
}

void hio_module_lcd_set_button_click_timeout(hio_tick_t click_timeout)
{
    hio_button_set_click_timeout(&_hio_module_lcd.button_left, click_timeout);
    hio_button_set_click_timeout(&_hio_module_lcd.button_right, click_timeout);
}

void hio_module_lcd_set_rotation(hio_module_lcd_rotation_t rotation)
{
    hio_gfx_set_rotation(&_hio_module_lcd.gfx, rotation);
}

hio_module_lcd_rotation_t hio_module_lcd_get_rotation(void)
{
    return hio_gfx_get_rotation(&_hio_module_lcd.gfx);
}

const hio_led_driver_t *hio_module_lcd_get_led_driver(void)
{
    static const hio_led_driver_t hio_module_lcd_led_driver =
    {
        .init = _hio_module_lcd_led_init,
        .on = _hio_module_lcd_led_on,
        .off = _hio_module_lcd_led_off,
    };

    return &hio_module_lcd_led_driver;
}

const hio_button_driver_t *hio_module_lcd_get_button_driver(void)
{
    static const hio_button_driver_t hio_module_lcd_button_driver =
    {
        .init = _hio_module_lcd_button_init,
        .get_input = _hio_module_lcd_button_get_input,
    };

    return &hio_module_lcd_button_driver;
}

static bool _hio_module_lcd_tca9534a_init(void)
{
	if (!_hio_module_lcd.is_tca9534a_initialized)
	{
		if (!hio_tca9534a_init(&_hio_module_lcd.tca9534a, HIO_I2C_I2C0, 0x3c))
		{
			return false;
		}

		if (!hio_tca9534a_write_port(&_hio_module_lcd.tca9534a, _HIO_MODULE_LCD_INITIALIZED))
		{
			return false;
		}

		if (!hio_tca9534a_set_port_direction(&_hio_module_lcd.tca9534a, (1 << _HIO_MODULE_LCD_BUTTON1_PIN) | (1 << _HIO_MODULE_LCD_BUTTON2_PIN)))
		{
			return false;
		}

		_hio_module_lcd.is_tca9534a_initialized = true;
	}

	return true;
}

static bool _hio_module_lcd_cs_pin_set(bool state)
{
    if (!_hio_module_lcd_tca9534a_init())
    {
        return false;
    }

    if (!hio_tca9534a_write_pin(&_hio_module_lcd.tca9534a, _HIO_MODULE_LCD_LED_DISP_CS_PIN, state))
    {
        _hio_module_lcd.is_tca9534a_initialized = false;

        return false;
    }

    return true;
}

static void _hio_module_lcd_led_init(hio_led_t *self)
{
    (void) self;

    _hio_module_lcd_tca9534a_init();
}

static void _hio_module_lcd_led_on(hio_led_t *self)
{
    if (!hio_tca9534a_write_pin(&_hio_module_lcd.tca9534a, _hio_module_lcd_led_pin_lut[self->_channel.virtual], self->_idle_state ? 0 : 1))
    {
    	_hio_module_lcd.is_tca9534a_initialized = false;
    }
}

static void _hio_module_lcd_led_off(hio_led_t *self)
{
    if (!hio_tca9534a_write_pin(&_hio_module_lcd.tca9534a, _hio_module_lcd_led_pin_lut[self->_channel.virtual], self->_idle_state ? 1 : 0))
    {
    	_hio_module_lcd.is_tca9534a_initialized = false;
    }
}

static void _hio_module_lcd_button_init(hio_button_t *self)
{
    (void) self;

    _hio_module_lcd_tca9534a_init();

    hio_gpio_set_mode(HIO_GPIO_BUTTON, HIO_GPIO_MODE_INPUT);
    hio_gpio_init(HIO_GPIO_BUTTON);
}

static int _hio_module_lcd_button_get_input(hio_button_t *self)
{
    if (hio_gpio_get_input(HIO_GPIO_BUTTON) == 0)
    {
        return 0;
    }

    int state;

    if (!hio_tca9534a_read_pin(&_hio_module_lcd.tca9534a, _hio_module_lcd_button_pin_lut[self->_channel.virtual], &state))
    {
    	_hio_module_lcd.is_tca9534a_initialized = false;
    	return 0;
    }

    return state;
}
