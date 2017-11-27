// http://www.mouser.com/ds/2/365/LS013B7DH03%20SPEC_SMA-224806.pdf
// https://www.embeddedartists.com/sites/default/files/support/datasheet/Memory_LCD_Programming.pdf
// https://www.silabs.com/documents/public/application-notes/AN0048.pdf

#include <stm32l0xx.h>
#include <bc_module_lcd.h>
#include <bc_spi.h>
#include <bc_tca9534a.h>
#include <bc_scheduler.h>

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

#define _BC_MODULE_LCD_VCOM_PERIOD 15000
#define _BC_MODULE_LCD_INITIALIZED ((1 << _BC_MODULE_LCD_LED_DISP_CS_PIN) | (1 << _BC_MODULE_LCD_DISP_ON_PIN) | (1 << _BC_MODULE_LCD_LED_GREEN_PIN) | (1 << _BC_MODULE_LCD_LED_RED_PIN) | (1 << _BC_MODULE_LCD_LED_BLUE_PIN))

typedef struct bc_module_lcd_t
{
    void (*event_handler)(bc_module_lcd_event_t, void *);
    void *event_param;
    bool is_tca9534a_initialized;
    bc_tca9534a_t tca9534a;
    uint8_t *framebuffer;
    const bc_font_t *font;
    bc_module_lcd_rotation_t rotation;
    uint8_t vcom;
    bc_scheduler_task_id_t task_id;

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

static bool _bc_module_lcd_spi_transfer(uint8_t *buffer, size_t length);

static void _bc_module_lcd_task(void *param);

static void _bc_spi_event_handler(bc_spi_event_t event, void *event_param);

static inline uint8_t _bc_module_lcd_reverse(uint8_t b);

static void _bc_module_lcd_led_init(bc_led_t *self);

static void _bc_module_lcd_led_on(bc_led_t *self);

static void _bc_module_lcd_led_off(bc_led_t *self);

static void _bc_module_lcd_button_init(bc_button_t *self);

static int _bc_module_lcd_button_get_input(bc_button_t *self);

void bc_module_lcd_init(bc_module_lcd_framebuffer_t *framebuffer)
{
	_bc_module_lcd_tca9534a_init();

    bc_spi_init(BC_SPI_SPEED_1_MHZ, BC_SPI_MODE_0);

    _bc_module_lcd.framebuffer = framebuffer->framebuffer;

    // Address lines
    uint8_t line;
    uint32_t offs;
    for (line = 0x01, offs = 1; line <= 128; line++, offs += 18)
    {
        // Fill the gate line addresses on the exact place in the buffer
        _bc_module_lcd.framebuffer[offs] = _bc_module_lcd_reverse(line);
    }

    // Prepare buffer so the background is "white" reflective
    bc_module_lcd_clear();

    _bc_module_lcd.task_id = bc_scheduler_register(_bc_module_lcd_task, NULL, _BC_MODULE_LCD_VCOM_PERIOD);
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
	return _bc_module_lcd_tca9534a_init() && bc_spi_is_ready();
}

void bc_module_lcd_clear(void)
{
	uint8_t line;
	uint32_t offs;
	uint8_t col;
	for (line = 0x01, offs = 2; line <= 128; line++, offs += 18)
	{
		for (col = 0; col < 16; col++)
		{
			_bc_module_lcd.framebuffer[offs + col] = 0xff;
		}
	}
}

void bc_module_lcd_draw_pixel(int x, int y, bool value)
{
    if (x > 127 || y > 127 || x < 0 || y < 0)
    {
        return;
    }

    int tmp;

    switch (_bc_module_lcd.rotation)
    {
        case BC_MODULE_LCD_ROTATION_90:
        {
            tmp = x;
            x = 127 - y;
            y = tmp;
            break;
        }
        case BC_MODULE_LCD_ROTATION_180:
        {
            x = 127 - x;
            y = 127 - y;
            break;
        }
        case BC_MODULE_LCD_ROTATION_270:
        {
            tmp = y;
            y = 127 - x;
            x = tmp;
            break;
        }
        case BC_MODULE_LCD_ROTATION_0:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    // Skip mode byte + addr byte
    uint32_t byteIndex = 2;
    // Skip lines
    byteIndex += y * 18;
    // Select column byte
    byteIndex += x / 8;

    uint8_t bitMask = 1 << (7 - (x % 8));

    if (!value)
    {
        _bc_module_lcd.framebuffer[byteIndex] |= bitMask;
    }
    else
    {
        _bc_module_lcd.framebuffer[byteIndex] &= ~bitMask;
    }
}

int bc_module_lcd_draw_char(int left, int top, uint8_t ch, bool color)
{
    const bc_font_t *font = _bc_module_lcd.font;

    int w = 0;
    uint8_t h = 0;
    uint16_t i;
    uint16_t x;
    uint16_t y;
    uint8_t bytes;

    for (i = 0; i < font->length; i++)
    {
        if (font->chars[i].code == ch)
        {
            w = font->chars[i].image->width;
            h = font->chars[i].image->heigth;

            bytes = (w + 7) / 8;

            for (y = 0; y < h; y++)
            {
                for (x = 0; x < w; x++)
                {
                    uint32_t byteIndex = x / 8;
                    byteIndex += y * bytes;

                    uint8_t bitMask = 1 << (7 - (x % 8));

                    if (font->chars[i].image->image[byteIndex] & bitMask)
                    {
                        bc_module_lcd_draw_pixel(left + x, top + y, !color);
                    }
                    else
                    {
                        bc_module_lcd_draw_pixel(left + x, top + y, color);
                    }
                }
            }
        }
    }

    return w;
}

int bc_module_lcd_draw_string(int left, int top, char *str, bool color)
{
    while(*str)
    {
        left += bc_module_lcd_draw_char(left, top, *str, color);
        str++;
    }
    return left;
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
    int16_t step = abs(y1 - y0) > abs(x1 - x0);
    int16_t tmp;

    if (step)
    {
        tmp = x0;
        x0 = y0;
        y0 = tmp;

        tmp = x1;
        x1 = y1;
        y1 = tmp;
    }

    if (x0 > x1)
    {
        tmp = x0;
        x0 = x1;
        x1 = tmp;

        tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    ystep = y0 < y1 ? 1 : -1;

    for (; x0 <= x1; x0++)
    {
        if (step)
        {
            bc_module_lcd_draw_pixel(y0, x0, color);
        }
        else
        {
            bc_module_lcd_draw_pixel(x0, y0, color);
        }

        err -= dy;

        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void bc_module_lcd_draw_rectangle(int x0, int y0, int x1, int y1, bool color)
{
    bc_module_lcd_draw_line(x0, y0, x0, y1, color);
    bc_module_lcd_draw_line(x0, y1, x1, y1, color);
    bc_module_lcd_draw_line(x1, y0, x1, y1, color);
    bc_module_lcd_draw_line(x1, y0, x0, y0, color);
}

// Using Midpoint circle algorithm
void bc_module_lcd_draw_circle(int x0, int y0, int radius, bool color)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        bc_module_lcd_draw_pixel(x0 + x, y0 + y, color);
        bc_module_lcd_draw_pixel(x0 + y, y0 + x, color);
        bc_module_lcd_draw_pixel(x0 - y, y0 + x, color);
        bc_module_lcd_draw_pixel(x0 - x, y0 + y, color);
        bc_module_lcd_draw_pixel(x0 - x, y0 - y, color);
        bc_module_lcd_draw_pixel(x0 - y, y0 - x, color);
        bc_module_lcd_draw_pixel(x0 + y, y0 - x, color);
        bc_module_lcd_draw_pixel(x0 + x, y0 - y, color);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }
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
                bc_module_lcd_draw_pixel(line + left, row + top, (img->data[byte_offset]) & (1 << bit));
			}
		}
    volatile bc_tick_t duration = bc_tick_get() - start;
    (void)duration;

}

/*

Framebuffer format for updating multiple lines, ideal for later DMA TX:

||    Set MODE      ||------18B for line---||--next 18B 2nd line--| ...
||        1B        ||   1B |  16B |  1B   ||   1B |  16B |  1B   |
||  M0 M1 M2  DUMMY || ADDR | DATA | DUMMY || ADDR | DATA | DUMMY |

*/
bool bc_module_lcd_update(void)
{
    if (bc_spi_is_ready())
    {
        if (!_bc_module_lcd_tca9534a_init())
        {
            return false;
        }

        if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_LED_DISP_CS_PIN, 0))
        {
            _bc_module_lcd.is_tca9534a_initialized = false;
            return false;
        }

        _bc_module_lcd.framebuffer[0] = 0x80 | _bc_module_lcd.vcom;

        if (!bc_spi_async_transfer(_bc_module_lcd.framebuffer, NULL, BC_LCD_FRAMEBUFFER_SIZE, _bc_spi_event_handler, NULL))
        {
            if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_LED_DISP_CS_PIN, 1))
            {
                _bc_module_lcd.is_tca9534a_initialized = false;
            }
            return false;
        }

        bc_scheduler_plan_relative(_bc_module_lcd.task_id, _BC_MODULE_LCD_VCOM_PERIOD);

        _bc_module_lcd.vcom ^= 0x40;

        return true;
    }

    return false;
}

bool bc_module_lcd_clear_memory_command(void)
{
    uint8_t spi_data[2] = { 0x20, 0x00 };

    return _bc_module_lcd_spi_transfer(spi_data, sizeof(spi_data));
}

void bc_module_lcd_set_font(const bc_font_t *font)
{
    _bc_module_lcd.font = font;
}

void bc_module_lcd_set_rotation(bc_module_lcd_rotation_t rotation)
{
    _bc_module_lcd.rotation = rotation;
}

bc_module_lcd_rotation_t bc_module_lcd_get_rotation(void)
{
    return _bc_module_lcd.rotation;
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

static inline uint8_t _bc_module_lcd_reverse(uint8_t b)
{
   b = (b & 0xf0) >> 4 | (b & 0x0f) << 4;
   b = (b & 0xcc) >> 2 | (b & 0x33) << 2;
   b = (b & 0xaa) >> 1 | (b & 0x55) << 1;

   return b;
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

		if (!bc_tca9534a_set_port_direction(&_bc_module_lcd.tca9534a, 0x0a))
		{
			return false;
		}

		_bc_module_lcd.is_tca9534a_initialized = true;
	}

	return true;
}

static bool _bc_module_lcd_spi_transfer(uint8_t *buffer, size_t length)
{
    if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_LED_DISP_CS_PIN, 0))
    {
    	_bc_module_lcd.is_tca9534a_initialized = false;
    	return false;
    }

    bool spi_state = bc_spi_transfer(buffer, NULL, length);

    if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_LED_DISP_CS_PIN, 1))
    {
    	_bc_module_lcd.is_tca9534a_initialized = false;
    	return false;
    }

    return spi_state;
}

static void _bc_module_lcd_task(void *param)
{
    (void) param;

    uint8_t spi_data[2] = {_bc_module_lcd.vcom, 0x00};

    if (_bc_module_lcd_spi_transfer(spi_data, sizeof(spi_data)))
    {
    	_bc_module_lcd.vcom ^= 0x40;
    }

    bc_scheduler_plan_current_from_now(_BC_MODULE_LCD_VCOM_PERIOD);
}

static void _bc_spi_event_handler(bc_spi_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_SPI_EVENT_DONE)
    {
        bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _BC_MODULE_LCD_LED_DISP_CS_PIN, 1);
    }
}

static void _bc_module_lcd_led_init(bc_led_t *self)
{
    (void) self;

    _bc_module_lcd_tca9534a_init();
}

static void _bc_module_lcd_led_on(bc_led_t *self)
{
    if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _bc_module_lcd_led_pin_lut[self->_channel.virtual_channel], self->_idle_state ? 0 : 1))
    {
    	_bc_module_lcd.is_tca9534a_initialized = false;
    }
}

static void _bc_module_lcd_led_off(bc_led_t *self)
{
    if (!bc_tca9534a_write_pin(&_bc_module_lcd.tca9534a, _bc_module_lcd_led_pin_lut[self->_channel.virtual_channel], self->_idle_state ? 1 : 0))
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

    if (!bc_tca9534a_read_pin(&_bc_module_lcd.tca9534a, _bc_module_lcd_button_pin_lut[self->_channel.virtual_channel], &state))
    {
    	_bc_module_lcd.is_tca9534a_initialized = false;
    	return 0;
    }

    return state;
}
