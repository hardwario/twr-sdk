#include <twr_chester_a.h>
#include <twr_log.h>

#define RELAY_1_MASK (1 << TWR_TCA9534A_PIN_P0)
#define RELAY_2_MASK (1 << TWR_TCA9534A_PIN_P1)

static bool init_tca9534a(twr_chester_a_t *self)
{
    if (self->is_tca9534a_initialized)
    {
        return true;
    }

    if (!twr_tca9534a_write_port(&self->tca9534a, 0x00)) {
        twr_log_error("CHESTER A: Expander write_port");
        return false;
    }

    if (!twr_tca9534a_set_port_direction(&self->tca9534a, 0x00)) {
        twr_log_error("CHESTER A: Expander port_direction");
        return false;
    }

    self->is_tca9534a_initialized = true;

    return true;
}

bool twr_chester_a_init(twr_chester_a_t *self, twr_i2c_channel_t i2c_channel)
{
    self->is_tca9534a_initialized = false;

    twr_tca9534a_init(&self->tca9534a, i2c_channel, 0x3a);

    return init_tca9534a(self);
}

bool twr_chester_a_relay_set_state(twr_chester_a_t *self, twr_chester_a_relay_t relay, bool state)
{
    if (!init_tca9534a(self))
    {
        return false;
    }

    uint8_t port = self->tca9534a._output_port;

    switch (relay)
    {
        case TWR_CHESTER_A_RELAY_1:
        {
            if (state) {
                port |= RELAY_1_MASK;
            } else {
                port &= ~RELAY_1_MASK;
            }
            break;
        }
        case TWR_CHESTER_A_RELAY_2:
        {
            if (state) {
                port |= RELAY_2_MASK;
            } else {
                port &= ~RELAY_2_MASK;
            }
            break;
        }
        case TWR_CHESTER_A_RELAY_BOTH:
        {
            if (state) {
                port |= RELAY_1_MASK | RELAY_2_MASK;
            } else {
                port &= ~(RELAY_1_MASK | RELAY_2_MASK);
            }
            break;
        }
        default:
        {
            twr_log_error("CHESTER A: unknown relay");
            return false;
        }
    }

    if (!twr_tca9534a_write_port(&self->tca9534a, port)) {
        self->is_tca9534a_initialized = false;
        twr_log_error("CHESTER A: Expander write_port");
        return false;
    }

    return true;
}

bool twr_chester_a_relay_get_state(twr_chester_a_t *self, twr_chester_a_relay_t relay, bool *state)
{
    if (!init_tca9534a(self))
    {
        return false;
    }

    uint8_t port = self->tca9534a._output_port;

    switch (relay)
    {
        case TWR_CHESTER_A_RELAY_1:
        {
            *state = (port & RELAY_1_MASK) != 0;
            break;
        }
        case TWR_CHESTER_A_RELAY_2:
        {
            *state = (port & RELAY_2_MASK) != 0;
            break;
        }
        case TWR_CHESTER_A_RELAY_BOTH:
        default:
        {
            twr_log_error("CHESTER A: unknown relay");
            return false;
        }
    }

    return true;
}

bool twr_chester_a_relay_toggle(twr_chester_a_t *self, twr_chester_a_relay_t relay)
{
    bool state;
    return twr_chester_a_relay_get_state(self, relay, &state) &&
           twr_chester_a_relay_set_state(self, relay, !state);
}

