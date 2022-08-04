#include <twr_module_sensor.h>
#include <twr_tca9534a.h>
#include <twr_onewire_gpio.h>
#include <twr_log.h>
#include <twr_error.h>

#define _TWR_MODULE_SENSOR_INITIALIZED_STATE 0xff
#define _TWR_MODULE_SENSOR_INITIALIZED_DIRECTION 0x00
#define _TWR_MODULE_SENSOR_VDD        (1 << 3)
#define _TWR_MODULE_SENSOR_CH_A_56R   (1 << 4)
#define _TWR_MODULE_SENSOR_CH_A_4K7   (1 << 5)
#define _TWR_MODULE_SENSOR_CH_B_4K7   (1 << 6)
#define _TWR_MODULE_SENSOR_CH_B_56R   (1 << 7)

static struct
{
    bool initialized;
    twr_tca9534a_t tca9534a;
    twr_module_sensor_revision_t revision;
    int onewire_power_semaphore;
    bool onewire_initialized;
    twr_onewire_t onewire;

} _twr_module_sensor = {
    .initialized = false,
    .onewire_initialized = false
};

static const twr_gpio_channel_t _twr_module_sensor_channel_gpio_lut[3] =
{
    [TWR_MODULE_SENSOR_CHANNEL_A] = TWR_GPIO_P4,
    [TWR_MODULE_SENSOR_CHANNEL_B] = TWR_GPIO_P5,
    [TWR_MODULE_SENSOR_CHANNEL_C] = TWR_GPIO_P7,
};

bool twr_module_sensor_init(void)
{
    if (!_twr_module_sensor.initialized)
    {
        if (!twr_tca9534a_init(&_twr_module_sensor.tca9534a, TWR_I2C_I2C0, 0x3e))
        {
            return false;
        }

        if (!twr_tca9534a_write_port(&_twr_module_sensor.tca9534a, _TWR_MODULE_SENSOR_INITIALIZED_STATE))
        {
            return false;
        }

        if (!twr_tca9534a_set_port_direction(&_twr_module_sensor.tca9534a, _TWR_MODULE_SENSOR_INITIALIZED_DIRECTION))
        {
            return false;
        }

        twr_gpio_init(_twr_module_sensor_channel_gpio_lut[TWR_MODULE_SENSOR_CHANNEL_A]);
        twr_gpio_init(_twr_module_sensor_channel_gpio_lut[TWR_MODULE_SENSOR_CHANNEL_B]);
        twr_gpio_init(_twr_module_sensor_channel_gpio_lut[TWR_MODULE_SENSOR_CHANNEL_C]);

        _twr_module_sensor.initialized = true;
    }

    return true;
}

void twr_module_sensor_deinit(void)
{
    _twr_module_sensor.initialized = false;
}

bool twr_module_sensor_set_pull(twr_module_sensor_channel_t channel, twr_module_sensor_pull_t pull)
{
    uint8_t port_actual;
    uint8_t port_new;

    port_actual = _twr_module_sensor.tca9534a._output_port;
//    twr_tca9534a_read_port(&_twr_module_sensor.tca9534a, &port_actual);

    port_new = port_actual;

    switch (channel)
    {
        case TWR_MODULE_SENSOR_CHANNEL_A:
        {
            port_new |= _TWR_MODULE_SENSOR_CH_A_4K7 | _TWR_MODULE_SENSOR_CH_A_56R;
            break;
        }
        case TWR_MODULE_SENSOR_CHANNEL_B:
        {
            port_new |= _TWR_MODULE_SENSOR_CH_B_4K7 | _TWR_MODULE_SENSOR_CH_B_56R;
            break;
        }
        case TWR_MODULE_SENSOR_CHANNEL_C:
        default:
        {
            break;
        }
    }

    switch (pull)
    {
        case TWR_MODULE_SENSOR_PULL_NONE:
        {
            // Done by OR mask above

            twr_gpio_set_pull(_twr_module_sensor_channel_gpio_lut[channel], TWR_GPIO_PULL_NONE);

            break;
        }
        case TWR_MODULE_SENSOR_PULL_UP_4K7:
        {
            switch (channel)
            {
                case TWR_MODULE_SENSOR_CHANNEL_A:
                {
                    port_new &= ~_TWR_MODULE_SENSOR_CH_A_4K7;
                    break;
                }
                case TWR_MODULE_SENSOR_CHANNEL_B:
                {
                    port_new &= ~_TWR_MODULE_SENSOR_CH_B_4K7;
                    break;
                }
                case TWR_MODULE_SENSOR_CHANNEL_C:
                default:
                {
                    return false;
                }
            }
            break;
        }
        case TWR_MODULE_SENSOR_PULL_UP_56R:
        {
            switch (channel)
            {
                case TWR_MODULE_SENSOR_CHANNEL_A:
                {
                    port_new &= ~_TWR_MODULE_SENSOR_CH_A_56R;
                    break;
                }
                case TWR_MODULE_SENSOR_CHANNEL_B:
                {
                    port_new &= ~_TWR_MODULE_SENSOR_CH_B_56R;
                    break;
                }
                case TWR_MODULE_SENSOR_CHANNEL_C:
                default:
                {
                    return false;
                }
            }
            break;
        }
        case TWR_MODULE_SENSOR_PULL_UP_INTERNAL:
        {
            twr_gpio_set_pull(_twr_module_sensor_channel_gpio_lut[channel], TWR_GPIO_PULL_UP);
            break;
        }
        case TWR_MODULE_SENSOR_PULL_DOWN_INTERNAL:
        {
            twr_gpio_set_pull(_twr_module_sensor_channel_gpio_lut[channel], TWR_GPIO_PULL_DOWN);
            break;
        }
        default:
        {
            return false;
        }
    }

    if (port_actual != port_new)
    {
        twr_gpio_set_pull(_twr_module_sensor_channel_gpio_lut[channel], TWR_GPIO_PULL_NONE);
        return twr_tca9534a_write_port(&_twr_module_sensor.tca9534a, port_new);
    }

    return true;
}

twr_module_sensor_pull_t twr_module_sensor_get_pull(twr_module_sensor_channel_t channel)
{
    if (channel == TWR_MODULE_SENSOR_CHANNEL_A)
    {
        uint8_t port_actual = _twr_module_sensor.tca9534a._output_port;

        if ((port_actual & _TWR_MODULE_SENSOR_CH_A_4K7) == 0)
        {
            return TWR_MODULE_SENSOR_PULL_UP_4K7;
        }

        if ((port_actual & _TWR_MODULE_SENSOR_CH_A_56R) == 0)
        {
            return TWR_MODULE_SENSOR_PULL_UP_56R;
        }
    }
    if (channel == TWR_MODULE_SENSOR_CHANNEL_B)
    {
        uint8_t port_actual = _twr_module_sensor.tca9534a._output_port;

        if ((port_actual & _TWR_MODULE_SENSOR_CH_B_4K7) == 0)
        {
            return TWR_MODULE_SENSOR_PULL_UP_4K7;
        }

        if ((port_actual & _TWR_MODULE_SENSOR_CH_B_56R) == 0)
        {
            return TWR_MODULE_SENSOR_PULL_UP_56R;
        }
    }

    return (twr_module_sensor_pull_t) twr_gpio_get_pull(_twr_module_sensor_channel_gpio_lut[channel]);
}

void twr_module_sensor_set_mode(twr_module_sensor_channel_t channel, twr_module_sensor_mode_t mode)
{
    twr_gpio_set_mode(_twr_module_sensor_channel_gpio_lut[channel], (twr_gpio_mode_t) mode);
}

int twr_module_sensor_get_input(twr_module_sensor_channel_t channel)
{
    return twr_gpio_get_input(_twr_module_sensor_channel_gpio_lut[channel]);
}

void twr_module_sensor_set_output(twr_module_sensor_channel_t channel, int state)
{
    twr_gpio_set_output(_twr_module_sensor_channel_gpio_lut[channel], state);
}

int twr_module_sensor_get_output(twr_module_sensor_channel_t channel)
{
    return twr_gpio_get_output(_twr_module_sensor_channel_gpio_lut[channel]);
}

void twr_module_sensor_toggle_output(twr_module_sensor_channel_t channel)
{
    twr_gpio_toggle_output(_twr_module_sensor_channel_gpio_lut[channel]);
}

bool twr_module_sensor_set_vdd(bool on)
{
    uint8_t port_actual;
    uint8_t port_new;

    if (twr_module_sensor_get_revision() < TWR_MODULE_SENSOR_REVISION_R1_1)
    {
        return false;
    }

    port_actual = _twr_module_sensor.tca9534a._output_port;

    if (on)
    {
        port_new = port_actual & ~_TWR_MODULE_SENSOR_VDD;
    }
    else
    {
        port_new = port_actual | _TWR_MODULE_SENSOR_VDD;
    }

    if (port_actual != port_new)
    {
        return twr_tca9534a_write_port(&_twr_module_sensor.tca9534a, port_new);
    }

    return true;
}

twr_module_sensor_revision_t twr_module_sensor_get_revision(void)
{
    if (!_twr_module_sensor.initialized)
    {
        return TWR_MODULE_SENSOR_REVISION_UNKNOWN;
    }

    if (_twr_module_sensor.revision == TWR_MODULE_SENSOR_REVISION_UNKNOWN)
    {
        if (!twr_tca9534a_set_pin_direction(&_twr_module_sensor.tca9534a, TWR_TCA9534A_PIN_P1, TWR_TCA9534A_PIN_DIRECTION_INPUT))
        {
            return TWR_MODULE_SENSOR_REVISION_UNKNOWN;
        }

        uint8_t test_vector = 0x86;
        int value_input;
        int value_output;

        for (size_t i = 0; i < sizeof(test_vector); i++)
        {
            value_output = (test_vector >> i) & 0x01;

            if (!twr_tca9534a_write_pin(&_twr_module_sensor.tca9534a, TWR_TCA9534A_PIN_P0, value_output))
            {
                return TWR_MODULE_SENSOR_REVISION_UNKNOWN;
            }

            if (!twr_tca9534a_read_pin(&_twr_module_sensor.tca9534a, TWR_TCA9534A_PIN_P1, &value_input))
            {
                return TWR_MODULE_SENSOR_REVISION_UNKNOWN;
            }

            if (value_output != value_input)
            {
                _twr_module_sensor.revision = TWR_MODULE_SENSOR_REVISION_R1_0;

                return _twr_module_sensor.revision;
            }
        }

        _twr_module_sensor.revision = TWR_MODULE_SENSOR_REVISION_R1_1;
    }

    return _twr_module_sensor.revision;
}

twr_onewire_t *twr_module_sensor_get_onewire(void)
{
    if (!_twr_module_sensor.onewire_initialized)
    {
        twr_onewire_gpio_init(&_twr_module_sensor.onewire, TWR_GPIO_P5);

        _twr_module_sensor.onewire_initialized = true;
    }

    return &_twr_module_sensor.onewire;
}

bool twr_module_sensor_onewire_power_up(void)
{
    if (++_twr_module_sensor.onewire_power_semaphore == 1)
    {
        switch (twr_module_sensor_get_revision())
        {
            case TWR_MODULE_SENSOR_REVISION_R1_1:
            {
                if (!twr_module_sensor_set_vdd(1))
                {
                    return false;
                }
                break;
            }
            case TWR_MODULE_SENSOR_REVISION_R1_0:
            {
                twr_gpio_set_mode(_twr_module_sensor_channel_gpio_lut[TWR_MODULE_SENSOR_CHANNEL_A], TWR_GPIO_MODE_ANALOG);

                if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_A, TWR_MODULE_SENSOR_PULL_UP_56R))
                {
                    return false;
                }
                break;
            }
            case TWR_MODULE_SENSOR_REVISION_UNKNOWN:
            default:
            {
                return false;
            }
        }

        if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_B, TWR_MODULE_SENSOR_PULL_UP_4K7))
        {
            return false;
        }
    }
    return true;
}

bool twr_module_sensor_onewire_power_down(void)
{
    if (_twr_module_sensor.onewire_power_semaphore < 1) twr_error(TWR_ERROR_ERROR_UNLOCK);

    if (--_twr_module_sensor.onewire_power_semaphore == 0)
    {
        switch (twr_module_sensor_get_revision())
        {
            case TWR_MODULE_SENSOR_REVISION_R1_1:
            {
                if (!twr_module_sensor_set_vdd(0))
                {
                    return false;
                }
                break;
            }
            case TWR_MODULE_SENSOR_REVISION_R1_0:
            {
                if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_A, TWR_MODULE_SENSOR_PULL_NONE))
                {
                    return false;
                }
                break;
            }
            case TWR_MODULE_SENSOR_REVISION_UNKNOWN:
            default:
            {
                return false;
            }
        }

        if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_B, TWR_MODULE_SENSOR_PULL_NONE))
        {
            return false;
        }
    }

    return true;
}
