#include <twr_ds2484.h>
#include <twr_i2c.h>
#include <twr_gpio.h>
#include <twr_log.h>
#include <twr_delay.h>

#define _TWR_DS2484_I2C_ADDRESS 0x18

// command set
#define _TWR_DS2484_CMD_DRST (0xF0) // Device Reset
#define _TWR_DS2484_CMD_SRP  (0xE1) // Set Read Pointer
#define _TWR_DS2484_CMD_WCFG (0xD2) // Write Configuration
#define _TWR_DS2484_CMD_CHSL (0xC3) // Channel Select(TWR_DS2484-800 only)
#define _TWR_DS2484_CMD_1WRS (0xB4) // 1-Wire Reset
#define _TWR_DS2484_CMD_1WSB (0x87) // 1-Wire Single Bit
#define _TWR_DS2484_CMD_1WWB (0xA5) // 1-Wire Write Byte
#define _TWR_DS2484_CMD_1WRB (0x96) // 1-Wire Read Byte
#define _TWR_DS2484_CMD_1WT  (0x78) // 1-Wire Wire Triplet

// register pointers
#define _TWR_DS2484_REG_ST   (0xF0) // Status Register
#define _TWR_DS2484_REG_DATA (0xE1) // Data Register
#define _TWR_DS2484_REG_CFG  (0xC3) // Configuration Register

// status register TWR_DS2484_REG_STATUS_* in *.h file

// configuration register _TWR_DS2484_REG_CFG
#define _TWR_DS2484_CFG_APU (1<<0) // Active Pullup(1==yes)
#define _TWR_DS2484_CFG_SPU (1<<2) // Strong Pullup(1==yes)
#define _TWR_DS2484_CFG_1WS (1<<3) // 1-Wire Speed(0==standard, 1==overdrive)

#define _TWR_DS2484_CFG_DEFAULT (_TWR_DS2484_CFG_APU)

static bool _twr_ds2484_i2c_read_byte(twr_ds2484_t *self, uint8_t *b);
static bool _twr_ds2484_i2c_write_byte(twr_ds2484_t *self, const uint8_t b);
static bool _twr_ds2484_device_reset(twr_ds2484_t *self);
static bool _twr_ds2484_write_config(twr_ds2484_t *self, const uint8_t cfg);
static bool _twr_ds2484_set_reed_pointer(twr_ds2484_t *self, uint8_t srp);

bool twr_ds2484_init(twr_ds2484_t *self, twr_i2c_channel_t i2c_channel)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;

    self->_ready = false;

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);

    return true;
}

void twr_ds2484_set_slpz_handler(twr_ds2484_t *self, bool (*handler)(void *, bool), void *handler_ctx)
{
    self->_set_slpz = handler;
    self->_set_slpz_ctx = handler_ctx;
}

void twr_ds2484_enable(twr_ds2484_t *self)
{
    self->_ready = false;

    if (self->_set_slpz != NULL)
    {
        if (!self->_set_slpz(self->_set_slpz_ctx, 1))
        {
            return;
        }
    }

    if (!_twr_ds2484_device_reset(self))
    {
        twr_log_error("TWR_DS2484: not detected");

        return;
    }

    if (!_twr_ds2484_write_config(self, _TWR_DS2484_CFG_DEFAULT))
    {
        twr_log_error("TWR_DS2484: set config");

        return;
    }

    self->_ready = true;

    return;
}

void twr_ds2484_disable(twr_ds2484_t *self)
{
    twr_ds2484_busy_wait(self);

    if (self->_set_slpz != NULL)
    {
        self->_set_slpz(self->_set_slpz_ctx, 0);
    }

    self->_ready = false;
}

bool twr_ds2484_reset(twr_ds2484_t *self)
{
    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_twr_ds2484_i2c_write_byte(self, _TWR_DS2484_CMD_1WRS))
    {
        return false;
    }

    self->_srp = _TWR_DS2484_REG_ST;

    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    // self->_search_ready = true;
    return (self->_status & TWR_DS2484_STATUS_PPD) != 0;
}

bool twr_ds2484_busy_wait(twr_ds2484_t *self)
{
    if (!self->_ready)
    {
        return false;
    }

    int limit = 100;

    if (!_twr_ds2484_set_reed_pointer(self, _TWR_DS2484_REG_ST))
    {
        return false;
    }

    while(limit--)
    {
        if (!_twr_ds2484_i2c_read_byte(self, &self->_status))
        {
            return false;
        }

        if ((self->_status & TWR_DS2484_STATUS_1WB) == 0)
        {
            return true;
        }

        twr_delay_us(1000);
    }

    return false;
}

bool twr_ds2484_write_byte(twr_ds2484_t *self, const uint8_t byte)
{
    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!twr_i2c_memory_write_8b(self->_i2c_channel, _TWR_DS2484_I2C_ADDRESS, _TWR_DS2484_CMD_1WWB, byte))
    {
        return false;
    }

    self->_srp = _TWR_DS2484_REG_ST;

    return true;
}

bool twr_ds2484_read_byte(twr_ds2484_t *self, uint8_t *byte)
{
    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_twr_ds2484_i2c_write_byte(self, _TWR_DS2484_CMD_1WRB))
    {
        return false;
    }

    self->_srp = _TWR_DS2484_REG_ST;

    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_twr_ds2484_set_reed_pointer(self, _TWR_DS2484_REG_DATA))
    {
        return false;
    }

    if (!_twr_ds2484_i2c_read_byte(self, byte))
    {
        return false;
    }

    return true;
}

bool twr_ds2484_read_bit(twr_ds2484_t *self, uint8_t *bit)
{
    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_twr_ds2484_i2c_write_byte(self, _TWR_DS2484_CMD_1WSB))
    {
        return false;
    }

    self->_srp = _TWR_DS2484_REG_ST;

    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_twr_ds2484_i2c_read_byte(self, bit))
    {
        return false;
    }

    *bit = *bit & 0x80 ? 1 : 0;

    return true;
}

bool twr_ds2484_triplet(twr_ds2484_t *self, const uint8_t direction)
{
    if (!twr_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!twr_i2c_memory_write_8b(self->_i2c_channel, _TWR_DS2484_I2C_ADDRESS, _TWR_DS2484_CMD_1WT, direction ? 0x80 : 0))
    {
        return false;
    }

    self->_srp = _TWR_DS2484_REG_ST;

    return true;
}

bool twr_ds2484_is_ready(twr_ds2484_t *self)
{
    return self->_ready;
}

uint8_t twr_ds2484_status_get(twr_ds2484_t *self)
{
    return self->_status;
}

bool twr_ds2484_is_present(twr_ds2484_t *self)
{
    return _twr_ds2484_device_reset(self);
}

static bool _twr_ds2484_i2c_write_byte(twr_ds2484_t *self, const uint8_t b)
{
    uint8_t buf[1];
    twr_i2c_transfer_t transfer;

    transfer.device_address = _TWR_DS2484_I2C_ADDRESS;
    buf[0] = b;
    transfer.buffer = buf;
    transfer.length = 1;

    return twr_i2c_write(self->_i2c_channel, &transfer);
}

static bool _twr_ds2484_i2c_read_byte(twr_ds2484_t *self, uint8_t *b)
{
    twr_i2c_transfer_t transfer;

    transfer.device_address = _TWR_DS2484_I2C_ADDRESS;
    transfer.buffer = b;
    transfer.length = 1;

    return twr_i2c_read(self->_i2c_channel, &transfer);
}

static bool _twr_ds2484_device_reset(twr_ds2484_t *self)
{
    if (!_twr_ds2484_i2c_write_byte(self, _TWR_DS2484_CMD_DRST))
    {
        return false;
    }

    self->_srp = _TWR_DS2484_REG_ST;

    twr_delay_us(1000);

    return true;
}

static bool _twr_ds2484_write_config(twr_ds2484_t *self, const uint8_t cfg)
{
    uint8_t res;

    if (!twr_i2c_memory_write_8b(self->_i2c_channel, _TWR_DS2484_I2C_ADDRESS, _TWR_DS2484_CMD_WCFG, ((~cfg << 4) | cfg)))
    {
        return false;
    }

    self->_srp = _TWR_DS2484_REG_CFG;

    if (!_twr_ds2484_i2c_read_byte(self, &res))
    {
        return false;
    }

    if (res != cfg)
    {
        return false;
    }

    self->_spu_on = (cfg & _TWR_DS2484_CFG_SPU) ? true : false;

    return true;
}

static bool _twr_ds2484_set_reed_pointer(twr_ds2484_t *self, uint8_t srp)
{
    if (self->_srp != srp)
    {
        if (!twr_i2c_memory_write_8b(self->_i2c_channel, _TWR_DS2484_I2C_ADDRESS, _TWR_DS2484_CMD_SRP, srp))
        {
            return false;
        }
        self->_srp = srp;
    }
    return true;
}
