#include <hio_ds2484.h>
#include <hio_i2c.h>
#include <hio_gpio.h>
#include <hio_log.h>
#include <hio_delay.h>

#define _HIO_DS2484_I2C_ADDRESS 0x18

// command set
#define _HIO_DS2484_CMD_DRST (0xF0) // Device Reset
#define _HIO_DS2484_CMD_SRP  (0xE1) // Set Read Pointer
#define _HIO_DS2484_CMD_WCFG (0xD2) // Write Configuration
#define _HIO_DS2484_CMD_CHSL (0xC3) // Channel Select(HIO_DS2484-800 only)
#define _HIO_DS2484_CMD_1WRS (0xB4) // 1-Wire Reset
#define _HIO_DS2484_CMD_1WSB (0x87) // 1-Wire Single Bit
#define _HIO_DS2484_CMD_1WWB (0xA5) // 1-Wire Write Byte
#define _HIO_DS2484_CMD_1WRB (0x96) // 1-Wire Read Byte
#define _HIO_DS2484_CMD_1WT  (0x78) // 1-Wire Wire Triplet

// register pointers
#define _HIO_DS2484_REG_ST   (0xF0) // Status Register
#define _HIO_DS2484_REG_DATA (0xE1) // Data Register
#define _HIO_DS2484_REG_CFG  (0xC3) // Configuration Register

// status register HIO_DS2484_REG_STATUS_* in *.h file

// configuration register _HIO_DS2484_REG_CFG
#define _HIO_DS2484_CFG_APU (1<<0) // Active Pullup(1==yes)
#define _HIO_DS2484_CFG_SPU (1<<2) // Strong Pullup(1==yes)
#define _HIO_DS2484_CFG_1WS (1<<3) // 1-Wire Speed(0==standard, 1==overdrive)

#define _HIO_DS2484_CFG_DEFAULT (_HIO_DS2484_CFG_APU)

static bool _hio_ds2484_i2c_read_byte(hio_ds2484_t *self, uint8_t *b);
static bool _hio_ds2484_i2c_write_byte(hio_ds2484_t *self, const uint8_t b);
static bool _hio_ds2484_device_reset(hio_ds2484_t *self);
static bool _hio_ds2484_write_config(hio_ds2484_t *self, const uint8_t cfg);
static bool _hio_ds2484_set_reed_pointer(hio_ds2484_t *self, uint8_t srp);

bool hio_ds2484_init(hio_ds2484_t *self, hio_i2c_channel_t i2c_channel)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;

    self->_ready = false;

    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);

    return true;
}

void hio_ds2484_set_slpz_handler(hio_ds2484_t *self, bool (*handler)(void *, bool), void *handler_ctx)
{
    self->_set_slpz = handler;
    self->_set_slpz_ctx = handler_ctx;
}

void hio_ds2484_enable(hio_ds2484_t *self)
{
    self->_ready = false;

    if (self->_set_slpz != NULL)
    {
        if (!self->_set_slpz(self->_set_slpz_ctx, 1))
        {
            return;
        }
    }

    if (!_hio_ds2484_device_reset(self))
    {
        hio_log_error("HIO_DS2484: not detected");

        return;
    }

    if (!_hio_ds2484_write_config(self, _HIO_DS2484_CFG_DEFAULT))
    {
        hio_log_error("HIO_DS2484: set config");

        return;
    }

    self->_ready = true;

    return;
}

void hio_ds2484_disable(hio_ds2484_t *self)
{
    hio_ds2484_busy_wait(self);

    if (self->_set_slpz != NULL)
    {
        self->_set_slpz(self->_set_slpz_ctx, 0);
    }

    self->_ready = false;
}

bool hio_ds2484_reset(hio_ds2484_t *self)
{
    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_hio_ds2484_i2c_write_byte(self, _HIO_DS2484_CMD_1WRS))
    {
        return false;
    }

    self->_srp = _HIO_DS2484_REG_ST;

    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    // self->_search_ready = true;
    return (self->_status & HIO_DS2484_STATUS_PPD) != 0;
}

bool hio_ds2484_busy_wait(hio_ds2484_t *self)
{
    if (!self->_ready)
    {
        return false;
    }

    int limit = 100;

    if (!_hio_ds2484_set_reed_pointer(self, _HIO_DS2484_REG_ST))
    {
        return false;
    }

    while(limit--)
    {
        if (!_hio_ds2484_i2c_read_byte(self, &self->_status))
        {
            return false;
        }

        if ((self->_status & HIO_DS2484_STATUS_1WB) == 0)
        {
            return true;
        }

        hio_delay_us(1000);
    }

    return false;
}

bool hio_ds2484_write_byte(hio_ds2484_t *self, const uint8_t byte)
{
    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, _HIO_DS2484_I2C_ADDRESS, _HIO_DS2484_CMD_1WWB, byte))
    {
        return false;
    }

    self->_srp = _HIO_DS2484_REG_ST;

    return true;
}

bool hio_ds2484_read_byte(hio_ds2484_t *self, uint8_t *byte)
{
    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_hio_ds2484_i2c_write_byte(self, _HIO_DS2484_CMD_1WRB))
    {
        return false;
    }

    self->_srp = _HIO_DS2484_REG_ST;

    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_hio_ds2484_set_reed_pointer(self, _HIO_DS2484_REG_DATA))
    {
        return false;
    }

    if (!_hio_ds2484_i2c_read_byte(self, byte))
    {
        return false;
    }

    return true;
}

bool hio_ds2484_read_bit(hio_ds2484_t *self, uint8_t *bit)
{
    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_hio_ds2484_i2c_write_byte(self, _HIO_DS2484_CMD_1WSB))
    {
        return false;
    }

    self->_srp = _HIO_DS2484_REG_ST;

    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_hio_ds2484_i2c_read_byte(self, bit))
    {
        return false;
    }

    *bit = *bit & 0x80 ? 1 : 0;

    return true;
}

bool hio_ds2484_triplet(hio_ds2484_t *self, const uint8_t direction)
{
    if (!hio_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, _HIO_DS2484_I2C_ADDRESS, _HIO_DS2484_CMD_1WT, direction ? 0x80 : 0))
    {
        return false;
    }

    self->_srp = _HIO_DS2484_REG_ST;

    return true;
}

bool hio_ds2484_is_ready(hio_ds2484_t *self)
{
    return self->_ready;
}

uint8_t hio_ds2484_status_get(hio_ds2484_t *self)
{
    return self->_status;
}

bool hio_ds2484_is_present(hio_ds2484_t *self)
{
    return _hio_ds2484_device_reset(self);
}

static bool _hio_ds2484_i2c_write_byte(hio_ds2484_t *self, const uint8_t b)
{
    uint8_t buf[1];
    hio_i2c_transfer_t transfer;

    transfer.device_address = _HIO_DS2484_I2C_ADDRESS;
    buf[0] = b;
    transfer.buffer = buf;
    transfer.length = 1;

    return hio_i2c_write(self->_i2c_channel, &transfer);
}

static bool _hio_ds2484_i2c_read_byte(hio_ds2484_t *self, uint8_t *b)
{
    hio_i2c_transfer_t transfer;

    transfer.device_address = _HIO_DS2484_I2C_ADDRESS;
    transfer.buffer = b;
    transfer.length = 1;

    return hio_i2c_read(self->_i2c_channel, &transfer);
}

static bool _hio_ds2484_device_reset(hio_ds2484_t *self)
{
    if (!_hio_ds2484_i2c_write_byte(self, _HIO_DS2484_CMD_DRST))
    {
        return false;
    }

    self->_srp = _HIO_DS2484_REG_ST;

    hio_delay_us(1000);

    return true;
}

static bool _hio_ds2484_write_config(hio_ds2484_t *self, const uint8_t cfg)
{
    uint8_t res;

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, _HIO_DS2484_I2C_ADDRESS, _HIO_DS2484_CMD_WCFG, ((~cfg << 4) | cfg)))
    {
        return false;
    }

    self->_srp = _HIO_DS2484_REG_CFG;

    if (!_hio_ds2484_i2c_read_byte(self, &res))
    {
        return false;
    }

    if (res != cfg)
    {
        return false;
    }

    self->_spu_on = (cfg & _HIO_DS2484_CFG_SPU) ? true : false;

    return true;
}

static bool _hio_ds2484_set_reed_pointer(hio_ds2484_t *self, uint8_t srp)
{
    if (self->_srp != srp)
    {
        if (!hio_i2c_memory_write_8b(self->_i2c_channel, _HIO_DS2484_I2C_ADDRESS, _HIO_DS2484_CMD_SRP, srp))
        {
            return false;
        }
        self->_srp = srp;
    }
    return true;
}
