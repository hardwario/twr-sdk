#include <hio_module_co2.h>
#include <hio_scheduler.h>
#include <hio_i2c.h>
#include <hio_tca9534a.h>
#include <hio_sc16is740.h>

#define _HIO_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS 0x38
#define _HIO_MODULE_CO2_I2C_UART_ADDRESS 0x4d
#define _HIO_MODULE_CO2_PIN_DEFAULT (~(1 << 0) & ~(1 << 4))
#define _HIO_MODULE_CO2_PIN_CAP_ON (~(1 << 1))
#define _HIO_MODULE_CO2_PIN_VDD2_ON (~(1 << 2))
#define _HIO_MODULE_CO2_PIN_EN (~(1 << 3))
#define _HIO_MODULE_CO2_PIN_UART_RESET (~(1 << 6))
#define _HIO_MODULE_CO2_PIN_RDY HIO_TCA9534A_PIN_P7

typedef enum
{
    HIO_MODULE_CO2_STATE_ERROR = -1,
    HIO_MODULE_CO2_STATE_INITIALIZE = 0,
    HIO_MODULE_CO2_STATE_INITIALIZE1,
    HIO_MODULE_CO2_STATE_INITIALIZE2,
    HIO_MODULE_CO2_STATE_READY,
    HIO_MODULE_CO2_STATE_CHARGE,
    HIO_MODULE_CO2_STATE_BOOT,
    HIO_MODULE_CO2_STATE_BOOT_READ,
    HIO_MODULE_CO2_STATE_MEASURE,
    HIO_MODULE_CO2_STATE_MEASURE_READ,

} hio_module_co2_state_t;

static bool _hio_module_co2_init(void);
static bool _hio_module_co2_charge_enable(bool state);
static bool _hio_module_co2_device_enable(bool state);
static bool _hio_module_co2_read_signal_rdy(int *value);
static bool _hio_module_co2_uart_enable(bool state);
static size_t _hio_module_co2_uart_write(uint8_t *buffer, size_t length);
static size_t _hio_module_co2_uart_read(uint8_t *buffer, size_t length);

static struct
{
    hio_tca9534a_t tca9534a;
    hio_sc16is740_t sc16is740;
    hio_lp8_t sensor;
    const hio_lp8_driver_t driver;

} _hio_module_co2 = {
    .driver = {
        .init = _hio_module_co2_init,
        .charge_enable = _hio_module_co2_charge_enable,
        .device_enable = _hio_module_co2_device_enable,
        .read_signal_rdy = _hio_module_co2_read_signal_rdy,
        .uart_enable = _hio_module_co2_uart_enable,
        .uart_write = _hio_module_co2_uart_write,
        .uart_read = _hio_module_co2_uart_read
    }
};

void hio_module_co2_init(void)
{
    hio_lp8_init(&_hio_module_co2.sensor, &_hio_module_co2.driver);
}

void hio_module_co2_set_event_handler(void (*event_handler)(hio_module_co2_event_t, void *), void *event_param)
{
    hio_lp8_set_event_handler(&_hio_module_co2.sensor, (void (*)(hio_lp8_event_t, void *)) event_handler, event_param);
}

void hio_module_co2_set_update_interval(hio_tick_t interval)
{
    hio_lp8_set_update_interval(&_hio_module_co2.sensor, interval);
}

bool hio_module_co2_measure(void)
{
    return hio_lp8_measure(&_hio_module_co2.sensor);
}

bool hio_module_co2_get_concentration_ppm(float *ppm)
{
    return hio_lp8_get_concentration_ppm(&_hio_module_co2.sensor, ppm);
}

bool hio_module_co2_get_error(hio_lp8_error_t *error)
{
    return hio_lp8_get_error(&_hio_module_co2.sensor, error);
}

void hio_module_co2_calibration(hio_lp8_calibration_t calibration)
{
    hio_lp8_calibration(&_hio_module_co2.sensor, calibration);
}

static bool _hio_module_co2_init(void)
{
    if (!hio_tca9534a_init(&_hio_module_co2.tca9534a, HIO_I2C_I2C0, _HIO_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS))
    {
        return false;
    }

    if (!hio_tca9534a_write_port(&_hio_module_co2.tca9534a, 0x00))
    {
        return false;
    }

    // Reset SC16IS740
    if (!hio_tca9534a_set_port_direction(&_hio_module_co2.tca9534a, _HIO_MODULE_CO2_PIN_DEFAULT & _HIO_MODULE_CO2_PIN_UART_RESET))
    {
        return false;
    }

    // Reset pulse width > 3us
    for (volatile int i = 0; i < 100; i++)
    {
        continue;
    }

    if (!hio_tca9534a_set_port_direction(&_hio_module_co2.tca9534a, _HIO_MODULE_CO2_PIN_DEFAULT))
    {
        return false;
    }

    // Delay time width > 10us
    for (volatile int i = 0; i < 1000; i++)
    {
        continue;
    }

    if (!hio_sc16is740_init(&_hio_module_co2.sc16is740, HIO_I2C_I2C0, _HIO_MODULE_CO2_I2C_UART_ADDRESS))
    {
        return false;
    }

    return true;
}

static bool _hio_module_co2_charge_enable(bool state)
{
    uint8_t direction = _HIO_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _HIO_MODULE_CO2_PIN_VDD2_ON & _HIO_MODULE_CO2_PIN_CAP_ON;
    }

    return hio_tca9534a_set_port_direction(&_hio_module_co2.tca9534a, direction);
}

static bool _hio_module_co2_device_enable(bool state)
{
    uint8_t direction = _HIO_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _HIO_MODULE_CO2_PIN_VDD2_ON & _HIO_MODULE_CO2_PIN_CAP_ON & _HIO_MODULE_CO2_PIN_EN;
    }

    return hio_tca9534a_set_port_direction(&_hio_module_co2.tca9534a, direction);
}

static bool _hio_module_co2_read_signal_rdy(int *value)
{
    if (!hio_tca9534a_read_pin(&_hio_module_co2.tca9534a, _HIO_MODULE_CO2_PIN_RDY, value))
    {
        return false;
    }

    return true;
}

static bool _hio_module_co2_uart_enable(bool state)
{
    if (state)
    {
        return hio_sc16is740_reset_fifo(&_hio_module_co2.sc16is740, HIO_SC16IS740_FIFO_RX);
    }

    return true;
}

static size_t _hio_module_co2_uart_write(uint8_t *buffer, size_t length)
{
    return hio_sc16is740_write(&_hio_module_co2.sc16is740, buffer, length);
}

static size_t _hio_module_co2_uart_read(uint8_t *buffer, size_t length)
{
    return hio_sc16is740_read(&_hio_module_co2.sc16is740, buffer, length, 0);
}
