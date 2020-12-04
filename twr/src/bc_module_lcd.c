#include <stm32l0xx.h>
#include <bc_module_lcd.h>
#include <bc_tca9534a.h>
#include <bc_scheduler.h>
#include <bc_ls013b7dh03.h>

enum
{
    _BC_MODULE_LCD_BACKLIGHT_PIN = BC_TCA9534A_PIN_P0,
    _BC_MODULE_LCD_BUTTON1_PIN = BC_TCA9534A_PIN_P1,
    _BC_MODULE_LCD_DISP_ON_PIN = BC_TCA9534A_PIN_P2,
    _BC_MODULE_LCD_BUTTON2_PIN = BC_TCA9534A_PIN_P3,
    _BC_MODULE_LCD_LED_GREEN_PIN = BC_TCA9534A_PIN_P4,
    _BC_MODULE_LCD_LED_RED_PIN = BC_TCA9534A_PIN_P5,
    _BC_MODULE_LCD_LED_BLUE_PIN = BC_TCA9534A_PIN_P6,
    _BC_MODULE_LCD_LED_DISP_CS_PIN = BC_TCA9534A_PIN_P7
};

#define _BC_MODULE_LCD_INITIALIZED ((1 << _BC_MODULE_LCD_BACKLIGHT_PIN) | (1 << _BC_MODULE_LCD_LED_DISP_CS_PIN) | (1 << _BC_MODULE_LCD_DISP_ON_PIN) | (1 << _BC_MODULE_LCD_LED_GREEN_PIN) | (1 << _BC_MODULE_LCD_LED_RED_PIN) | (1 << _BC_MODULE_LCD_LED_BLUE_PIN))

typedef struct bc_module_lcd_t
{
    void (*event_handler)(bc_module_lcd_event_t, void *);
    void *event_param;
    bool is_tca9534a_initialized;
    bc_tca9534a_t tca9534a;
    bc_ls013b7dh03_t ls013b7dh03;
    bc_gfx_t gfx;

    bc_button_t button_left;
    bc_button_t button_right;

} bc_module_lcd_t;

bc_module_lcd_t _bc_module_lcd;

static bc_tca9534a_pin_t _bc_module_lcd_led_pin_lut[3] =
{
        [BC_MODULE_LCD_LED_RED] = BC_TCA9534A_PIN_P5,
        [BC_MODULE_LCD_LED_GREEN] = BC_TCA9534A_PIN_P4,
        [BC_MODULE_LCD_LED_BLUE] = BC_TCA9534A_PIN_P6
};

static bc_tca9534a_pin_t _bc_module_lcd_button_pin_lut[2] =
{
        [BC_MODULE_LCD_BUTTON_LEFT] = BC_TCA9534A_PIN_P3,
        [BC_MODULE_LCD_BUTTON_RIGHT] = BC_TCA9534A_PIN_P1
};

static bool _bc_module_lcd_tca9534a_init(void);

static bool _bc_module_lcd_cs_pin_set(bool state);

static void _bc_module_lcd_led_init(bc_led_t *self);

static void _bc_module_lcd_led_on(bc_led_t *self);

static void _bc_module_lcd_led_off(bc_led_t *self);

static void _bc_module_lcd_button_init(bc_button_t *self);

static int _bc_module_lcd_button_get_input(bc_button_t *self);

void bc_module_lcd_init()
{
	_bc_module_lcd_tca9534a_init();

	bc_ls013b7dh03_init(&_bc_module_lcd.ls013b7dh03, _bc_module_lcd_cs_pin_set);

	bc_gfx_init(&_bc_module_lcd.gfx, &_bc_module_lcd.ls013b7dh03, bc_ls013b7dh03_get_driver());

	bc_gfx_clear(&_bc_module_lcd.gfx);
}

bc_gfx_t *bc_module_lcd_get_gfx()
{
    return &_bc_module_lcd.gfx;
}

bool bc_module_lcd_on(void)
{
    return bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_DISP_ON_PIN, 1);
}

bool bc_module_lcd_off(void)
{
    return bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_DISP_ON_PIN, 0);
}

bool bc_module_lcd_is_ready(void)
{
	return _bc_module_lcd_tca9534a_init() && bc_gfx_display_is_ready(&_bc_module_lcd.gfx);
}

void bc_module_lcd_clear(void)
{
    bc_gfx_clear(&_bc_module_lcd.gfx);
}

void bc_module_lcd_draw_pixel(int x, int y, bool value)
{
    bc_gfx_draw_pixel(&_bc_module_lcd.gfx, x, y, value);
}

int bc_module_lcd_draw_char(int left, int top, uint8_t ch, bool color)
{
    return bc_gfx_draw_char(&_bc_module_lcd.gfx, left, top, ch, color);
}

int bc_module_lcd_draw_string(int left, int top, char *str, bool color)
{
    return bc_gfx_draw_string(&_bc_module_lcd.gfx, left, top, str, color);
}


void bc_module_lcd_draw(const uint8_t *frame, uint8_t width, uint8_t height) // In pixels
{
    (void)frame;
    (void)width;
    (void)height;
}

void bc_module_lcd_printf(uint8_t line, /*uint8_t size, font, */const uint8_t *string/*, ...*/)
{
    (void) line;
    (void) string;
}

void bc_module_lcd_draw_line(int x0, int y0, int x1, int y1, bool color)
{
    bc_gfx_draw_line(&_bc_module_lcd.gfx, x0, y0, x1, y1, color);
}

void bc_module_lcd_draw_rectangle(int x0, int y0, int x1, int y1, bool color)
{
    bc_gfx_draw_rectangle(&_bc_module_lcd.gfx, x0, y0, x1, y1, color);
}

void bc_module_lcd_draw_circle(int x0, int y0, int radius, bool color)
{
    bc_gfx_draw_circle(&_bc_module_lcd.gfx, x0, y0, radius, color);
}

void bc_module_lcd_draw_image(int left, int top, const bc_image_t *img)
{
    uint8_t line;
	uint8_t row;
    uint8_t bytes_per_row = img->width / 8;

    if(img->width % 8 != 0)
    {
        bytes_per_row++;
    }

    bc_tick_t start = bc_tick_get();

        for (row = 0; row < img->height; row++) {
            for (line = 0; line < img->width; line++) {
                uint32_t byte_offset = line / 8 + row * bytes_per_row;
                uint32_t bit = line % 8;
                bc_gfx_draw_pixel(&_bc_module_lcd.gfx, line + left, row + top, (img->data[byte_offset]) & (1 << bit));
			}
		}
    volatile bc_tick_t duration = bc_tick_get() - start;
    (void)duration;

}

bool bc_module_lcd_update(void)
{
    return bc_gfx_update(&_bc_module_lcd.gfx);
}

void bc_module_lcd_set_font(const bc_font_t *font)
{
    bc_gfx_set_font(&_bc_module_lcd.gfx, font);
}

static void _bc_module_lcd_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) event_param;

    if (self == &_bc_module_lcd.button_left)
    {
        if (event == BC_BUTTON_EVENT_PRESS)
        {
            _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_LEFT_PRESS, _bc_module_lcd.event_param);
        }
        if (event == BC_BUTTON_EVENT_RELEASE)
        {
            _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_LEFT_RELEASE, _bc_module_lcd.event_param);
        }
        if (event == BC_BUTTON_EVENT_CLICK)
        {
            _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_LEFT_CLICK, _bc_module_lcd.event_param);
        }
        if (event == BC_BUTTON_EVENT_HOLD)
        {
            if (_bc_module_lcd.button_right._state)
            {
                _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_BOTH_HOLD, _bc_module_lcd.event_param);
                // Force _hold_signalized to true, so the hold event of the second button won't trigger which would cause event duplication
                _bc_module_lcd.button_right._hold_signalized = true;
            }
            else
            {
                _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_LEFT_HOLD, _bc_module_lcd.event_param);
            }
        }
    }

    if (self == &_bc_module_lcd.button_right)
    {
        if (event == BC_BUTTON_EVENT_PRESS)
        {
            _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_RIGHT_PRESS, _bc_module_lcd.event_param);
        }
        if (event == BC_BUTTON_EVENT_RELEASE)
        {
            _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_RIGHT_RELEASE, _bc_module_lcd.event_param);
        }
        if (event == BC_BUTTON_EVENT_CLICK)
        {
            _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_RIGHT_CLICK, _bc_module_lcd.event_param);
        }
        if (event == BC_BUTTON_EVENT_HOLD)
        {
            if (_bc_module_lcd.button_left._state)
            {
                _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_BOTH_HOLD, _bc_module_lcd.event_param);
                // Force _hold_signalized to true, so the hold event of the second button won't trigger which would cause event duplication
                _bc_module_lcd.button_left._hold_signalized = true;
            }
            else
            {
                _bc_module_lcd.event_handler(BC_MODULE_LCD_EVENT_RIGHT_HOLD, _bc_module_lcd.event_param);
            }
        }
    }
}

void bc_module_lcd_set_event_handler(void (*event_handler)(bc_module_lcd_event_t, void *), void *event_param)
{
    _bc_module_lcd.event_handler = event_handler;
    _bc_module_lcd.event_param = event_param;

    const bc_button_driver_t* lcdButtonDriver =  bc_module_lcd_get_button_driver();

    bc_button_init_virtual(&_bc_module_lcd.button_left, 0, lcdButtonDriver, 0);
    bc_button_init_virtual(&_bc_module_lcd.button_right, 1, lcdButtonDriver, 0);

    bc_button_set_event_handler(&_bc_module_lcd.button_left, _bc_module_lcd_button_event_handler, (int*)0);
    bc_button_set_event_handler(&_bc_module_lcd.button_right, _bc_module_lcd_button_event_handler, (int*)1);
}

void bc_module_lcd_set_button_hold_time(bc_tick_t hold_time)
{
    bc_button_set_hold_time(&_bc_module_lcd.button_left, hold_time);
    bc_button_set_hold_time(&_bc_module_lcd.button_right, hold_time);
}

void bc_module_lcd_set_button_scan_interval(bc_tick_t scan_interval)
{
    bc_button_set_scan_interval(&_bc_module_lcd.button_left, scan_interval);
    bc_button_set_scan_interval(&_bc_module_lcd.button_right, scan_interval);
}

void bc_module_lcd_set_button_debounce_time(bc_tick_t debounce_time)
{
    bc_button_set_debounce_time(&_bc_module_lcd.button_left, debounce_time);
    bc_button_set_debounce_time(&_bc_module_lcd.button_right, debounce_time);
}

void bc_module_lcd_set_button_click_timeout(bc_tick_t click_timeout)
{
    bc_button_set_click_timeout(&_bc_module_lcd.button_left, click_timeout);
    bc_button_set_click_timeout(&_bc_module_lcd.button_right, click_timeout);
}

void bc_module_lcd_set_rotation(bc_module_lcd_rotation_t rotation)
{
    bc_gfx_set_rotation(&_bc_module_lcd.gfx, rotation);
}

bc_module_lcd_rotation_t bc_module_lcd_get_rotation(void)
{
    return bc_gfx_get_rotation(&_bc_module_lcd.gfx);
}

const bc_led_driver_t *bc_module_lcd_get_led_driver(void)
{
    static const bc_led_driver_t bc_module_lcd_led_driver =
    {
        .init = _bc_module_lcd_led_init,
        .on = _bc_module_lcd_led_on,
        .off = _bc_module_lcd_led_off,
    };

    return &bc_module_lcd_led_driver;
}

const bc_button_driver_t *bc_module_lcd_get_button_driver(void)
{
    static const bc_button_driver_t bc_module_lcd_button_driver =
    {
        .init = _bc_module_lcd_button_init,
        .get_input = _bc_module_lcd_button_get_input,
    };

    return &bc_module_lcd_button_driver;
}

static bool _bc_module_lcd_tca9534a_init(void)
{
	if (!_bc_module_lcd.is_tca9534a_initialized)
	{
		if (!bc_tca9534a_init(&_bc_module_lcd.tca9534a, BC_I2C_I2C0, 0x3c))
		{
			return false;
		}

		if (!bc_tca9534a_write_port(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_INITIALIZED))
		{
			return false;
		}

		if (!bc_tca9534a_set_port_direction(&_bc_module_lcd.tca9534a, (1 << _BC_MODULE_LCD_BUTTON1_PIN) | (1 << _BC_MODULE_LCD_BUTTON2_PIN)))
		{
			return false;
		}

		_bc_module_lcd.is_tca9534a_initialized = true;
	}

	return true;
}

static bool _bc_module_lcd_cs_pin_set(bool state)
{
    if (!_bc_module_lcd_tca9534a_init())
    {
        return false;
    }

    if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_LED_DISP_CS_PIN, state))
    {
        _bc_module_lcd.is_tca9534a_initialized = false;

        return false;
    }

    return true;
}

static void _bc_module_lcd_led_init(bc_led_t *self)
{
    (void) self;

    _bc_module_lcd_tca9534a_init();
}

static void _bc_module_lcd_led_on(bc_led_t *self)
{
    if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _bc_module_lcd_led_pin_lut[self->_channel.virtual], self->_idle_state ? 0 : 1))
    {
    	_bc_module_lcd.is_tca9534a_initialized = false;
    }
}

static void _bc_module_lcd_led_off(bc_led_t *self)
{
    if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _bc_module_lcd_led_pin_lut[self->_channel.virtual], self->_idle_state ? 1 : 0))
    {
    	_bc_module_lcd.is_tca9534a_initialized = false;
    }
}

static void _bc_module_lcd_button_init(bc_button_t *self)
{
    (void) self;

    _bc_module_lcd_tca9534a_init();

    bc_gpio_set_mode(BC_GPIO_BUTTON, BC_GPIO_MODE_INPUT);
    bc_gpio_init(BC_GPIO_BUTTON);
}

static int _bc_module_lcd_button_get_input(bc_button_t *self)
{
    if (bc_gpio_get_input(BC_GPIO_BUTTON) == 0)
    {
        return 0;
    }

    int state;

    if (!bc_tca9534a_read_pin(&_bc_module_lcd.tca9534a, _bc_module_lcd_button_pin_lut[self->_channel.virtual], &state))
    {
    	_bc_module_lcd.is_tca9534a_initialized = false;
    	return 0;
    }

    return state;
}
