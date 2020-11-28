#include <hio_module_sensor.h>
#include <hio_tca9534a.h>
#include <hio_onewire_gpio.h>

#define _HIO_MODULE_SENSOR_INITIALIZED_STATE 0xff
#define _HIO_MODULE_SENSOR_INITIALIZED_DIRECTION 0x00
#define _HIO_MODULE_SENSOR_VDD        (1 << 3)
#define _HIO_MODULE_SENSOR_CH_A_56R   (1 << 4)
#define _HIO_MODULE_SENSOR_CH_A_4K7   (1 << 5)
#define _HIO_MODULE_SENSOR_CH_B_4K7   (1 << 6)
#define _HIO_MODULE_SENSOR_CH_B_56R   (1 << 7)

static struct
{
    bool initialized;
    hio_tca9534a_t tca9534a;
    hio_module_sensor_revision_t revision;
    int onewire_power_semaphore;
    bool onewire_initialized;
    hio_onewire_t onewire;

} _hio_module_sensor = {
    .initialized = false,
    .onewire_initialized = false
};

static const hio_gpio_channel_t _hio_module_sensor_channel_gpio_lut[3] =
{
    [HIO_MODULE_SENSOR_CHANNEL_A] = HIO_GPIO_P4,
    [HIO_MODULE_SENSOR_CHANNEL_B] = HIO_GPIO_P5,
    [HIO_MODULE_SENSOR_CHANNEL_C] = HIO_GPIO_P7,
};

bool hio_module_sensor_init(void)
{
    if (!_hio_module_sensor.initialized)
    {
        if (!hio_tca9534a_init(&_hio_module_sensor.tca9534a, HIO_I2C_I2C0, 0x3e))
        {
            return false;
        }

        if (!hio_tca9534a_write_port(&_hio_module_sensor.tca9534a, _HIO_MODULE_SENSOR_INITIALIZED_STATE))
        {
            return false;
        }

        if (!hio_tca9534a_set_port_direction(&_hio_module_sensor.tca9534a, _HIO_MODULE_SENSOR_INITIALIZED_DIRECTION))
        {
            return false;
        }

        hio_gpio_init(_hio_module_sensor_channel_gpio_lut[HIO_MODULE_SENSOR_CHANNEL_A]);
        hio_gpio_init(_hio_module_sensor_channel_gpio_lut[HIO_MODULE_SENSOR_CHANNEL_B]);
        hio_gpio_init(_hio_module_sensor_channel_gpio_lut[HIO_MODULE_SENSOR_CHANNEL_C]);

        _hio_module_sensor.initialized = true;
    }

    return true;
}

void hio_module_sensor_deinit(void)
{
    _hio_module_sensor.initialized = false;
}

bool hio_module_sensor_set_pull(hio_module_sensor_channel_t channel, hio_module_sensor_pull_t pull)
{
    uint8_t port_actual;
    uint8_t port_new;

    port_actual = _hio_module_sensor.tca9534a._output_port;
//    hio_tca9534a_read_port(&_hio_module_sensor.tca9534a, &port_actual);

    port_new = port_actual;

    switch (channel)
    {
        case HIO_MODULE_SENSOR_CHANNEL_A:
        {
            port_new |= _HIO_MODULE_SENSOR_CH_A_4K7 | _HIO_MODULE_SENSOR_CH_A_56R;
            break;
        }
        case HIO_MODULE_SENSOR_CHANNEL_B:
        {
            port_new |= _HIO_MODULE_SENSOR_CH_B_4K7 | _HIO_MODULE_SENSOR_CH_B_56R;
            break;
        }
        case HIO_MODULE_SENSOR_CHANNEL_C:
        default:
        {
            break;
        }
    }

    switch (pull)
    {
        case HIO_MODULE_SENSOR_PULL_NONE:
        {
            // Done by OR mask above

            hio_gpio_set_pull(_hio_module_sensor_channel_gpio_lut[channel], HIO_GPIO_PULL_NONE);

            break;
        }
        case HIO_MODULE_SENSOR_PULL_UP_4K7:
        {
            switch (channel)
            {
                case HIO_MODULE_SENSOR_CHANNEL_A:
                {
                    port_new &= ~_HIO_MODULE_SENSOR_CH_A_4K7;
                    break;
                }
                case HIO_MODULE_SENSOR_CHANNEL_B:
                {
                    port_new &= ~_HIO_MODULE_SENSOR_CH_B_4K7;
                    break;
                }
                case HIO_MODULE_SENSOR_CHANNEL_C:
                default:
                {
                    return false;
                }
            }
            break;
        }
        case HIO_MODULE_SENSOR_PULL_UP_56R:
        {
            switch (channel)
            {
                case HIO_MODULE_SENSOR_CHANNEL_A:
                {
                    port_new &= ~_HIO_MODULE_SENSOR_CH_A_56R;
                    break;
                }
                case HIO_MODULE_SENSOR_CHANNEL_B:
                {
                    port_new &= ~_HIO_MODULE_SENSOR_CH_B_56R;
                    break;
                }
                case HIO_MODULE_SENSOR_CHANNEL_C:
                default:
                {
                    return false;
                }
            }
            break;
        }
        case HIO_MODULE_SENSOR_PULL_UP_INTERNAL:
        {
            hio_gpio_set_pull(_hio_module_sensor_channel_gpio_lut[channel], HIO_GPIO_PULL_UP);
            break;
        }
        case HIO_MODULE_SENSOR_PULL_DOWN_INTERNAL:
        {
            hio_gpio_set_pull(_hio_module_sensor_channel_gpio_lut[channel], HIO_GPIO_PULL_DOWN);
            break;
        }
        default:
        {
            return false;
        }
    }

    if (port_actual != port_new)
    {
        hio_gpio_set_pull(_hio_module_sensor_channel_gpio_lut[channel], HIO_GPIO_PULL_NONE);
        return hio_tca9534a_write_port(&_hio_module_sensor.tca9534a, port_new);
    }

    return true;
}

hio_module_sensor_pull_t hio_module_sensor_get_pull(hio_module_sensor_channel_t channel)
{
    if (channel == HIO_MODULE_SENSOR_CHANNEL_A)
    {
        uint8_t port_actual = _hio_module_sensor.tca9534a._output_port;

        if ((port_actual & _HIO_MODULE_SENSOR_CH_A_4K7) == 0)
        {
            return HIO_MODULE_SENSOR_PULL_UP_4K7;
        }

        if ((port_actual & _HIO_MODULE_SENSOR_CH_A_56R) == 0)
        {
            return HIO_MODULE_SENSOR_PULL_UP_56R;
        }
    }
    if (channel == HIO_MODULE_SENSOR_CHANNEL_B)
    {
        uint8_t port_actual = _hio_module_sensor.tca9534a._output_port;

        if ((port_actual & _HIO_MODULE_SENSOR_CH_B_4K7) == 0)
        {
            return HIO_MODULE_SENSOR_PULL_UP_4K7;
        }

        if ((port_actual & _HIO_MODULE_SENSOR_CH_B_56R) == 0)
        {
            return HIO_MODULE_SENSOR_PULL_UP_56R;
        }
    }

    return hio_gpio_get_pull(_hio_module_sensor_channel_gpio_lut[channel]);
}

void hio_module_sensor_set_mode(hio_module_sensor_channel_t channel, hio_module_sensor_mode_t mode)
{
    hio_gpio_set_mode(_hio_module_sensor_channel_gpio_lut[channel], mode);
}

int hio_module_sensor_get_input(hio_module_sensor_channel_t channel)
{
    return hio_gpio_get_input(_hio_module_sensor_channel_gpio_lut[channel]);
}

void hio_module_sensor_set_output(hio_module_sensor_channel_t channel, int state)
{
    hio_gpio_set_output(_hio_module_sensor_channel_gpio_lut[channel], state);
}

int hio_module_sensor_get_output(hio_module_sensor_channel_t channel)
{
    return hio_gpio_get_output(_hio_module_sensor_channel_gpio_lut[channel]);
}

void hio_module_sensor_toggle_output(hio_module_sensor_channel_t channel)
{
    hio_gpio_toggle_output(_hio_module_sensor_channel_gpio_lut[channel]);
}

bool hio_module_sensor_set_vdd(bool on)
{
    uint8_t port_actual;
    uint8_t port_new;

    if (hio_module_sensor_get_revision() < HIO_MODULE_SENSOR_REVISION_R1_1)
    {
        return false;
    }

    port_actual = _hio_module_sensor.tca9534a._output_port;

    if (on)
    {
        port_new = port_actual & ~_HIO_MODULE_SENSOR_VDD;
    }
    else
    {
        port_new = port_actual | _HIO_MODULE_SENSOR_VDD;
    }

    if (port_actual != port_new)
    {
        return hio_tca9534a_write_port(&_hio_module_sensor.tca9534a, port_new);
    }

    return true;
}

hio_module_sensor_revision_t hio_module_sensor_get_revision(void)
{
    if (!_hio_module_sensor.initialized)
    {
        return HIO_MODULE_SENSOR_REVISION_UNKNOWN;
    }

    if (_hio_module_sensor.revision == HIO_MODULE_SENSOR_REVISION_UNKNOWN)
    {
        if (!hio_tca9534a_set_pin_direction(&_hio_module_sensor.tca9534a, HIO_TCA9534A_PIN_P1, HIO_TCA9534A_PIN_DIRECTION_INPUT))
        {
            return HIO_MODULE_SENSOR_REVISION_UNKNOWN;
        }

        uint8_t test_vector = 0x86;
        int value_input;
        int value_output;

        for (size_t i = 0; i < sizeof(test_vector); i++)
        {
            value_output = (test_vector >> i) & 0x01;

            if (!hio_tca9534a_write_pin(&_hio_module_sensor.tca9534a, HIO_TCA9534A_PIN_P0, value_output))
            {
                return HIO_MODULE_SENSOR_REVISION_UNKNOWN;
            }

            if (!hio_tca9534a_read_pin(&_hio_module_sensor.tca9534a, HIO_TCA9534A_PIN_P1, &value_input))
            {
                return HIO_MODULE_SENSOR_REVISION_UNKNOWN;
            }

            if (value_output != value_input)
            {
                _hio_module_sensor.revision = HIO_MODULE_SENSOR_REVISION_R1_0;

                return _hio_module_sensor.revision;
            }
        }

        _hio_module_sensor.revision = HIO_MODULE_SENSOR_REVISION_R1_1;
    }

    return _hio_module_sensor.revision;
}

hio_onewire_t *hio_module_sensor_get_onewire(void)
{
    if (!_hio_module_sensor.onewire_initialized)
    {
        hio_onewire_gpio_init(&_hio_module_sensor.onewire, HIO_GPIO_P5);

        _hio_module_sensor.onewire_initialized = true;
    }

    return &_hio_module_sensor.onewire;
}

bool hio_module_sensor_onewire_power_up(void)
{
    if (_hio_module_sensor.onewire_power_semaphore == 0)
    {
        if (hio_module_sensor_get_revision() == HIO_MODULE_SENSOR_REVISION_R1_1)
        {
            if (!hio_module_sensor_set_vdd(1))
            {
                return false;
            }
        }
        else
        {
            if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_A, HIO_MODULE_SENSOR_PULL_UP_56R))
            {
                return false;
            }
        }

        if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_B, HIO_MODULE_SENSOR_PULL_UP_4K7))
        {
            return false;
        }
    }

    _hio_module_sensor.onewire_power_semaphore++;

    return true;
}

bool hio_module_sensor_onewire_power_down(void)
{
    _hio_module_sensor.onewire_power_semaphore--;

    if (_hio_module_sensor.onewire_power_semaphore == 0)
    {
        if (hio_module_sensor_get_revision() == HIO_MODULE_SENSOR_REVISION_R1_1)
        {
            if (!hio_module_sensor_set_vdd(0))
            {
                return false;
            }
        }
        else
        {
            if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_A, HIO_MODULE_SENSOR_PULL_NONE))
            {
                return false;
            }
        }

        if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_B, HIO_MODULE_SENSOR_PULL_NONE))
        {
            return false;
        }
    }

    return true;
}
