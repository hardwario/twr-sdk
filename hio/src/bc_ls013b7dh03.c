#include <bc_ls013b7dh03.h>
#include <bc_spi.h>

#define _BC_LS013B7DH03_VCOM_PERIOD 15000

static void _bc_ls013b7dh03_task(void *param);
static bool _bc_ls013b7dh03_spi_transfer(bc_ls013b7dh03_t *self, uint8_t *buffer, size_t length);
static void _bc_ls013b7dh03_spi_event_handler(bc_spi_event_t event, void *event_param);
static inline uint8_t _bc_ls013b7dh03_reverse(uint8_t b);

void bc_ls013b7dh03_init(bc_ls013b7dh03_t *self, bool (*pin_cs_set)(bool state))
{
    memset(self, 0xff, sizeof(*self));

    self->_vcom = 0;
    self->_pin_cs_set = pin_cs_set;

    bc_spi_init(BC_SPI_SPEED_1_MHZ, BC_SPI_MODE_0);

    // Address lines
    uint8_t line;
    uint32_t offs;
    for (line = 0x01, offs = 1; line <= 128; line++, offs += 18)
    {
        // Fill the gate line addresses on the exact place in the buffer
        self->_framebuffer[offs] = _bc_ls013b7dh03_reverse(line);
    }

    self->_pin_cs_set(1);

    self->_task_id = bc_scheduler_register(_bc_ls013b7dh03_task, self, _BC_LS013B7DH03_VCOM_PERIOD);
}

bc_gfx_caps_t bc_ls013b7dh03_get_caps(bc_ls013b7dh03_t *self)
{
    (void) self;

    static const bc_gfx_caps_t caps = { .width = BC_LS013B7DH03_WIDTH, .height = BC_LS013B7DH03_HEIGHT };

    return caps;
}

bool bc_ls013b7dh03_is_ready(bc_ls013b7dh03_t *self)
{
    (void) self;

    return bc_spi_is_ready();
}

void bc_ls013b7dh03_clear(bc_ls013b7dh03_t *self)
{
    uint8_t line;
    uint32_t offs;
    uint8_t col;
    for (line = 0x01, offs = 2; line <= 128; line++, offs += 18)
    {
        for (col = 0; col < 16; col++)
        {
            self->_framebuffer[offs + col] = 0xff;
        }
    }
}

void bc_ls013b7dh03_draw_pixel(bc_ls013b7dh03_t *self, int x, int y, uint32_t color)
{
    // Skip mode byte + addr byte
    uint32_t byteIndex = 2;
    // Skip lines
    byteIndex += y * 18;
    // Select column byte
    byteIndex += x / 8;

    uint8_t bitMask = 1 << (7 - (x % 8));

    if (color == 0)
    {
        self->_framebuffer[byteIndex] |= bitMask;
    }
    else
    {
        self->_framebuffer[byteIndex] &= ~bitMask;
    }
}

uint32_t bc_ls013b7dh03_get_pixel(bc_ls013b7dh03_t *self, int x, int y)
{
    // Skip mode byte + addr byte
    uint32_t byteIndex = 2;
    // Skip lines
    byteIndex += y * 18;
    // Select column byte
    byteIndex += x / 8;

    return (self->_framebuffer[byteIndex] >> (7 - (x % 8))) & 1 ? 0 : 1;
}

/*

Framebuffer format for updating multiple lines, ideal for later DMA TX:

||    Set MODE      ||------18B for line---||--next 18B 2nd line--| ...
||        1B        ||   1B |  16B |  1B   ||   1B |  16B |  1B   |
||  M0 M1 M2  DUMMY || ADDR | DATA | DUMMY || ADDR | DATA | DUMMY |

*/
bool bc_ls013b7dh03_update(bc_ls013b7dh03_t *self)
{
    if (bc_spi_is_ready())
    {
        if (!self->_pin_cs_set(0))
        {
            return false;
        }

        self->_framebuffer[0] = 0x80 | self->_vcom;

        if (!bc_spi_async_transfer(self->_framebuffer, NULL, BC_LS013B7DH03_FRAMEBUFFER_SIZE, _bc_ls013b7dh03_spi_event_handler, self))
        {
            self->_pin_cs_set(1);

            return false;
        }

        bc_scheduler_plan_relative(self->_task_id, _BC_LS013B7DH03_VCOM_PERIOD);

        self->_vcom ^= 0x40;

        return true;
    }

    return false;
}

const bc_gfx_driver_t *bc_ls013b7dh03_get_driver(void)
{
    static const bc_gfx_driver_t driver =
    {
        .is_ready = (bool (*)(void *)) bc_ls013b7dh03_is_ready,
        .clear = (void (*)(void *)) bc_ls013b7dh03_clear,
        .draw_pixel = (void (*)(void *, int, int, uint32_t)) bc_ls013b7dh03_draw_pixel,
        .get_pixel = (uint32_t (*)(void *, int, int)) bc_ls013b7dh03_get_pixel,
        .update = (bool (*)(void *)) bc_ls013b7dh03_update,
        .get_caps = (bc_gfx_caps_t (*)(void *)) bc_ls013b7dh03_get_caps
    };

    return &driver;
}

bool bc_ls013b7dh03_clear_memory_command(bc_ls013b7dh03_t *self)
{
    uint8_t spi_data[2] = { 0x20, 0x00 };

    return _bc_ls013b7dh03_spi_transfer(self, spi_data, sizeof(spi_data));
}

static void _bc_ls013b7dh03_task(void *param)
{
    bc_ls013b7dh03_t *self = (bc_ls013b7dh03_t *) param;

    uint8_t spi_data[2] = {self->_vcom, 0x00};

    if (_bc_ls013b7dh03_spi_transfer(self, spi_data, sizeof(spi_data)))
    {
        self->_vcom ^= 0x40;
    }

    bc_scheduler_plan_current_from_now(_BC_LS013B7DH03_VCOM_PERIOD);
}

static bool _bc_ls013b7dh03_spi_transfer(bc_ls013b7dh03_t *self, uint8_t *buffer, size_t length)
{
    if (!bc_spi_is_ready())
    {
        return false;
    }

    if (!self->_pin_cs_set(0))
    {
        return false;
    }

    bool spi_state = bc_spi_transfer(buffer, NULL, length);

    self->_pin_cs_set(1);

    return spi_state;
}

static void _bc_ls013b7dh03_spi_event_handler(bc_spi_event_t event, void *event_param)
{
    bc_ls013b7dh03_t *self = (bc_ls013b7dh03_t *) event_param;

    if (event == BC_SPI_EVENT_DONE)
    {
        self->_pin_cs_set(1);
    }
}

static inline uint8_t _bc_ls013b7dh03_reverse(uint8_t b)
{
   b = (b & 0xf0) >> 4 | (b & 0x0f) << 4;
   b = (b & 0xcc) >> 2 | (b & 0x33) << 2;
   b = (b & 0xaa) >> 1 | (b & 0x55) << 1;

   return b;
}
