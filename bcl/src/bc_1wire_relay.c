#include <bc_1wire_relay.h>
#include <bc_gpio.h>

bool _bc_onewire_relay_read_state(bc_1wire_relay_t *self);

bool bc_1wire_relay_init(bc_1wire_relay_t *self, uint64_t device_number, bc_module_sensor_channel_t channel)
{
    memset(self, 0, sizeof(*self));
    self->_device_number = device_number;

    self->_channel = channel == BC_MODULE_SENSOR_CHANNEL_A ? BC_GPIO_P4 : BC_GPIO_P5;

    bc_1wire_init(self->_channel);

    if (!bc_module_sensor_init())
    {
        return false;
    }

    if (!bc_module_sensor_set_pull(channel, BC_MODULE_SENSOR_PULL_UP_4K7))
    {
        return false;
    }

    if (!_bc_onewire_relay_read_state(self))
    {
        return false;
    }

    return true;
}

bool bc_1wire_relay_set_state(bc_1wire_relay_t *self, bc_1wire_relay_q_t q, bool state)
{

    if (!bc_1wire_reset(self->_channel))
    {
        return false;
    }

    uint8_t new_state = self->_state;

    if (!state)
    {
        new_state |= (1 << q);
    }
    else
    {
        new_state &= ~(1 << q);
    }

    bc_1wire_select(self->_channel, &self->_device_number);

    uint8_t buffer[] = { 0x5A, new_state, ~new_state };

    bc_1wire_write(self->_channel, buffer, sizeof(buffer));

    if (bc_1wire_read_8b(self->_channel) != 0xAA)
    {
        return false;
    }

    if (bc_1wire_read_8b(self->_channel) != new_state)
    {
        return false;
    }

    self->_state = new_state;

    return true;
}

bool bc_1wire_relay_get_state(bc_1wire_relay_t *self, bc_1wire_relay_q_t q, bool *state)
{
    *state = ((self->_state >> q) & 0x01) == 0x00;

    return true;
}

bool _bc_onewire_relay_read_state(bc_1wire_relay_t *self)
{
    if (!bc_1wire_reset(self->_channel))
    {
        return false;
    }

    bc_1wire_select(self->_channel, &self->_device_number);

    bc_1wire_write_8b(self->_channel, 0xF5);

    self->_state = bc_1wire_read_8b(self->_channel);

    return true;
}

