#include <bc_module_co2.h>
#include <bc_scheduler.h>
#include <bc_i2c.h>
#include <bc_tca9534a.h>
#include <bc_sc16is740.h>
#include <bc_tick.h>

#define _BC_MODULE_CO2_PIN_DEFAULT            (~(1 << 0) & ~(1 << 4))
#define _BC_MODULE_CO2_PIN_CAP_ON             (~(1 << 1))
#define _BC_MODULE_CO2_PIN_VDD2_ON            (~(1 << 2))
#define _BC_MODULE_CO2_PIN_EN                 (~(1 << 3))
#define _BC_MODULE_CO2_PIN_UART_RESET         (~(1 << 6))
#define _BC_MODULE_CO2_PIN_RDY                BC_TCA9534A_PIN_P7
#define BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS  0xFE
#define BC_MODULE_CO2_MODBUS_WRITE           0x41
#define BC_MODULE_CO2_MODBUS_READ            0x44
#define BC_MODULE_CO2_INITIAL_MEASUREMENT    0x10
#define BC_MODULE_CO2_SEQUENTIAL_MEASUREMENT 0x20
#define BC_MODULE_CO2_RX_ERROR_STATUS0       (3+39)
#define BC_MODULE_CO2_CALIBRATION_TIMEOUT    8 * 24 * 60 * 60 * 1000

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
static bool _bc_module_co2_charge(bool state);
static bool _bc_module_co2_enable(bool state);
static bool _bc_module_co2_rdy(bool *state);
static bool _bc_module_co2_uart_enable(bool state);
static size_t _bc_module_co2_uart_write(uint8_t *buffer, size_t length);
static size_t _bc_module_co2_uart_read(uint8_t *buffer, size_t length);

static struct
{
    bc_tca9534a_t tca9534a;
    bc_sc16is740_t sc16is740;
    bc_co2_sensor_t sensor;
    bc_co2_sensor_driver_t driver;

} _bc_module_co2 = {
    .driver = {
        .init = _bc_module_co2_init,
        .charge = _bc_module_co2_charge,
        .enable = _bc_module_co2_enable,
        .rdy = _bc_module_co2_rdy,
        .uart_enable = _bc_module_co2_uart_enable,
        .uart_write = _bc_module_co2_uart_write,
        .uart_read = _bc_module_co2_uart_read
    }
};

void bc_module_co2_init(void)
{
    bc_co2_sensor_init(&_bc_module_co2.sensor, &_bc_module_co2.driver);
}

void bc_module_co2_set_event_handler(void (*event_handler)(bc_module_co2_event_t, void *), void *event_param)
{
    bc_co2_sensor_set_event_handler(&_bc_module_co2.sensor, (void (*)(bc_co2_sensor_event_t, void *)) event_handler, event_param);
}

void bc_module_co2_set_update_interval(bc_tick_t interval)
{
    bc_co2_sensor_set_update_interval(&_bc_module_co2.sensor, interval);
}

bool bc_module_co2_measure(void)
{
    return bc_co2_sensor_measure(&_bc_module_co2.sensor);
}

bool bc_module_co2_get_concentration(float *concentration)
{
    return bc_co2_sensor_get_concentration(&_bc_module_co2.sensor, concentration);
}

void bc_module_co2_calibration(bc_co2_sensor_calibration_t calibration)
{
    bc_co2_sensor_calibration(&_bc_module_co2.sensor, calibration);
}

static bool _bc_module_co2_init(void)
{
    if (!bc_tca9534a_init(&_bc_module_co2.tca9534a, BC_I2C_I2C0, BC_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS))
    {
        return false;
    }

    if (!bc_tca9534a_write_port(&_bc_module_co2.tca9534a, 0x00))
    {
        return false;
    }

    // Reset sc16is740
    if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT & _BC_MODULE_CO2_PIN_UART_RESET))
    {
        return false;
    }

    // Reset pulse width > 3us
    for (int i = 0; i < 100; i++)
    {
        continue;
    }

    if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT))
    {
        return false;
    }

    // Delay time width > 10us
    for (int i = 0; i < 1000; i++)
    {
        continue;
    }

    if (!bc_sc16is740_init(&_bc_module_co2.sc16is740, BC_I2C_I2C0, BC_MODULE_CO2_I2C_UART_ADDRESS))
    {
        return false;
    }

    return true;
}

static bool _bc_module_co2_charge(bool state)
{
    uint8_t direction = _BC_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _BC_MODULE_CO2_PIN_VDD2_ON & _BC_MODULE_CO2_PIN_CAP_ON;
    }

    return bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, direction);
}

static bool _bc_module_co2_enable(bool state)
{
    uint8_t direction = _BC_MODULE_CO2_PIN_DEFAULT;

    if (state)
    {
        direction &= _BC_MODULE_CO2_PIN_VDD2_ON & _BC_MODULE_CO2_PIN_CAP_ON & _BC_MODULE_CO2_PIN_EN;
    }

    return bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, direction);
}

static bool _bc_module_co2_rdy(bool *state)
{
    bc_tca9534a_state_t rdy_pin_value;

    if (!bc_tca9534a_read_pin(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_RDY, &rdy_pin_value))
    {
        return false;
    }

    *state = rdy_pin_value == BC_TCA9534A_PIN_STATE_HIGH;

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
