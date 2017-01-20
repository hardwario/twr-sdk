#include <bc_tca9534a.h>
#include <bc_scheduler.h>

#include <bc_module_relay.h>

void bc_tca9534a_test()
{
    /*uint8_t data[4];

    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x00, &data[0]);
    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x01, &data[1]);
    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x02, &data[2]);
    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x03, &data[3]);

    uint8_t out_port = (1 << 7);
    uint8_t out_dir = 0xF0;
    bc_i2c_write_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x01, out_port);
    bc_i2c_write_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x03, ~out_dir);


    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x00, &data[0]);
    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x01, &data[1]);
    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x02, &data[2]);
    bc_i2c_read_8b(BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT, 0x03, &data[3]);
*/
    bc_tca9534a_t instance;

    bc_tca9534a_init(&instance, BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    bc_tca9534a_set_port_direction(&instance, 0x0F); // inverted: O = output

    bc_tca9534a_write_port(&instance, (1 << 6) | (1 << 7)); // pol A

    bc_tca9534a_write_port(&instance, (1 << 6) | (1 << 4)); // off

    bc_tca9534a_write_port(&instance, (1 << 4) | (1 << 5)); // pol B

    bc_tca9534a_write_port(&instance, (1 << 6) | (1 << 4)); // off


    bc_tca9534a_write_port(&instance, (1 << 6) | (1 << 7)); // pol A

    bc_tca9534a_write_port(&instance, (1 << 6) | (1 << 4)); // off

    bc_tca9534a_write_port(&instance, (1 << 4) | (1 << 5)); // pol B

    bc_tca9534a_write_port(&instance, (1 << 6) | (1 << 4)); // off


}


bool bc_tca9534a_init(bc_tca9534a_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    uint8_t direction;

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, BC_TCA9534A_REGISTER_CONFIGURATION, &direction))
    {
        return false;
    }

    return true;
}

bool bc_tca9534a_read_port(bc_tca9534a_t *self, uint8_t *value)
{
    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, BC_TCA9534A_REGISTER_INPUT_PORT, value))
    {
        return false;
    }

    return true;
}

bool bc_tca9534a_write_port(bc_tca9534a_t *self, uint8_t value)
{
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, BC_TCA9534A_REGISTER_OUTPUT_PORT, value))
    {
        return false;
    }

    return true;
}

bool bc_tca9534a_read_pin(bc_tca9534a_t *self, bc_tca9534a_pin_t pin, bc_tca9534a_value_t *value)
{
    uint8_t port;

    if (!bc_tca9534a_read_port(self, &port))
    {
        return false;
    }

    if (((port >> (uint8_t) pin) & 1) == 0)
    {
        *value = BC_I2C_TCA9534A_VALUE_LOW;
    }
    else
    {
        *value = BC_I2C_TCA9534A_VALUE_HIGH;
    }

    return true;
}

bool bc_tca9534a_write_pin(bc_tca9534a_t *self, bc_tca9534a_pin_t pin, bc_tca9534a_value_t value)
{
    uint8_t port;

    if (!bc_tca9534a_read_port(self, &port))
    {
        return false;
    }

    port &= ~(1 << (uint8_t) pin);

    if (value != BC_I2C_TCA9534A_VALUE_LOW)
    {
        port |= 1 << (uint8_t) pin;
    }

    if (!bc_tca9534a_write_port(self, port))
    {
        return false;
    }

    return true;
}

bool bc_tca9534a_get_port_direction(bc_tca9534a_t *self, uint8_t *direction)
{
    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, BC_TCA9534A_REGISTER_CONFIGURATION, direction))
    {
        return false;
    }

    return true;
}

bool bc_tca9534a_set_port_direction(bc_tca9534a_t *self, uint8_t direction)
{
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, BC_TCA9534A_REGISTER_CONFIGURATION, direction))
    {
        return false;
    }

    return true;
}

bool bc_tca9534a_get_pin_direction(bc_tca9534a_t *self, bc_tca9534a_pin_t pin,
                                       bc_tca9534a_direction_t *direction)
{
    uint8_t port_direction;

    if (!bc_tca9534a_get_port_direction(self, &port_direction))
    {
        return false;
    }

    if (((port_direction >> (uint8_t) pin) & 1) == 0)
    {
        *direction = BC_I2C_TCA9534A_DIRECTION_OUTPUT;
    }
    else
    {
        *direction = BC_I2C_TCA9534A_DIRECTION_INPUT;
    }

    return true;
}

bool bc_tca9534a_set_pin_direction(bc_tca9534a_t *self, bc_tca9534a_pin_t pin,
                                       bc_tca9534a_direction_t direction)
{
    uint8_t port_direction;

    if (!bc_tca9534a_get_port_direction(self, &port_direction))
    {
        return false;
    }

    port_direction &= ~(1 << (uint8_t) pin);

    if (direction == BC_I2C_TCA9534A_DIRECTION_INPUT)
    {
        port_direction |= 1 << (uint8_t) pin;
    }

    if (!bc_tca9534a_set_port_direction(self, port_direction))
    {
        return false;
    }

    return true;
}
