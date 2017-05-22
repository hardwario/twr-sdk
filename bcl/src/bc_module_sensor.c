#include <bc_module_sensor.h>
#include <bc_tca9534a.h>

#define _BC_MODULE_SENSOR_CH_A_VDD   (1 << 4)
#define _BC_MODULE_SENSOR_CH_A_EN    (1 << 5)
#define _BC_MODULE_SENSOR_CH_B_EN    (1 << 6)
#define _BC_MODULE_SENSOR_CH_B_VDD   (1 << 7)

static struct
{
    bc_tca9534a_t tca9534a;
    uint8_t direction;

} _bc_module_sensor;


bool bc_module_sensor_init(void)
{
    if (!bc_tca9534a_init(&_bc_module_sensor.tca9534a, BC_I2C_I2C0, 0x3e))
    {
        return false;
    }

    if (!bc_tca9534a_write_port(&_bc_module_sensor.tca9534a, 0x00))
    {
        return false;
    }

    _bc_module_sensor.direction = 0xff;

    if (!bc_tca9534a_set_port_direction(&_bc_module_sensor.tca9534a, _bc_module_sensor.direction))
    {
        return false;
    }

    return true;
}

bool bc_module_sensor_set_pull(bc_module_channel_t channel, bc_module_pull_t pull)
{
    if (channel == BC_MODULE_SENSOR_CHANNEL_A)
    {
        _bc_module_sensor.direction |= _BC_MODULE_SENSOR_CH_A_VDD | _BC_MODULE_SENSOR_CH_A_EN;

        if (pull == BC_MODULE_PULL_4K7)
        {
            _bc_module_sensor.direction &= ~_BC_MODULE_SENSOR_CH_A_EN;
        }
        else if (pull == BC_MODULE_PULL_56)
        {
            _bc_module_sensor.direction &= ~_BC_MODULE_SENSOR_CH_A_VDD;
        }
    }
    else
    {
        _bc_module_sensor.direction |= _BC_MODULE_SENSOR_CH_B_EN | _BC_MODULE_SENSOR_CH_B_VDD;

        if (pull == BC_MODULE_PULL_4K7)
        {
            _bc_module_sensor.direction &= ~_BC_MODULE_SENSOR_CH_B_EN;
        }
        else if (pull == BC_MODULE_PULL_56)
        {
            _bc_module_sensor.direction &= ~_BC_MODULE_SENSOR_CH_B_VDD;
        }
    }

    return bc_tca9534a_set_port_direction(&_bc_module_sensor.tca9534a, _bc_module_sensor.direction);
}
