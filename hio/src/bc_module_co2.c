#include <bc_module_co2.h>
#include <bc_scheduler.h>
#include <bc_i2c.h>
#include <bc_tca9534a.h>
#include <bc_sc16is740.h>

#define _BC_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS 0x38
#define _BC_MODULE_CO2_I2C_UART_ADDRESS 0x4d
#define _BC_MODULE_CO2_PIN_DEFAULT (~(1 << 0) & ~(1 << 4))
#define _BC_MODULE_CO2_PIN_CAP_ON (~(1 << 1))
#define _BC_MODULE_CO2_PIN_VDD2_ON (~(1 << 2))
#define _BC_MODULE_CO2_PIN_EN (~(1 << 3))
#define _BC_MODULE_CO2_PIN_UART_RESET (~(1 << 6))
#define _BC_MODULE_CO2_PIN_RDY BC_TCA9534A_PIN_P7

typedef enum
{
    BC_MODULE_CO2_STATE_ERROR = -1,
    BC_MODULE_CO2_STATE_INITIALIZE = 0,
    BC_MODULE_CO2_STATE_INITIALIZE1,
    BC_MODULE_CO2_STATE_INITIALIZE2,
    BC_MODULE_CO2_STATE_READY,
    BC_MODULE_CO2_STATE_CHARGE,
    BC_MODULE_CO2_STATE_BOOT,
    BC_MODULE_CO2_STATE_BOOT_READ,
    BC_MODULE_CO2_STATE_MEASURE,
    BC_MODULE_CO2_STATE_MEASURE_READ,

} bc_module_co2_state_t;

static bool _bc_module_co2_init(void);
static bool _bc_module_co2_charge_enable(bool state);
static bool _bc_module_co2_device_enable(bool state);
static bool _bc_module_co2_read_signal_rdy(int *value);
static bool _bc_module_co2_uart_enable(bool state);
static size_t _bc_module_co2_uart_write(uint8_t *buffer, size_t length);
static size_t _bc_module_co2_uart_read(uint8_t *buffer, size_t length);

static struct
{
    bc_tca9534a_t tca9534a;
    bc_sc16is740_t sc16is740;
    bc_lp8_t sensor;
    const bc_lp8_driver_t driver;

} _bc_module_co2 = {
    .driver = {
        .init = _bc_module_co2_init,
        .charge_enable = _bc_module_co2_charge_enable,
        .device_enable = _bc_module_co2_device_enable,
        .read_signal_rdy = _bc_module_co2_read_signal_rdy,
        .uart_enable = _bc_module_co2_uart_enable,
        .uart_write = _bc_module_co2_uart_write,
        .uart_read = _bc_module_co2_uart_read
    }
};

void bc_module_co2_init(void)
{
    bc_lp8_init(&_bc_module_co2.sensor, &_bc_module_co2.driver);
}

void bc_module_co2_set_event_handler(void (*event_handler)(bc_module_co2_event_t, void *), void *event_param)
{
    bc_lp8_set_event_handler(&_bc_module_co2.sensor, (void (*)(bc_lp8_event_t, void *)) event_handler, event_param);
}

void bc_module_co2_set_update_interval(bc_tick_t interval)
{
    bc_lp8_set_update_interval(&_bc_module_co2.sensor, interval);
}

bool bc_module_co2_measure(void)
{
    return bc_lp8_measure(&_bc_module_co2.sensor);
}

bool bc_module_co2_get_concentration_ppm(float *ppm)
{
    return bc_lp8_get_concentration_ppm(&_bc_module_co2.sensor, ppm);
}

bool bc_module_co2_get_error(bc_lp8_error_t *error)
{
    return bc_lp8_get_error(&_bc_module_co2.sensor, error);
}

void bc_module_co2_calibration(bc_lp8_calibration_t calibration)
{
    bc_lp8_calibration(&_bc_module_co2.sensor, calibration);
}

static bool _bc_module_co2_init(void)
{
    if (!bc_tca9534a_init(&_bc_module_co2.tca9534a, BC_I2C_I2C0, _BC_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS))
    {
        return false;
    }

    if (!bc_tca9534a_write_port(&_bc_module_co2.tca9534a, 0x00))
    {
        return false;
    }

    // Reset SC16IS740
    if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT & _BC_MODULE_CO2_PIN_UART_RESET))
    {
        return false;
    }

    // Reset pulse width > 3us
    for (volatile int i = 0; i < 100; i++)
    {
        continue;
    }

    if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT))
    {
        return false;
    }

    // Delay time width > 10us
    for (volatile int i = 0; i < 1000; i++)
    {
        continue;
    }

    if (!bc_sc16is740_init(&_bc_module_co2.sc16is740, BC_I2C_I2C0, _BC_MODULE_CO2_I2C_UART_ADDRESS))
    {
        return false;
    }

    return true;
}

static bool _bc_module_co2_charge_enable(bool state)
{
    uint8_t direction = _BC_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _BC_MODULE_CO2_PIN_VDD2_ON & _BC_MODULE_CO2_PIN_CAP_ON;
    }

    return bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, direction);
}

static bool _bc_module_co2_device_enable(bool state)
{
    uint8_t direction = _BC_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _BC_MODULE_CO2_PIN_VDD2_ON & _BC_MODULE_CO2_PIN_CAP_ON & _BC_MODULE_CO2_PIN_EN;
    }

    return bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, direction);
}

static bool _bc_module_co2_read_signal_rdy(int *value)
{
    if (!bc_tca9534a_read_pin(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_RDY, value))
    {
        return false;
    }

    return true;
}

static bool _bc_module_co2_uart_enable(bool state)
{
    if (state)
    {
        return bc_sc16is740_reset_fifo(&_bc_module_co2.sc16is740, BC_SC16IS740_FIFO_RX);
    }

    return true;
}

static size_t _bc_module_co2_uart_write(uint8_t *buffer, size_t length)
{
    return bc_sc16is740_write(&_bc_module_co2.sc16is740, buffer, length);
}

static size_t _bc_module_co2_uart_read(uint8_t *buffer, size_t length)
{
    return bc_sc16is740_read(&_bc_module_co2.sc16is740, buffer, length, 0);
}
