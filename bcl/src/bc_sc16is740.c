#include <bc_sc16is740.h>

#define _BC_SC16IS740_CRYSTCAL_FREQ (13560000UL)
#define _BC_SC16IS740_FIFO_SIZE               64
#define _BC_SC16IS740_REG_RHR                 0x00
#define _BC_SC16IS740_REG_THR                 0x00
#define _BC_SC16IS740_REG_IER                 0x01 << 3
#define _BC_SC16IS740_REG_FCR                 0x02 << 3
#define _BC_SC16IS740_REG_LCR                 0x03 << 3
#define _BC_SC16IS740_REG_MCR                 0x04 << 3
#define _BC_SC16IS740_BIT_FIFO_ENABLE         0x01
#define _BC_SC16IS740_REG_SPR                 0x07 << 3
#define _BC_SC16IS740_REG_TXLVL               0x08 << 3
#define _BC_SC16IS740_REG_RXLVL               0x09 << 3
#define _BC_SC16IS740_REG_IOCONTROL           0x0E << 3
#define _BC_SC16IS740_BIT_UART_SOFTWARE_RESET 0x08
#define _BC_SC16IS740_LCR_SPECIAL_REGISTER    0x80
#define _BC_SC16IS740_SPECIAL_REG_DLL         0x00 << 3
#define _BC_SC16IS740_SPECIAL_REG_DLH         0x01 << 3
#define _BC_SC16IS740_LCR_SPECIAL_ENHANCED_REGISTER  0xBF
#define _BC_SC16IS740_ENHANCED_REG_EFR        0x02 << 3

bool bc_sc16is740_init(bc_sc16is740_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    uint32_t baudrate = 9600;

    uint8_t register_lcr;
    uint8_t register_mcr;
    uint8_t register_efr;
    uint8_t register_fcr;
    uint8_t register_ier;
    uint8_t register_spr;

    uint8_t prescaler;
    uint16_t divisor;

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_MCR, &register_mcr))
    {
        return false;
    }
    if ((register_mcr & 0x80) == 0x00)
    {
        prescaler = 1;
    }
    else
    {
        prescaler = 4;
    }

    divisor = (_BC_SC16IS740_CRYSTCAL_FREQ / prescaler) / (baudrate * 16);

    //baudrate
    register_lcr = _BC_SC16IS740_LCR_SPECIAL_REGISTER; //switch to access Special register
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_LCR, register_lcr))
    {
        return false;
    }
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_SPECIAL_REG_DLL, (uint8_t) divisor))
    {
        return false;
    }
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_SPECIAL_REG_DLH, (uint8_t) (divisor >> 8)))
    {
        return false;
    }

    //no transmit flow control
    register_lcr = _BC_SC16IS740_LCR_SPECIAL_ENHANCED_REGISTER; //switch to access Enhanced register
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_LCR, register_lcr))
    {
        return false;
    }
    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_ENHANCED_REG_EFR, &register_efr))
    {
        return false;
    }
    register_efr &= 0xf0;
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_ENHANCED_REG_EFR, register_efr))
    {
        return false;
    }

    register_lcr = 0x07; //General register set
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_LCR, register_lcr))
    {
        return false;
    }

    // fifo enabled
    register_fcr = _BC_SC16IS740_BIT_FIFO_ENABLE;
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_FCR, register_fcr))
    {
        return false;
    }

    // Polled mode operation
    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_IER, &register_ier))
    {
        return false;
    }
    register_ier &= 0xf0;
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_IER, register_ier))
    {
        return false;
    }

    // no break, no parity, 2 stop bits, 8 data bits
    register_lcr = 0x07;

    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_LCR, register_lcr))
    {
        return false;
    }

    // TEST
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_SPR, 0x48))
    {
        return false;
    }
    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_SPR, &register_spr))
    {
        return false;
    }

    return register_spr == 0x48;
}

bool bc_sc16is740_reset_fifo(bc_sc16is740_t *self, bc_sc16is740_fifo_t fifo)
{
    uint8_t register_fcr;
    register_fcr = fifo | _BC_SC16IS740_BIT_FIFO_ENABLE;
    return bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_FCR, register_fcr);
}

bool bc_sc16is740_get_spaces_available(bc_sc16is740_t *self, uint8_t *spaces_available)
{
    return bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_TXLVL, spaces_available);
}

uint8_t bc_sc16is740_write(bc_sc16is740_t *self, uint8_t *buffer, uint8_t length)
{
    uint8_t spaces_available;

    if (length > _BC_SC16IS740_FIFO_SIZE)
    {
        return false;
    }

    if (!bc_sc16is740_get_spaces_available(self, &spaces_available))
    {
        return false;
    }

    if (spaces_available < length)
    {
        return false;
    }

    bc_i2c_tranfer_t transfer;
    transfer.device_address = self->_i2c_address;
    transfer.memory_address = _BC_SC16IS740_REG_THR;
    transfer.length = length;
    transfer.buffer = buffer;

    if (!bc_i2c_write(self->_i2c_channel, &transfer))
    {
        return 0;
    }

    return length;
}

bool bc_sc16is740_available(bc_sc16is740_t *self, uint8_t *available)
{
    return bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, _BC_SC16IS740_REG_RXLVL, available);
}

uint8_t bc_sc16is740_read(bc_sc16is740_t *self, uint8_t *buffer, uint8_t length, bc_tick_t timeout)
{
    uint8_t read_length = 0;
    bc_tick_t stop = bc_tick_get() + timeout;
    uint8_t available;
    bc_i2c_tranfer_t transfer;
    transfer.device_address = self->_i2c_address;
    transfer.memory_address = _BC_SC16IS740_REG_RHR;
    transfer.buffer = buffer;

    while (bc_tick_get() < stop)
    {
        if (!bc_sc16is740_available(self, &available))
        {
            return 0;
        }

        if (available != 0)
        {
            transfer.buffer = buffer + read_length;
            transfer.length = length - read_length;
            if (transfer.length > available)
            {
                transfer.length = available;
            }

            if (transfer.length < 1)
            {
                return 0;
            }

            if (!bc_i2c_read(self->_i2c_channel, &transfer))
            {
                return 0;
            }

            read_length += transfer.length;

            if (read_length == length)
            {
                return read_length;
            }
        }
        else
        {
            //TODO sleep or rewrite to task and callback
        }
    }

    return read_length;
}
