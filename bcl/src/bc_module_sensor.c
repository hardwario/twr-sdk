#include <bc_module_sensor.h>
#include <bc_tca9534a.h>
#include <bc_onewire.h>

#define _BC_MODULE_SENSOR_INITIALIZED_STATE 0xff
#define _BC_MODULE_SENSOR_INITIALIZED_DIRECTION 0x00
#define _BC_MODULE_SENSOR_VDD        (1 << 3)
#define _BC_MODULE_SENSOR_CH_A_56R   (1 << 4)
#define _BC_MODULE_SENSOR_CH_A_4K7   (1 << 5)
#define _BC_MODULE_SENSOR_CH_B_4K7   (1 << 6)
#define _BC_MODULE_SENSOR_CH_B_56R   (1 << 7)

static struct
{
    bool initialized;
    bc_tca9534a_t tca9534a;
    bc_module_sensor_revision_t revision;
    int onewire_init_semaphore;

} _bc_module_sensor;

static const bc_gpio_channel_t _bc_module_sensor_channel_gpio_lut[2] =
{
    [BC_MODULE_SENSOR_CHANNEL_A] = BC_GPIO_P4,
    [BC_MODULE_SENSOR_CHANNEL_B] = BC_GPIO_P5
};

static const bc_tca9534a_pin_t _bc_module_sensor_channel_virtual_4k7_lut[2] =
{
    [BC_MODULE_SENSOR_CHANNEL_A] = _BC_MODULE_SENSOR_CH_A_4K7,
    [BC_MODULE_SENSOR_CHANNEL_B] = _BC_MODULE_SENSOR_CH_B_4K7
};

static const bc_tca9534a_pin_t _bc_module_sensor_channel_virtual_56r_lut[2] =
{
    [BC_MODULE_SENSOR_CHANNEL_A] = _BC_MODULE_SENSOR_CH_A_56R,
    [BC_MODULE_SENSOR_CHANNEL_B] = _BC_MODULE_SENSOR_CH_B_56R
};

bool bc_module_sensor_init(void)
{
    if (!_bc_module_sensor.initialized)
    {
        if (!bc_tca9534a_init(&_bc_module_sensor.tca9534a, BC_I2C_I2C0, 0x3e))
        {
            return false;
        }

        if (!bc_tca9534a_write_port(&_bc_module_sensor.tca9534a, _BC_MODULE_SENSOR_INITIALIZED_STATE))
        {
            return false;
        }

        if (!bc_tca9534a_set_port_direction(&_bc_module_sensor.tca9534a, _BC_MODULE_SENSOR_INITIALIZED_DIRECTION))
        {
            return false;
        }

        bc_gpio_init(_bc_module_sensor_channel_gpio_lut[BC_MODULE_SENSOR_CHANNEL_A]);
        bc_gpio_init(_bc_module_sensor_channel_gpio_lut[BC_MODULE_SENSOR_CHANNEL_B]);

        _bc_module_sensor.initialized = true;
    }

    return true;
}

void bc_module_sensor_deinit(void)
{
    _bc_module_sensor.initialized = false;
}

bool bc_module_sensor_set_pull(bc_module_sensor_channel_t channel, bc_module_sensor_pull_t pull)
{
    uint8_t port_actual;
    uint8_t port_new;

    port_actual = _bc_module_sensor.tca9534a._output_port;

//    bc_tca9534a_read_port(&_bc_module_sensor.tca9534a, &port_actual);

    port_new = port_actual | _bc_module_sensor_channel_virtual_4k7_lut[channel] | _bc_module_sensor_channel_virtual_56r_lut[channel];

    switch (pull)
    {
        case BC_MODULE_SENSOR_PULL_NONE:
        {
            // Done by OR mask above

            bc_gpio_set_pull(_bc_module_sensor_channel_gpio_lut[channel], BC_GPIO_PULL_NONE);

            break;
        }
        case BC_MODULE_SENSOR_PULL_UP_4K7:
        {
            port_new &= ~_bc_module_sensor_channel_virtual_4k7_lut[channel];
            bc_gpio_set_pull(_bc_module_sensor_channel_gpio_lut[channel], BC_GPIO_PULL_NONE);

            break;
        }
        case BC_MODULE_SENSOR_PULL_UP_56R:
        {
            port_new &= ~_bc_module_sensor_channel_virtual_56r_lut[channel];
            bc_gpio_set_pull(_bc_module_sensor_channel_gpio_lut[channel], BC_GPIO_PULL_NONE);

            break;
        }
        case BC_MODULE_SENSOR_PULL_UP_INTERNAL:
        {
            bc_gpio_set_pull(_bc_module_sensor_channel_gpio_lut[channel], BC_GPIO_PULL_UP);
            break;
        }
        case BC_MODULE_SENSOR_PULL_DOWN_INTERNAL:
        {
            bc_gpio_set_pull(_bc_module_sensor_channel_gpio_lut[channel], BC_GPIO_PULL_DOWN);
            break;
        }
        default:
        {
            return false;
        }
    }

    if (port_actual != port_new)
    {
        return bc_tca9534a_write_port(&_bc_module_sensor.tca9534a, port_new);
    }

    return true;
}

bc_module_sensor_pull_t bc_module_sensor_get_pull(bc_module_sensor_channel_t channel)
{
    // TODO Implement better

    int weak;
    int strong;
    bc_gpio_pull_t internal;

    bc_tca9534a_read_pin(&_bc_module_sensor.tca9534a, _bc_module_sensor_channel_virtual_56r_lut[channel], &weak);
    bc_tca9534a_read_pin(&_bc_module_sensor.tca9534a, _bc_module_sensor_channel_virtual_4k7_lut[channel], &strong);
    internal = bc_gpio_get_pull(_bc_module_sensor_channel_gpio_lut[channel]);

    if (!weak)
    {
        return BC_MODULE_SENSOR_PULL_UP_4K7;
    }
    else if (!strong)
    {
        return BC_MODULE_SENSOR_PULL_UP_56R;
    }
    else if (internal == BC_GPIO_PULL_UP)
    {
        return BC_MODULE_SENSOR_PULL_UP_INTERNAL;
    }
    else if (internal == BC_GPIO_PULL_DOWN)
    {
        return BC_GPIO_PULL_DOWN;
    }
    else
    {
        return BC_GPIO_PULL_NONE;
    }
}

void bc_module_sensor_set_mode(bc_module_sensor_channel_t channel, bc_module_sensor_mode_t mode)
{
    bc_gpio_set_mode(_bc_module_sensor_channel_gpio_lut[channel], mode);
}

int bc_module_sensor_get_input(bc_module_sensor_channel_t channel)
{
    return bc_gpio_get_input(_bc_module_sensor_channel_gpio_lut[channel]);
}

void bc_module_sensor_set_output(bc_module_sensor_channel_t channel, int state)
{
    bc_gpio_set_output(_bc_module_sensor_channel_gpio_lut[channel], state);
}

int bc_module_sensor_get_output(bc_module_sensor_channel_t channel)
{
    return bc_gpio_get_output(_bc_module_sensor_channel_gpio_lut[channel]);
}

void bc_module_sensor_toggle_output(bc_module_sensor_channel_t channel)
{
    bc_gpio_toggle_output(_bc_module_sensor_channel_gpio_lut[channel]);
}

bool bc_module_sensor_set_vdd(bool on)
{
    uint8_t port_actual;
    uint8_t port_new;

    if (bc_module_sensor_get_revision() < BC_MODULE_SENSOR_REVISION_R1_1)
    {
        return false;
    }

    port_actual = _bc_module_sensor.tca9534a._output_port;

    if (on)
    {
        port_new = port_actual & ~_BC_MODULE_SENSOR_VDD;
    }
    else
    {
        port_new = port_actual | _BC_MODULE_SENSOR_VDD;
    }

    if (port_actual != port_new)
    {
        return bc_tca9534a_write_port(&_bc_module_sensor.tca9534a, port_new);
    }

    return true;
}

bc_module_sensor_revision_t bc_module_sensor_get_revision(void)
{
    if (!_bc_module_sensor.initialized)
    {
        return BC_MODULE_SENSOR_REVISION_UNKNOWN;
    }

    if (_bc_module_sensor.revision == BC_MODULE_SENSOR_REVISION_UNKNOWN)
    {
        if (!bc_tca9534a_set_pin_direction(&_bc_module_sensor.tca9534a, BC_TCA9534A_PIN_P1, BC_TCA9534A_PIN_DIRECTION_INPUT))
        {
            return BC_MODULE_SENSOR_REVISION_UNKNOWN;
        }

        uint8_t test_vector = 0x86;
        int value_input;
        int value_output;

        for (size_t i = 0; i < sizeof(test_vector); i++)
        {
            value_output = (test_vector >> i) & 0x01;

            if (!bc_tca9534a_write_pin(&_bc_module_sensor.tca9534a, BC_TCA9534A_PIN_P0, value_output))
            {
                return BC_MODULE_SENSOR_REVISION_UNKNOWN;
            }

            if (!bc_tca9534a_read_pin(&_bc_module_sensor.tca9534a, BC_TCA9534A_PIN_P1, &value_input))
            {
                return BC_MODULE_SENSOR_REVISION_UNKNOWN;
            }

            if (value_output != value_input)
            {
                _bc_module_sensor.revision = BC_MODULE_SENSOR_REVISION_R1_0;

                return _bc_module_sensor.revision;
            }
        }

        _bc_module_sensor.revision = BC_MODULE_SENSOR_REVISION_R1_1;
    }

    return _bc_module_sensor.revision;
}

void bc_module_sensor_onewire_init(void)
{
    bc_onewire_init(_bc_module_sensor_channel_gpio_lut[BC_MODULE_SENSOR_CHANNEL_B]);
}

bool bc_module_sensor_onewire_power_up(void)
{
    if (_bc_module_sensor.onewire_init_semaphore == 0)
    {
        if (bc_module_sensor_get_revision() == BC_MODULE_SENSOR_REVISION_R1_1)
        {
            if (!bc_module_sensor_set_vdd(1))
            {
                return false;
            }
        }
        else
        {
            if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_A, BC_MODULE_SENSOR_PULL_UP_56R))
            {
                return false;
            }
        }

        if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_B, BC_MODULE_SENSOR_PULL_UP_4K7))
        {
            return false;
        }
    }

    _bc_module_sensor.onewire_init_semaphore++;

    return true;
}

bool bc_module_sensor_onewire_power_down(void)
{
    _bc_module_sensor.onewire_init_semaphore--;

    if (_bc_module_sensor.onewire_init_semaphore == 0)
    {
        if (bc_module_sensor_get_revision() == BC_MODULE_SENSOR_REVISION_R1_1)
        {
            if (!bc_module_sensor_set_vdd(0))
            {
                return false;
            }
        }
        else
        {
            if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_A, BC_MODULE_SENSOR_PULL_NONE))
            {
                return false;
            }
        }

        if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_B, BC_MODULE_SENSOR_PULL_NONE))
        {
            return false;
        }
    }

    return true;
}
