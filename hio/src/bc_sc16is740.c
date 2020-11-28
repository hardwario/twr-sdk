#include <hio_sc16is740.h>

#define _HIO_SC16IS740_CRYSTCAL_FREQ           (13560000UL)
#define _HIO_SC16IS740_FIFO_SIZE               64
#define _HIO_SC16IS740_REG_RHR                 0x00
#define _HIO_SC16IS740_REG_THR                 0x00
#define _HIO_SC16IS740_REG_IER                 0x01 << 3
#define _HIO_SC16IS740_REG_FCR                 0x02 << 3
#define _HIO_SC16IS740_REG_LCR                 0x03 << 3
#define _HIO_SC16IS740_REG_MCR                 0x04 << 3
#define _HIO_SC16IS740_BIT_FIFO_ENABLE         0x01
#define _HIO_SC16IS740_REG_SPR                 0x07 << 3
#define _HIO_SC16IS740_REG_TXLVL               0x08 << 3
#define _HIO_SC16IS740_REG_RXLVL               0x09 << 3
#define _HIO_SC16IS740_REG_IOCONTROL           0x0E << 3
#define _HIO_SC16IS740_BIT_UART_SOFTWARE_RESET 0x08
#define _HIO_SC16IS740_LCR_SPECIAL_REGISTER    0x80
#define _HIO_SC16IS740_SPECIAL_REG_DLL         0x00 << 3
#define _HIO_SC16IS740_SPECIAL_REG_DLH         0x01 << 3
#define _HIO_SC16IS740_LCR_SPECIAL_ENHANCED_REGISTER  0xBF
#define _HIO_SC16IS740_ENHANCED_REG_EFR        0x02 << 3

bool hio_sc16is740_init(hio_sc16is740_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);

    // Switch to access Special register
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_LCR, _HIO_SC16IS740_LCR_SPECIAL_REGISTER))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_SPECIAL_REG_DLL, 0x58))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_SPECIAL_REG_DLH, 0x00))
    {
        return false;
    }

    // Switch to access Enhanced register
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_LCR, _HIO_SC16IS740_LCR_SPECIAL_ENHANCED_REGISTER))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_ENHANCED_REG_EFR, 0x10))
    {
        return false;
    }

    // No break, no parity, 2 stop bits, 8 data bits
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_LCR, 0x07))
    {
        return false;
    }

    // FIFO enabled, FIFO reset RX and TX
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_FCR, 0x07))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_IER, 0x11))
    {
        return false;
    }

    return true;
}

bool hio_sc16is740_reset_fifo(hio_sc16is740_t *self, hio_sc16is740_fifo_t fifo)
{
    uint8_t register_fcr;

    register_fcr = fifo | _HIO_SC16IS740_BIT_FIFO_ENABLE;

    return hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_FCR, register_fcr);
}

bool hio_sc16is740_get_spaces_available(hio_sc16is740_t *self, size_t *spaces_available)
{
    uint8_t value;

    if (!hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_TXLVL, &value))
    {
        return false;
    }

    *spaces_available = value;

    return true;
}

size_t hio_sc16is740_write(hio_sc16is740_t *self, uint8_t *buffer, size_t length)
{
    size_t spaces_available;

    if (length > _HIO_SC16IS740_FIFO_SIZE)
    {
        return 0;
    }

    if (!hio_sc16is740_get_spaces_available(self, &spaces_available))
    {
        return 0;
    }

    if (spaces_available < length)
    {
        return 0;
    }

    hio_i2c_memory_transfer_t transfer;

    transfer.device_address = self->_i2c_address;
    transfer.memory_address = _HIO_SC16IS740_REG_THR;
    transfer.length = length;
    transfer.buffer = buffer;

    if (!hio_i2c_memory_write(self->_i2c_channel, &transfer))
    {
        return 0;
    }

    return length;
}

bool hio_sc16is740_available(hio_sc16is740_t *self, size_t *available)
{
    uint8_t value;

    if (!hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_RXLVL, &value))
    {
        return false;
    }

    *available = value;

    return true;
}

size_t hio_sc16is740_read(hio_sc16is740_t *self, uint8_t *buffer, size_t length, hio_tick_t timeout)
{
    size_t read_length = 0;

    hio_tick_t stop = (timeout != HIO_TICK_INFINITY) ? hio_tick_get() + timeout : HIO_TICK_INFINITY;

    do
    {
        size_t available;

        if (!hio_sc16is740_available(self, &available))
        {
            return 0;
        }

        if (available != 0)
        {
            hio_i2c_memory_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.memory_address = _HIO_SC16IS740_REG_RHR;
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

            if (!hio_i2c_memory_read(self->_i2c_channel, &transfer))
            {
                return 0;
            }

            read_length += transfer.length;

            if (read_length == length)
            {
                return read_length;
            }
        }

    } while (hio_tick_get() > stop);

    return read_length;
}

bool hio_sc16is740_set_baudrate(hio_sc16is740_t *self, hio_sc16is740_baudrate_t baudrate)
{
    uint8_t lcr_read;

    // Copy LCR
    if (!hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_LCR, &lcr_read))
    {
        return false;
    }

    // Enable access to special registers
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_LCR, _HIO_SC16IS740_LCR_SPECIAL_REGISTER))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_SPECIAL_REG_DLL, baudrate))
    {
        return false;
    }

    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_SPECIAL_REG_DLH, 0x00))
    {
        return false;
    }

    // Restore LCR
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, _HIO_SC16IS740_REG_LCR, lcr_read))
    {
        return false;
    }

    return true;
}
