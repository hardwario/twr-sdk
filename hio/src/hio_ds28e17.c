#include <hio_ds28e17.h>
#include <hio_tick.h>
#include <hio_log.h>

static const hio_gpio_channel_t _hio_ds28e17_set_speed_lut[2] =
{
        [HIO_I2C_SPEED_100_KHZ] = 0x00,
        [HIO_I2C_SPEED_400_KHZ] = 0x01
};

static bool _hio_ds28e17_write(hio_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length);
static bool _hio_ds28e17_read(hio_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length);

void hio_ds28e17_init(hio_ds28e17_t *self, hio_onewire_t *onewire, uint64_t device_number)
{
    memset(self, 0, sizeof(*self));

    self->_device_number = device_number;

    self->_onewire = onewire;

    hio_onewire_auto_ds28e17_sleep_mode(self->_onewire, true);
}

void hio_ds28e17_deinit(hio_ds28e17_t *self)
{
    (void) self;
}

uint64_t hio_ds28e17_get_device_number(hio_ds28e17_t *self)
{
    return self->_device_number;
}

bool hio_ds28e17_set_speed(hio_ds28e17_t *self, hio_i2c_speed_t speed)
{
    hio_onewire_transaction_start(self->_onewire);

    if (!hio_onewire_reset(self->_onewire))
    {
        hio_onewire_transaction_stop(self->_onewire);

        return false;
    }

    hio_onewire_select(self->_onewire, &self->_device_number);

    uint8_t buffer[2];

    buffer[0] = 0xD2;
    buffer[1] = _hio_ds28e17_set_speed_lut[speed];

    hio_onewire_write(self->_onewire, buffer, sizeof(buffer));

    hio_onewire_transaction_stop(self->_onewire);

    return true;
}

bool hio_ds28e17_write(hio_ds28e17_t *self, const hio_i2c_transfer_t *transfer)
{
    uint8_t head[3];

    head[0] = 0x4B;
    head[1] = transfer->device_address << 1;
    head[2] = (uint8_t) transfer->length;

    return _hio_ds28e17_write(self, head, sizeof(head), transfer->buffer, transfer->length);
}

bool hio_ds28e17_read(hio_ds28e17_t *self, const hio_i2c_transfer_t *transfer)
{
    uint8_t head[3];

    head[0] = 0x87;
    head[1] = (transfer->device_address << 1) | 0x01;
    head[2] = (uint8_t) transfer->length;

    return _hio_ds28e17_read(self, head, sizeof(head), transfer->buffer, transfer->length);
}

bool hio_ds28e17_memory_write(hio_ds28e17_t *self, const hio_i2c_memory_transfer_t *transfer)
{
    size_t head_length;
    uint8_t head[5];

    head[0] = 0x4B;
    head[1] = transfer->device_address << 1;

    if ((transfer->memory_address & HIO_I2C_MEMORY_ADDRESS_16_BIT) != 0)
    {
        head_length = 5;
        head[2] = (uint8_t) transfer->length + 2;
        head[3] = transfer->memory_address >> 8;
        head[4] = transfer->memory_address;
    }
    else
    {
        head_length = 4;
        head[2] = (uint8_t) transfer->length + 1;
        head[3] = (uint8_t) transfer->memory_address;
    }

    return _hio_ds28e17_write(self, head, head_length, transfer->buffer, transfer->length);
}

bool hio_ds28e17_memory_read(hio_ds28e17_t *self, const hio_i2c_memory_transfer_t *transfer)
{
    size_t head_length;
    uint8_t head[6];

    head[0] = 0x2D;
    head[1] = transfer->device_address << 1;

    if ((transfer->memory_address & HIO_I2C_MEMORY_ADDRESS_16_BIT) != 0)
    {
        head_length = 6;
        head[2] = 2;
        head[3] = transfer->memory_address >> 8;
        head[4] = transfer->memory_address;
        head[5] = transfer->length;
    }
    else
    {
        head_length = 5;
        head[2] = 1;
        head[3] = transfer->memory_address;
        head[4] = transfer->length;
    }

    return _hio_ds28e17_read(self, head, head_length, transfer->buffer, transfer->length);
}

static bool _hio_ds28e17_write(hio_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length)
{
    hio_onewire_transaction_start(self->_onewire);

    hio_onewire_reset(self->_onewire);

    if (!hio_onewire_reset(self->_onewire))
    {
        hio_onewire_transaction_stop(self->_onewire);

        return false;
    }

    uint16_t crc16 = hio_onewire_crc16(head, head_length, 0x00);

    crc16 = hio_onewire_crc16(buffer, length, crc16);

    hio_onewire_select(self->_onewire, &self->_device_number);

    hio_onewire_write(self->_onewire, head, head_length);

    hio_onewire_write(self->_onewire, buffer, length);

    crc16 = ~crc16;

    hio_onewire_write(self->_onewire, &crc16, sizeof(crc16));

    hio_tick_t timeout = hio_tick_get() + 50;

    while (hio_onewire_read_bit(self->_onewire) != 0)
    {
        if (timeout < hio_tick_get())
        {
            hio_onewire_transaction_stop(self->_onewire);

            return false;
        }

        continue;
    }

    uint8_t status = hio_onewire_read_byte(self->_onewire);

    uint8_t write_status = hio_onewire_read_byte(self->_onewire);

    hio_onewire_transaction_stop(self->_onewire);

    return (status == 0x00) && (write_status == 0x00);
}

static bool _hio_ds28e17_read(hio_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length)
{
    hio_onewire_transaction_start(self->_onewire);

    hio_onewire_reset(self->_onewire);

    if (!hio_onewire_reset(self->_onewire))
    {
        hio_onewire_transaction_stop(self->_onewire);

        return false;
    }

    uint16_t crc16 = hio_onewire_crc16(head, head_length, 0x00);

    hio_onewire_select(self->_onewire, &self->_device_number);

    hio_onewire_write(self->_onewire, head, head_length);

    crc16 = ~crc16;

    hio_onewire_write(self->_onewire, &crc16, sizeof(crc16));

    hio_tick_t timeout = hio_tick_get() + 50;

    while (hio_onewire_read_bit(self->_onewire) != 0)
    {
        if (timeout < hio_tick_get())
        {
            hio_onewire_transaction_stop(self->_onewire);

            return false;
        }

        continue;
    }

    uint8_t status = hio_onewire_read_byte(self->_onewire);

    uint8_t write_status = head[0] == 0x87 ? 0 : hio_onewire_read_byte(self->_onewire);

    if ((status != 0x00) || (write_status != 0x00))
    {
        hio_onewire_transaction_stop(self->_onewire);

        return false;
    }

    hio_onewire_read(self->_onewire, buffer, length);

    hio_onewire_transaction_stop(self->_onewire);

    return true;
}
