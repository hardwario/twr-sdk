#include <hio_atsha204.h>
#include <hio_tick.h>

#define _HIO_ATSHA204_OPCODE_NULL  0x00
#define _HIO_ATSHA204_OPCODE_DEVREV 0x30
#define _HIO_ATSHA204_OPCODE_READ 0x02

static void _hio_atsha204_task(void *param);
static bool _hio_atsha204_send_command(hio_atsha204_t *self, uint8_t opcode, uint8_t param0, uint16_t param1);
static void _hio_atsha204_wakeup_puls(hio_atsha204_t *self);
static bool _hio_atsha204_wakeup(hio_atsha204_t *self);
static bool _hio_atsha204_read(hio_atsha204_t *self, uint8_t *buffer, size_t length);
static uint16_t _hio_atsha204_calculate_crc16(uint8_t *buffer, uint8_t length);

void hio_atsha204_init(hio_atsha204_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);

    self->_task_id = hio_scheduler_register(_hio_atsha204_task, self, HIO_TICK_INFINITY);

    self->_ready = true;
}

void hio_atsha204_set_event_handler(hio_atsha204_t *self, void (*event_handler)(hio_atsha204_t *, hio_atsha204_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

bool hio_atsha204_is_ready(hio_atsha204_t *self)
{
    return self->_ready;
}

bool hio_atsha204_read_serial_number(hio_atsha204_t *self)
{
    if (!hio_atsha204_is_ready(self))
    {
        return false;
    }

    if (!_hio_atsha204_send_command(self, _HIO_ATSHA204_OPCODE_READ, 0, 0x00))
    {
        return false;
    }

    self->_ready = false;
    self->_state = HIO_ATSHA204_STATE_READ_SERIAL_NUMBER;

    hio_scheduler_plan_relative(self->_task_id, 4);
    return true;
}

bool hio_atsha204_get_serial_number(hio_atsha204_t *self, void *destination, size_t size)
{
    if (!hio_atsha204_is_ready(self) || self->_state != HIO_ATSHA204_STATE_SERIAL_NUMBER)
    {
        return false;
    }

    uint8_t *number = (uint8_t *) destination;

    size_t i = 0;

    for (; (i < size) && (i < 2); i++)
    {
        *number++ = self->_rx_buffer[3 + i];
    }

    for (; (i < size) && (i < 6); i++)
    {
        *number++ = self->_rx_buffer[6 + i];
    }

    for (; i < size; i++)
    {
        *number++ = 0;
    }

    return true;
}

static void _hio_atsha204_task(void *param)
{
    hio_atsha204_t *self = param;

    hio_atsha204_event_t event = HIO_ATSHA204_EVENT_ERROR;

    switch (self->_state) {
        case HIO_ATSHA204_STATE_READ_SERIAL_NUMBER:
        {
            if (!_hio_atsha204_read(self, self->_rx_buffer, 7))
            {
                break;
            }

            if (!_hio_atsha204_send_command(self, _HIO_ATSHA204_OPCODE_READ, 0, 0x02))
            {
                break;
            }

            self->_state = HIO_ATSHA204_STATE_READ_SERIAL_NUMBER2;
            hio_scheduler_plan_current_from_now(4);
            return;
        }
        case HIO_ATSHA204_STATE_READ_SERIAL_NUMBER2:
        {
            if (!_hio_atsha204_read(self, self->_rx_buffer + 7, 7))
            {
                break;
            }

            self->_state = HIO_ATSHA204_STATE_SERIAL_NUMBER;
            event = HIO_ATSHA204_EVENT_SERIAL_NUMBER;

            break;
        }
        case HIO_ATSHA204_STATE_READY:
        case HIO_ATSHA204_STATE_SERIAL_NUMBER:
        default:
        {
            return;
        }
    }

    self->_ready = true;

    if (self->_event_handler)
    {
        self->_event_handler(self, event, self->_event_param);
    }


}

static bool _hio_atsha204_send_command(hio_atsha204_t *self, uint8_t opcode, uint8_t param0, uint16_t param1)
{
    uint8_t buffer[8];
    buffer[0] = 0x03;
    buffer[1] = 7;
    buffer[2] = opcode;
    buffer[3] = param0;
    buffer[4] = param1 & 0xff;
    buffer[5] = param1 >> 8;

    uint16_t crc = _hio_atsha204_calculate_crc16(buffer + 1, 5);

    buffer[6] = crc & 0xff;
    buffer[7] = crc >> 8;

    hio_i2c_transfer_t transfer;
    transfer.device_address = self->_i2c_address;
    transfer.buffer = buffer;
    transfer.length = 8;

    if (!hio_i2c_write(self->_i2c_channel, &transfer))
    {
        if (!_hio_atsha204_wakeup(self))
        {
            return false;
        }

        if (!hio_i2c_write(self->_i2c_channel, &transfer))
        {
            return false;
        }
    }

    return true;
}

static bool _hio_atsha204_read(hio_atsha204_t *self, uint8_t *buffer, size_t length)
{
    hio_i2c_transfer_t transfer;
    transfer.device_address = self->_i2c_address;
    transfer.buffer = buffer;
    transfer.length = length;

    if (!hio_i2c_read(self->_i2c_channel, &transfer))
    {

        _hio_atsha204_wakeup_puls(self);

        if (!hio_i2c_read(self->_i2c_channel, &transfer))
        {
            if (!hio_i2c_read(self->_i2c_channel, &transfer))
            {
                return false;
            }
        }
    }

    uint16_t crc = _hio_atsha204_calculate_crc16(buffer, length - 2);

    return (buffer[0] == length) &&
            (buffer[length - 2] == (uint8_t) (crc & 0x00FF)) &&
            (buffer[length - 1] = (uint8_t) (crc >> 8));
}

static void _hio_atsha204_wakeup_puls(hio_atsha204_t *self)
{
    hio_i2c_set_speed(self->_i2c_channel, HIO_I2C_SPEED_100_KHZ);

    hio_i2c_transfer_t transfer = {.device_address = 0x00, .buffer = NULL, .length = 0};

    hio_i2c_write(self->_i2c_channel, &transfer);

    hio_i2c_set_speed(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);
}

static bool _hio_atsha204_wakeup(hio_atsha204_t *self)
{
    _hio_atsha204_wakeup_puls(self);

    uint8_t buffer[4];
    hio_i2c_transfer_t transfer;
    transfer.device_address = self->_i2c_address;
    transfer.buffer = buffer;
    transfer.length = sizeof(buffer);

    if (!hio_i2c_read(self->_i2c_channel, &transfer))
    {
        if (!hio_i2c_read(self->_i2c_channel, &transfer))
        {
            return false;
        }
    }

    return ((buffer[0] == 0x04) && buffer[1] == 0x11 && buffer[2] == 0x33 && buffer[3] == 0x43);
}

static uint16_t _hio_atsha204_calculate_crc16(uint8_t *buffer, uint8_t length)
{
    uint16_t crc16;
    uint8_t shift_register;
    uint8_t data;
    uint8_t data_bit;
    uint8_t crc_bit;

    for (crc16 = 0; length != 0; length--, buffer++)
    {
        data = *buffer;

        for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {

            data_bit = (data & shift_register) ? 1 : 0;

            crc_bit = crc16 >> 15;

            crc16 <<= 1;

            if (data_bit != crc_bit)
            {
                crc16 ^= 0x8005;
            }
        }
    }

    return crc16;
}
