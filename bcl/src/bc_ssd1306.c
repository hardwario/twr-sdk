#include <bc_ssd1306.h>
#include <math.h>

#define _BC_SSD1306_SETCONTRAST  0x81
#define _BC_SSD1306_DISPLAYALLON_RESUME  0xA4
#define _BC_SSD1306_DISPLAYALLON  0xA5
#define _BC_SSD1306_NORMALDISPLAY  0xA6
#define _BC_SSD1306_INVERTDISPLAY  0xA7
#define _BC_SSD1306_DISPLAYOFF  0xAE
#define _BC_SSD1306_DISPLAYON  0xAF
#define _BC_SSD1306_SETDISPLAYOFFSET  0xD3
#define _BC_SSD1306_SETCOMPINS  0xDA
#define _BC_SSD1306_SETVCOMDETECT  0xDB
#define _BC_SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define _BC_SSD1306_SETPRECHARGE  0xD9
#define _BC_SSD1306_SETMULTIPLEX  0xA8
#define _BC_SSD1306_SETLOWCOLUMN  0x00
#define _BC_SSD1306_SETHIGHCOLUMN  0x10
#define _BC_SSD1306_SETSTARTLINE  0x40
#define _BC_SSD1306_MEMORYMODE  0x20
#define _BC_SSD1306_COLUMNADDR  0x21
#define _BC_SSD1306_PAGEADDR  0x22
#define _BC_SSD1306_COMSCANINC  0xC0
#define _BC_SSD1306_COMSCANDEC  0xC8
#define _BC_SSD1306_SEGREMAP  0xA0
#define _BC_SSD1306_CHARGEPUMP  0x8D
#define _BC_SSD1306_EXTERNALVCC  0x1
#define _BC_SSD1306_SWITCHCAPVCC  0x2

static bool _bc_ssd1306_command(bc_ssd1306_t *self, uint8_t command);
static bool _bc_ssd1306_send_data(bc_ssd1306_t *self, uint8_t *buffer, size_t length);
static bool _bc_ssd1306_init(bc_ssd1306_t *self);

bool bc_ssd1306_init(bc_ssd1306_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address, const bc_ssd1306_framebuffer_t *framebuffer)
{
    memset(self, 0xff, sizeof(*self));

    self->_i2c_channel = i2c_channel;

    self->_i2c_address = i2c_address;

    self->_framebuffer = framebuffer;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    self->_initialized = _bc_ssd1306_init(self);

    return self->_initialized;
}

bc_gfx_caps_t bc_ssd1306_get_caps(bc_ssd1306_t *self)
{
    bc_gfx_caps_t caps = { .width = self->_framebuffer->width, .height = self->_framebuffer->height };

    return caps;
}

bool bc_ssd1306_is_ready(bc_ssd1306_t *self)
{
    if (!self->_initialized)
    {
        self->_initialized = _bc_ssd1306_init(self);
    }

    return self->_initialized;
}

void bc_ssd1306_clear(bc_ssd1306_t *self)
{
    memset(self->_framebuffer->buffer, 0x00, self->_framebuffer->length);
}

void bc_ssd1306_draw_pixel(bc_ssd1306_t *self, int x, int y, uint32_t color)
{
    // Skip mode byte + addr byte
    uint32_t byteIndex = 0;
    // Skip lines
    byteIndex += x;
    // Select column byte
    byteIndex += (y / 8) * self->_framebuffer->width;

    uint8_t bitMask = 1 << (y % 8);

    if (color == 0)
    {
        self->_framebuffer->buffer[byteIndex] &= ~bitMask;
    }
    else
    {
        self->_framebuffer->buffer[byteIndex] |= bitMask;
    }
}

uint32_t bc_ssd1306_get_pixel(bc_ssd1306_t *self, int x, int y)
{
    (void) self;
    (void) x;
    (void) y;

    return 0;
}

bool bc_ssd1306_update(bc_ssd1306_t *self)
{
    if (!self->_initialized)
    {
        self->_initialized = _bc_ssd1306_init(self);
    }

    if (!self->_initialized)
    {
        return false;
    }

    if (_bc_ssd1306_command(self, _BC_SSD1306_COLUMNADDR) &&
            _bc_ssd1306_command(self, 0) &&        // Column start address. (0 = reset)
            _bc_ssd1306_command(self, self->_framebuffer->width - 1) &&    // Column end address.
            _bc_ssd1306_command(self, _BC_SSD1306_PAGEADDR) &&
            _bc_ssd1306_command(self, 0) &&        // Page start address. (0 = reset)
            _bc_ssd1306_command(self, self->_framebuffer->pages - 1)) // Page end address.
    {
        for (size_t i = 0; i < self->_framebuffer->length; i += 8)
        {
            if (!_bc_ssd1306_send_data(self, self->_framebuffer->buffer + i, 8))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

const bc_gfx_driver_t *bc_ssd1306_get_driver(void)
{
    static const bc_gfx_driver_t driver =
    {
        .is_ready = (bool (*)(void *)) bc_ssd1306_is_ready,
        .clear = (void (*)(void *)) bc_ssd1306_clear,
        .draw_pixel = (void (*)(void *, int, int, uint32_t)) bc_ssd1306_draw_pixel,
        .get_pixel = (uint32_t (*)(void *, int, int)) bc_ssd1306_get_pixel,
        .update = (bool (*)(void *)) bc_ssd1306_update,
        .get_caps = (bc_gfx_caps_t (*)(void *)) bc_ssd1306_get_caps
    };

    return &driver;
}

static bool _bc_ssd1306_command(bc_ssd1306_t *self, uint8_t command)
{
    return bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x00, command);
}

static bool _bc_ssd1306_send_data(bc_ssd1306_t *self, uint8_t *buffer, size_t length)
{
    bc_i2c_memory_transfer_t transfer;

    transfer.device_address = self->_i2c_address;
    transfer.memory_address = 0x40;
    transfer.buffer = buffer;
    transfer.length = length;

    return bc_i2c_memory_write(self->_i2c_channel, &transfer);
}

static bool _bc_ssd1306_init(bc_ssd1306_t *self)
{
    return _bc_ssd1306_command(self, _BC_SSD1306_DISPLAYOFF) &&
            _bc_ssd1306_command(self, _BC_SSD1306_SETDISPLAYCLOCKDIV) &&
            _bc_ssd1306_command(self, 0x80) && // the suggested ratio
            _bc_ssd1306_command(self, _BC_SSD1306_SETMULTIPLEX) &&
            _bc_ssd1306_command(self, self->_framebuffer->height - 1) &&
            _bc_ssd1306_command(self, _BC_SSD1306_SETDISPLAYOFFSET) &&
            _bc_ssd1306_command(self, 0x0) && // no offset
            _bc_ssd1306_command(self, _BC_SSD1306_SETSTARTLINE | 0x0) && // line #0
            _bc_ssd1306_command(self, _BC_SSD1306_CHARGEPUMP) &&
            _bc_ssd1306_command(self, 0x14) &&
            _bc_ssd1306_command(self, _BC_SSD1306_MEMORYMODE) &&
            _bc_ssd1306_command(self, 0x00) && // Horizontal addressing mode
            _bc_ssd1306_command(self, _BC_SSD1306_SEGREMAP | 0x1) &&
            _bc_ssd1306_command(self, _BC_SSD1306_COMSCANDEC) &&
            _bc_ssd1306_command(self, _BC_SSD1306_SETCOMPINS) &&
            _bc_ssd1306_command(self, self->_framebuffer->height == 64 ? 0x12 : 0x02) &&
            _bc_ssd1306_command(self, _BC_SSD1306_SETCONTRAST) &&
            _bc_ssd1306_command(self, 0x8f) &&
            _bc_ssd1306_command(self, _BC_SSD1306_SETPRECHARGE) &&
            _bc_ssd1306_command(self, 0xF1) &&
            _bc_ssd1306_command(self, _BC_SSD1306_SETVCOMDETECT) &&
            _bc_ssd1306_command(self, 0x40) &&
            _bc_ssd1306_command(self, _BC_SSD1306_DISPLAYALLON_RESUME) &&
            _bc_ssd1306_command(self, _BC_SSD1306_NORMALDISPLAY) &&
            _bc_ssd1306_command(self, _BC_SSD1306_DISPLAYON); // Turn on the display.
}
