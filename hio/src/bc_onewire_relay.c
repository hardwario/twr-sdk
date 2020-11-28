#include <hio_gpio.h>
#include <hio_onewire_relay.h>

bool _hio_onewire_relay_read_state(hio_onewire_relay_t *self);

bool hio_onewire_relay_init(hio_onewire_relay_t *self, hio_onewire_t *onewire, uint64_t device_number)
{
    memset(self, 0, sizeof(*self));
    self->_device_number = device_number;
    self->_onewire = onewire;

    if (!_hio_onewire_relay_read_state(self))
    {
        return false;
    }

    return true;
}

bool hio_onewire_relay_set_state(hio_onewire_relay_t *self, hio_onewire_relay_channel_t relay_channel, bool state)
{
    if (!self->_state_valid && !_hio_onewire_relay_read_state(self))
    {
        return false;
    }

    if (!hio_onewire_reset(self->_onewire))
    {
        return false;
    }

    uint8_t new_state = self->_state;

    if (!state)
    {
        new_state |= (1 << relay_channel);
    }
    else
    {
        new_state &= ~(1 << relay_channel);
    }

    hio_onewire_select(self->_onewire, &self->_device_number);

    uint8_t buffer[] = { 0x5A, new_state, ~new_state };

    hio_onewire_write(self->_onewire, buffer, sizeof(buffer));

    if (hio_onewire_read_byte(self->_onewire) != 0xAA)
    {
        return false;
    }

    if (hio_onewire_read_byte(self->_onewire) != new_state)
    {
        return false;
    }

    self->_state = new_state;

    return true;
}

bool hio_onewire_relay_get_state(hio_onewire_relay_t *self, hio_onewire_relay_channel_t relay_channel, bool *state)
{
    if (!self->_state_valid)
    {
        return false;
    }

    *state = ((self->_state >> relay_channel) & 0x01) == 0x00;

    return true;
}

bool _hio_onewire_relay_read_state(hio_onewire_relay_t *self)
{
    self->_state_valid = false;

    if (!hio_onewire_reset(self->_onewire))
    {
        return false;
    }

    hio_onewire_select(self->_onewire, &self->_device_number);

    hio_onewire_write_byte(self->_onewire, 0xF5);

    self->_state = hio_onewire_read_byte(self->_onewire);

    self->_state_valid = true;

    return true;
}
