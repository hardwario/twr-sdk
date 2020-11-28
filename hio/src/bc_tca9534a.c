#include <hio_tca9534a.h>

#define HIO_TCA9534A_REGISTER_INPUT_PORT 0x00
#define HIO_TCA9534A_REGISTER_OUTPUT_PORT 0x01
#define HIO_TCA9534A_REGISTER_POLARITY_INVERSION 0x02
#define HIO_TCA9534A_REGISTER_CONFIGURATION 0x03

bool hio_tca9534a_init(hio_tca9534a_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);

    if (!hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HIO_TCA9534A_REGISTER_CONFIGURATION, &self->_direction))
    {
        return false;
    }

    if (!hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HIO_TCA9534A_REGISTER_OUTPUT_PORT, &self->_output_port))
	{
		return false;
	}

    return true;
}

bool hio_tca9534a_read_port(hio_tca9534a_t *self, uint8_t *value)
{
    return hio_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HIO_TCA9534A_REGISTER_INPUT_PORT, value);
}

bool hio_tca9534a_write_port(hio_tca9534a_t *self, uint8_t value)
{
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HIO_TCA9534A_REGISTER_OUTPUT_PORT, value))
    {
        return false;
    }

    self->_output_port = value;

    return true;
}

bool hio_tca9534a_read_pin(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, int *value)
{
    uint8_t port;

    if (!hio_tca9534a_read_port(self, &port))
    {
        return false;
    }

    *value = ((port >> (uint8_t) pin) & 0x01);

    return true;
}

bool hio_tca9534a_write_pin(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, int value)
{
    uint8_t port = self->_output_port;

    port &= ~(1 << (uint8_t) pin);

    if (value != 0)
    {
        port |= 1 << (uint8_t) pin;
    }

    if (!hio_tca9534a_write_port(self, port))
    {
        return false;
    }

    return true;
}

bool hio_tca9534a_get_port_direction(hio_tca9534a_t *self, uint8_t *direction)
{
	*direction = self->_direction;

    return true;
}

bool hio_tca9534a_set_port_direction(hio_tca9534a_t *self, uint8_t direction)
{
    if (!hio_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HIO_TCA9534A_REGISTER_CONFIGURATION, direction))
    {
        return false;
    }

    self->_direction = direction;

    return true;
}

bool hio_tca9534a_get_pin_direction(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, hio_tca9534a_pin_direction_t *direction)
{
    if (((self->_direction >> (uint8_t) pin) & 1) == 0)
    {
        *direction = HIO_TCA9534A_PIN_DIRECTION_OUTPUT;
    }
    else
    {
        *direction = HIO_TCA9534A_PIN_DIRECTION_INPUT;
    }

    return true;
}

bool hio_tca9534a_set_pin_direction(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, hio_tca9534a_pin_direction_t direction)
{
    uint8_t port_direction = self->_direction;

    port_direction &= ~(1 << (uint8_t) pin);

    if (direction == HIO_TCA9534A_PIN_DIRECTION_INPUT)
    {
        port_direction |= 1 << (uint8_t) pin;
    }

    if (!hio_tca9534a_set_port_direction(self, port_direction))
    {
        return false;
    }

    return true;
}
