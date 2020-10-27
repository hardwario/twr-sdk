#include <bc_ds2484.h>
#include <bc_i2c.h>
#include <bc_gpio.h>
#include <bc_log.h>
#include <bc_delay.h>

#define _BC_DS2484_I2C_ADDRESS 0x18

// command set
#define _BC_DS2484_CMD_DRST (0xF0) // Device Reset
#define _BC_DS2484_CMD_SRP  (0xE1) // Set Read Pointer
#define _BC_DS2484_CMD_WCFG (0xD2) // Write Configuration
#define _BC_DS2484_CMD_CHSL (0xC3) // Channel Select(BC_DS2484-800 only)
#define _BC_DS2484_CMD_1WRS (0xB4) // 1-Wire Reset
#define _BC_DS2484_CMD_1WSB (0x87) // 1-Wire Single Bit
#define _BC_DS2484_CMD_1WWB (0xA5) // 1-Wire Write Byte
#define _BC_DS2484_CMD_1WRB (0x96) // 1-Wire Read Byte
#define _BC_DS2484_CMD_1WT  (0x78) // 1-Wire Wire Triplet

// register pointers
#define _BC_DS2484_REG_ST   (0xF0) // Status Register
#define _BC_DS2484_REG_DATA (0xE1) // Data Register
#define _BC_DS2484_REG_CFG  (0xC3) // Configuration Register

// status register BC_DS2484_REG_STATUS_* in *.h file

// configuration register _BC_DS2484_REG_CFG
#define _BC_DS2484_CFG_APU (1<<0) // Active Pullup(1==yes)
#define _BC_DS2484_CFG_SPU (1<<2) // Strong Pullup(1==yes)
#define _BC_DS2484_CFG_1WS (1<<3) // 1-Wire Speed(0==standard, 1==overdrive)

#define _BC_DS2484_CFG_DEFAULT (_BC_DS2484_CFG_APU)
// #define _BC_DS2484_PIN_SLPZ (BC_GPIO_P2)

static bool _bc_ds2484_i2c_read_byte(bc_ds2484_t *self, uint8_t *b);
static bool _bc_ds2484_i2c_write_byte(bc_ds2484_t *self, const uint8_t b);
static bool _bc_ds2484_device_reset(bc_ds2484_t *self);
static bool _bc_ds2484_write_config(bc_ds2484_t *self, const uint8_t cfg);
static bool _bc_ds2484_set_reed_pointer(bc_ds2484_t *self, uint8_t srp);

bool bc_ds2484_init(bc_ds2484_t *self, bc_i2c_channel_t i2c_channel)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;

    self->_ready = false;

    // bc_gpio_init(_BC_DS2484_PIN_SLPZ);
    // bc_gpio_set_output(_BC_DS2484_PIN_SLPZ, 0);
    // bc_gpio_set_mode(_BC_DS2484_PIN_SLPZ, BC_GPIO_MODE_OUTPUT);

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    return true;
}

void bc_ds2484_enable(bc_ds2484_t *self)
{
    self->_ready = false;

    // bc_gpio_set_output(_BC_DS2484_PIN_SLPZ, 1);

    if (!_bc_ds2484_device_reset(self))
    {
        bc_log_error("BC_DS2484: not detected");

        return;
    }

    if (!_bc_ds2484_write_config(self, _BC_DS2484_CFG_DEFAULT))
    {
        bc_log_error("BC_DS2484: set config");

        return;
    }

    self->_ready = true;

    return;
}

void bc_ds2484_disable(bc_ds2484_t *self)
{
    bc_ds2484_busy_wait(self);

    // bc_gpio_set_output(_BC_DS2484_PIN_SLPZ, 0);

    self->_ready = false;
}

bool bc_ds2484_reset(bc_ds2484_t *self)
{
    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_bc_ds2484_i2c_write_byte(self, _BC_DS2484_CMD_1WRS))
    {
        return false;
    }

    self->_srp = _BC_DS2484_REG_ST;

    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    // self->_search_ready = true;
    return (self->_status & BC_DS2484_STATUS_PPD) != 0;
}

bool bc_ds2484_busy_wait(bc_ds2484_t *self)
{
    if (!self->_ready)
    {
        return false;
    }

    int limit = 100;

    if (!_bc_ds2484_set_reed_pointer(self, _BC_DS2484_REG_ST))
    {
        return false;
    }

    while(limit--)
    {
        if (!_bc_ds2484_i2c_read_byte(self, &self->_status))
        {
            return false;
        }

        if ((self->_status & BC_DS2484_STATUS_1WB) == 0)
        {
            return true;
        }

        bc_delay_us(1000);
    }

    return false;
}

bool bc_ds2484_write_byte(bc_ds2484_t *self, const uint8_t byte)
{
    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!bc_i2c_memory_write_8b(self->_i2c_channel, _BC_DS2484_I2C_ADDRESS, _BC_DS2484_CMD_1WWB, byte))
    {
        return false;
    }

    self->_srp = _BC_DS2484_REG_ST;

    return true;
}

bool bc_ds2484_read_byte(bc_ds2484_t *self, uint8_t *byte)
{
    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_bc_ds2484_i2c_write_byte(self, _BC_DS2484_CMD_1WRB))
    {
        return false;
    }

    self->_srp = _BC_DS2484_REG_ST;

    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_bc_ds2484_set_reed_pointer(self, _BC_DS2484_REG_DATA))
    {
        return false;
    }

    if (!_bc_ds2484_i2c_read_byte(self, byte))
    {
        return false;
    }

    return true;
}

bool bc_ds2484_read_bit(bc_ds2484_t *self, uint8_t *bit)
{
    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_bc_ds2484_i2c_write_byte(self, _BC_DS2484_CMD_1WSB))
    {
        return false;
    }

    self->_srp = _BC_DS2484_REG_ST;

    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!_bc_ds2484_i2c_read_byte(self, bit))
    {
        return false;
    }

    *bit = *bit & 0x80 ? 1 : 0;

    return true;
}

bool bc_ds2484_triplet(bc_ds2484_t *self, const uint8_t direction)
{
    if (!bc_ds2484_busy_wait(self))
    {
        return false;
    }

    if (!bc_i2c_memory_write_8b(self->_i2c_channel, _BC_DS2484_I2C_ADDRESS, _BC_DS2484_CMD_1WT, direction ? 0x80 : 0))
    {
        return false;
    }

    self->_srp = _BC_DS2484_REG_ST;

    return true;
}

bool bc_ds2484_is_ready(bc_ds2484_t *self)
{
    return self->_ready;
}

uint8_t bc_ds2484_status_get(bc_ds2484_t *self)
{
    return self->_status;
}

bool bc_ds2484_is_present(bc_ds2484_t *self)
{
    return _bc_ds2484_device_reset(self);
}

static bool _bc_ds2484_i2c_write_byte(bc_ds2484_t *self, const uint8_t b)
{
    uint8_t buf[1];
    bc_i2c_transfer_t transfer;

    transfer.device_address = _BC_DS2484_I2C_ADDRESS;
    buf[0] = b;
    transfer.buffer = buf;
    transfer.length = 1;

    return bc_i2c_write(self->_i2c_channel, &transfer);
}

static bool _bc_ds2484_i2c_read_byte(bc_ds2484_t *self, uint8_t *b)
{
    bc_i2c_transfer_t transfer;

    transfer.device_address = _BC_DS2484_I2C_ADDRESS;
    transfer.buffer = b;
    transfer.length = 1;

    return bc_i2c_read(self->_i2c_channel, &transfer);
}

static bool _bc_ds2484_device_reset(bc_ds2484_t *self)
{
    if (!_bc_ds2484_i2c_write_byte(self, _BC_DS2484_CMD_DRST))
    {
        return false;
    }

    self->_srp = _BC_DS2484_REG_ST;

    bc_delay_us(1000);

    return true;
}

static bool _bc_ds2484_write_config(bc_ds2484_t *self, const uint8_t cfg)
{
    uint8_t res;

    if (!bc_i2c_memory_write_8b(self->_i2c_channel, _BC_DS2484_I2C_ADDRESS, _BC_DS2484_CMD_WCFG, ((~cfg << 4) | cfg)))
    {
        return false;
    }

    self->_srp = _BC_DS2484_REG_CFG;

    if (!_bc_ds2484_i2c_read_byte(self, &res))
    {
        return false;
    }

    if (res != cfg)
    {
        return false;
    }

    self->_spu_on = (cfg & _BC_DS2484_CFG_SPU) ? true : false;

    return true;
}

static bool _bc_ds2484_set_reed_pointer(bc_ds2484_t *self, uint8_t srp)
{
    if (self->_srp != srp)
    {
        if (!bc_i2c_memory_write_8b(self->_i2c_channel, _BC_DS2484_I2C_ADDRESS, _BC_DS2484_CMD_SRP, srp))
        {
            return false;
        }
        self->_srp = srp;
    }
    return true;
}
