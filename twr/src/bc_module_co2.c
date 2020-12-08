#include <twr_module_co2.h>
#include <twr_scheduler.h>
#include <twr_i2c.h>
#include <twr_tca9534a.h>
#include <twr_sc16is740.h>

#define _TWR_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS 0x38
#define _TWR_MODULE_CO2_I2C_UART_ADDRESS 0x4d
#define _TWR_MODULE_CO2_PIN_DEFAULT (~(1 << 0) & ~(1 << 4))
#define _TWR_MODULE_CO2_PIN_CAP_ON (~(1 << 1))
#define _TWR_MODULE_CO2_PIN_VDD2_ON (~(1 << 2))
#define _TWR_MODULE_CO2_PIN_EN (~(1 << 3))
#define _TWR_MODULE_CO2_PIN_UART_RESET (~(1 << 6))
#define _TWR_MODULE_CO2_PIN_RDY TWR_TCA9534A_PIN_P7

typedef enum
{
    TWR_MODULE_CO2_STATE_ERROR = -1,
    TWR_MODULE_CO2_STATE_INITIALIZE = 0,
    TWR_MODULE_CO2_STATE_INITIALIZE1,
    TWR_MODULE_CO2_STATE_INITIALIZE2,
    TWR_MODULE_CO2_STATE_READY,
    TWR_MODULE_CO2_STATE_CHARGE,
    TWR_MODULE_CO2_STATE_BOOT,
    TWR_MODULE_CO2_STATE_BOOT_READ,
    TWR_MODULE_CO2_STATE_MEASURE,
    TWR_MODULE_CO2_STATE_MEASURE_READ,

} twr_module_co2_state_t;

static bool _twr_module_co2_init(void);
static bool _twr_module_co2_charge_enable(bool state);
static bool _twr_module_co2_device_enable(bool state);
static bool _twr_module_co2_read_signal_rdy(int *value);
static bool _twr_module_co2_uart_enable(bool state);
static size_t _twr_module_co2_uart_write(uint8_t *buffer, size_t length);
static size_t _twr_module_co2_uart_read(uint8_t *buffer, size_t length);

static struct
{
    twr_tca9534a_t tca9534a;
    twr_sc16is740_t sc16is740;
    twr_lp8_t sensor;
    const twr_lp8_driver_t driver;

} _twr_module_co2 = {
    .driver = {
        .init = _twr_module_co2_init,
        .charge_enable = _twr_module_co2_charge_enable,
        .device_enable = _twr_module_co2_device_enable,
        .read_signal_rdy = _twr_module_co2_read_signal_rdy,
        .uart_enable = _twr_module_co2_uart_enable,
        .uart_write = _twr_module_co2_uart_write,
        .uart_read = _twr_module_co2_uart_read
    }
};

void twr_module_co2_init(void)
{
    twr_lp8_init(&_twr_module_co2.sensor, &_twr_module_co2.driver);
}

void twr_module_co2_set_event_handler(void (*event_handler)(twr_module_co2_event_t, void *), void *event_param)
{
    twr_lp8_set_event_handler(&_twr_module_co2.sensor, (void (*)(twr_lp8_event_t, void *)) event_handler, event_param);
}

void twr_module_co2_set_update_interval(twr_tick_t interval)
{
    twr_lp8_set_update_interval(&_twr_module_co2.sensor, interval);
}

bool twr_module_co2_measure(void)
{
    return twr_lp8_measure(&_twr_module_co2.sensor);
}

bool twr_module_co2_get_concentration_ppm(float *ppm)
{
    return twr_lp8_get_concentration_ppm(&_twr_module_co2.sensor, ppm);
}

bool twr_module_co2_get_error(twr_lp8_error_t *error)
{
    return twr_lp8_get_error(&_twr_module_co2.sensor, error);
}

void twr_module_co2_calibration(twr_lp8_calibration_t calibration)
{
    twr_lp8_calibration(&_twr_module_co2.sensor, calibration);
}

static bool _twr_module_co2_init(void)
{
    if (!twr_tca9534a_init(&_twr_module_co2.tca9534a, TWR_I2C_I2C0, _TWR_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS))
    {
        return false;
    }

    if (!twr_tca9534a_write_port(&_twr_module_co2.tca9534a, 0x00))
    {
        return false;
    }

    // Reset SC16IS740
    if (!twr_tca9534a_set_port_direction(&_twr_module_co2.tca9534a, _TWR_MODULE_CO2_PIN_DEFAULT & _TWR_MODULE_CO2_PIN_UART_RESET))
    {
        return false;
    }

    // Reset pulse width > 3us
    for (volatile int i = 0; i < 100; i++)
    {
        continue;
    }

    if (!twr_tca9534a_set_port_direction(&_twr_module_co2.tca9534a, _TWR_MODULE_CO2_PIN_DEFAULT))
    {
        return false;
    }

    // Delay time width > 10us
    for (volatile int i = 0; i < 1000; i++)
    {
        continue;
    }

    if (!twr_sc16is740_init(&_twr_module_co2.sc16is740, TWR_I2C_I2C0, _TWR_MODULE_CO2_I2C_UART_ADDRESS))
    {
        return false;
    }

    return true;
}

static bool _twr_module_co2_charge_enable(bool state)
{
    uint8_t direction = _TWR_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _TWR_MODULE_CO2_PIN_VDD2_ON & _TWR_MODULE_CO2_PIN_CAP_ON;
    }

    return twr_tca9534a_set_port_direction(&_twr_module_co2.tca9534a, direction);
}

static bool _twr_module_co2_device_enable(bool state)
{
    uint8_t direction = _TWR_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _TWR_MODULE_CO2_PIN_VDD2_ON & _TWR_MODULE_CO2_PIN_CAP_ON & _TWR_MODULE_CO2_PIN_EN;
    }

    return twr_tca9534a_set_port_direction(&_twr_module_co2.tca9534a, direction);
}

static bool _twr_module_co2_read_signal_rdy(int *value)
{
    if (!twr_tca9534a_read_pin(&_twr_module_co2.tca9534a, _TWR_MODULE_CO2_PIN_RDY, value))
    {
        return false;
    }

    return true;
}

static bool _twr_module_co2_uart_enable(bool state)
{
    if (state)
    {
        return twr_sc16is740_reset_fifo(&_twr_module_co2.sc16is740, TWR_SC16IS740_FIFO_RX);
    }

    return true;
}

static size_t _twr_module_co2_uart_write(uint8_t *buffer, size_t length)
{
    return twr_sc16is740_write(&_twr_module_co2.sc16is740, buffer, length);
}

static size_t _twr_module_co2_uart_read(uint8_t *buffer, size_t length)
{
    return twr_sc16is740_read(&_twr_module_co2.sc16is740, buffer, length, 0);
}
