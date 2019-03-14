#include <bc_ds28e17.h>
#include <bc_tick.h>
#include <bc_log.h>

static const bc_gpio_channel_t _bc_ds28e17_set_speed_lut[2] =
{
        [BC_I2C_SPEED_100_KHZ] = 0x00,
        [BC_I2C_SPEED_400_KHZ] = 0x01
};

static bool _bc_ds28e17_write(bc_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length);
static bool _bc_ds28e17_read(bc_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length);

void bc_ds28e17_init(bc_ds28e17_t *self, bc_gpio_channel_t channel, uint64_t device_number)
{
    memset(self, 0, sizeof(*self));

    self->_device_number = device_number;

    self->_channel = channel;

    bc_onewire_init(self->_channel);

    bc_onewire_auto_ds28e17_sleep_mode(true);
}

void bc_ds28e17_deinit(bc_ds28e17_t *self)
{
    (void) self;
}

uint64_t bc_ds28e17_get_device_number(bc_ds28e17_t *self)
{
    return self->_device_number;
}

bool bc_ds28e17_set_speed(bc_ds28e17_t *self, bc_i2c_speed_t speed)
{
    bc_onewire_transaction_start(self->_channel);

    if (!bc_onewire_reset(self->_channel))
    {
        bc_onewire_transaction_stop(self->_channel);

        return false;
    }

    bc_onewire_select(self->_channel, &self->_device_number);

    uint8_t buffer[2];

    buffer[0] = 0xD2;
    buffer[1] = _bc_ds28e17_set_speed_lut[speed];

    bc_onewire_write(self->_channel, buffer, sizeof(buffer));

    bc_onewire_transaction_stop(self->_channel);

    return true;
}

bool bc_ds28e17_write(bc_ds28e17_t *self, const bc_i2c_transfer_t *transfer)
{
    uint8_t head[3];

    head[0] = 0x4B;
    head[1] = transfer->device_address << 1;
    head[2] = (uint8_t) transfer->length;

    return _bc_ds28e17_write(self, head, sizeof(head), transfer->buffer, transfer->length);
}

bool bc_ds28e17_read(bc_ds28e17_t *self, const bc_i2c_transfer_t *transfer)
{
    uint8_t head[3];

    head[0] = 0x87;
    head[1] = (transfer->device_address << 1) | 0x01;
    head[2] = (uint8_t) transfer->length;

    return _bc_ds28e17_read(self, head, sizeof(head), transfer->buffer, transfer->length);
}

bool bc_ds28e17_memory_write(bc_ds28e17_t *self, const bc_i2c_memory_transfer_t *transfer)
{
    size_t head_length;
    uint8_t head[5];

    head[0] = 0x4B;
    head[1] = transfer->device_address << 1;

    if ((transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0)
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

    return _bc_ds28e17_write(self, head, head_length, transfer->buffer, transfer->length);
}

bool bc_ds28e17_memory_read(bc_ds28e17_t *self, const bc_i2c_memory_transfer_t *transfer)
{
    size_t head_length;
    uint8_t head[6];

    head[0] = 0x2D;
    head[1] = transfer->device_address << 1;

    if ((transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0)
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

    return _bc_ds28e17_read(self, head, head_length, transfer->buffer, transfer->length);
}

static bool _bc_ds28e17_write(bc_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length)
{
    bc_onewire_transaction_start(self->_channel);

    bc_onewire_reset(self->_channel);

    if (!bc_onewire_reset(self->_channel))
    {
        bc_onewire_transaction_stop(self->_channel);

        return false;
    }

    uint16_t crc16 = bc_onewire_crc16(head, head_length, 0x00);

    crc16 = bc_onewire_crc16(buffer, length, crc16);

    bc_onewire_select(self->_channel, &self->_device_number);

    bc_onewire_write(self->_channel, head, head_length);

    bc_onewire_write(self->_channel, buffer, length);

    crc16 = ~crc16;

    bc_onewire_write(self->_channel, &crc16, sizeof(crc16));

    bc_tick_t timeout = bc_tick_get() + 50;

    while (bc_onewire_read_bit(self->_channel) != 0)
    {
        if (timeout < bc_tick_get())
        {
            bc_onewire_transaction_stop(self->_channel);

            return false;
        }

        continue;
    }

    uint8_t status = bc_onewire_read_8b(self->_channel);

    uint8_t write_status = bc_onewire_read_8b(self->_channel);

    bc_onewire_transaction_stop(self->_channel);

    return (status == 0x00) && (write_status == 0x00);
}

static bool _bc_ds28e17_read(bc_ds28e17_t *self, uint8_t *head, size_t head_length, void *buffer, size_t length)
{
    bc_onewire_transaction_start(self->_channel);

    bc_onewire_reset(self->_channel);

    if (!bc_onewire_reset(self->_channel))
    {
        bc_onewire_transaction_stop(self->_channel);

        return false;
    }

    uint16_t crc16 = bc_onewire_crc16(head, head_length, 0x00);

    bc_onewire_select(self->_channel, &self->_device_number);

    bc_onewire_write(self->_channel, head, head_length);

    crc16 = ~crc16;

    bc_onewire_write(self->_channel, &crc16, sizeof(crc16));

    bc_tick_t timeout = bc_tick_get() + 50;

    while (bc_onewire_read_bit(self->_channel) != 0)
    {
        if (timeout < bc_tick_get())
        {
            bc_onewire_transaction_stop(self->_channel);

            return false;
        }

        continue;
    }

    uint8_t status = bc_onewire_read_8b(self->_channel);

    uint8_t write_status = head[0] == 0x87 ? 0 : bc_onewire_read_8b(self->_channel);

    if ((status != 0x00) || (write_status != 0x00))
    {
        bc_onewire_transaction_stop(self->_channel);

        return false;
    }

    bc_onewire_read(self->_channel, buffer, length);

    bc_onewire_transaction_stop(self->_channel);

    return true;
}
