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

static struct
{
    bc_scheduler_task_id_t task_id_interval;
    bc_scheduler_task_id_t task_id_measure;
    void (*event_handler)(bc_module_co2_event_t, void *);
    void *event_param;
    bc_tick_t update_interval;
    bc_module_co2_state_t state;
    bc_tca9534a_t tca9534a;
    bc_sc16is740_t sc16is740;
    bool first_measurement_done;
    bc_tick_t tick_timeout;
    bc_tick_t tick_calibration_timeout;
    bc_module_co2_calibration_t calibration;
    bool calibration_run;
    uint8_t rx_buffer[49];
    uint8_t tx_buffer[33];
    uint8_t sensor_state[23];
    bool valid;
    int16_t concentration;
    uint16_t pressure;

} _bc_module_co2;

static void _bc_module_co2_task_interval(void *param);

static void _bc_module_co2_task_measure(void *param);

static uint16_t _bc_module_co2_calculate_crc16(uint8_t *buffer, uint8_t length);

void bc_module_co2_init(void)
{
    memset(&_bc_module_co2, 0, sizeof(_bc_module_co2));

    _bc_module_co2.pressure = 10124;
    _bc_module_co2.calibration = BC_MODULE_CO2_CALIBRATION_ABC;
    _bc_module_co2.tick_calibration_timeout = bc_tick_get() + BC_MODULE_CO2_CALIBRATION_TIMEOUT;

    _bc_module_co2.task_id_interval = bc_scheduler_register(_bc_module_co2_task_interval, NULL, BC_TICK_INFINITY);
    _bc_module_co2.task_id_measure = bc_scheduler_register(_bc_module_co2_task_measure, NULL, 0);
}

void bc_module_co2_set_event_handler(void (*event_handler)(bc_module_co2_event_t, void *), void *event_param)
{
    _bc_module_co2.event_handler = event_handler;
    _bc_module_co2.event_param = event_param;
}

void bc_module_co2_set_update_interval(bc_tick_t interval)
{
    _bc_module_co2.update_interval = interval;

    if (_bc_module_co2.update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(_bc_module_co2.task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(_bc_module_co2.task_id_interval, _bc_module_co2.update_interval);

        bc_module_co2_measure();
    }
}

bool bc_module_co2_measure(void)
{
    if (_bc_module_co2.state == BC_MODULE_CO2_STATE_READY)
    {
        _bc_module_co2.state = BC_MODULE_CO2_STATE_CHARGE;
        bc_scheduler_plan_now(_bc_module_co2.task_id_measure);
        return true;
    }
    else if (_bc_module_co2.state == BC_MODULE_CO2_STATE_INITIALIZE)
    {
        bc_scheduler_plan_now(_bc_module_co2.task_id_measure);
    }
    return false;
}

bool bc_module_co2_get_concentration(float *concentration)
{
    if (!_bc_module_co2.valid)
    {
        return false;
    }

    *concentration = (float) _bc_module_co2.concentration;
    return true;
}

void bc_module_co2_calibration(bc_module_co2_calibration_t calibration)
{
    _bc_module_co2.calibration = calibration;
    _bc_module_co2.tick_calibration_timeout = 0;
    bc_module_co2_measure();
}

static void _bc_module_co2_task_interval(void *param)
{
    (void) param;

    bc_module_co2_measure();

    bc_scheduler_plan_current_relative(_bc_module_co2.update_interval);
}

volatile bc_module_co2_state_t state;

static void _bc_module_co2_task_measure(void *param)
{
    (void) param;

    state = _bc_module_co2.state;

start:

    switch (_bc_module_co2.state)
    {
        case BC_MODULE_CO2_STATE_ERROR:
        {
            _bc_module_co2.valid = false;
            _bc_module_co2.first_measurement_done = false;

            if (_bc_module_co2.event_handler != NULL)
            {
                _bc_module_co2.event_handler(BC_MODULE_CO2_EVENT_ERROR, _bc_module_co2.event_param);
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_INITIALIZE;
            bc_scheduler_plan_current_relative(_bc_module_co2.update_interval);
            return;
        }
        case BC_MODULE_CO2_STATE_INITIALIZE:
        {
            if (!bc_tca9534a_init(&_bc_module_co2.tca9534a, BC_I2C_I2C0, BC_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (!bc_tca9534a_write_port(&_bc_module_co2.tca9534a, 0x00))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT & _BC_MODULE_CO2_PIN_UART_RESET))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_INITIALIZE1;
            bc_scheduler_plan_current_now();
            return;
        }
        case BC_MODULE_CO2_STATE_INITIALIZE1:
        {
            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_INITIALIZE2;
            bc_scheduler_plan_current_relative(50);
            return;

        }
        case BC_MODULE_CO2_STATE_INITIALIZE2:
        {
            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT & _BC_MODULE_CO2_PIN_VDD2_ON & _BC_MODULE_CO2_PIN_CAP_ON))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (!bc_sc16is740_init(&_bc_module_co2.sc16is740, BC_I2C_I2C0, BC_MODULE_CO2_I2C_UART_ADDRESS))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_CHARGE;
            bc_scheduler_plan_current_relative(45000);
            return;
        }
        case BC_MODULE_CO2_STATE_READY:
        {
            return;
        }
        case BC_MODULE_CO2_STATE_CHARGE:
        {
            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT & _BC_MODULE_CO2_PIN_VDD2_ON & _BC_MODULE_CO2_PIN_CAP_ON & _BC_MODULE_CO2_PIN_EN))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_BOOT;
            _bc_module_co2.tick_timeout = bc_tick_get() + 300;
            bc_scheduler_plan_current_relative(100);
            return;
        }
        case BC_MODULE_CO2_STATE_BOOT:
        {
            bc_tca9534a_state_t rdy_pin_value;
            uint8_t length;

            if (!bc_tca9534a_read_pin(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_RDY, &rdy_pin_value))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (rdy_pin_value == BC_TCA9534A_PIN_STATE_HIGH)
            {
                if (bc_tick_get() >= _bc_module_co2.tick_timeout)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }
                else
                {
                    bc_scheduler_plan_current_now();
                    return;
                }
            }

            if (!_bc_module_co2.first_measurement_done)
            {
                _bc_module_co2.tx_buffer[0] = BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS;
                _bc_module_co2.tx_buffer[1] = BC_MODULE_CO2_MODBUS_WRITE;
                _bc_module_co2.tx_buffer[2] = 0x00;
                _bc_module_co2.tx_buffer[3] = 0x80;
                _bc_module_co2.tx_buffer[4] = 0x01;
                _bc_module_co2.tx_buffer[5] = BC_MODULE_CO2_INITIAL_MEASUREMENT;
                _bc_module_co2.tx_buffer[6] = 0x28;//crc low
                _bc_module_co2.tx_buffer[7] = 0x7E;//crc high

                length = 8;
            }
            else
            {
                uint16_t crc16;

                _bc_module_co2.tx_buffer[0] = BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS;
                _bc_module_co2.tx_buffer[1] = BC_MODULE_CO2_MODBUS_WRITE;
                _bc_module_co2.tx_buffer[2] = 0x00;
                _bc_module_co2.tx_buffer[3] = 0x80;
                _bc_module_co2.tx_buffer[4] = 0x1A;//26

                _bc_module_co2.calibration_run = _bc_module_co2.tick_calibration_timeout < bc_tick_get();

                if (_bc_module_co2.calibration_run)
                {
                    _bc_module_co2.tx_buffer[5] = _bc_module_co2.calibration;
                }
                else
                {
                    _bc_module_co2.tx_buffer[5] = BC_MODULE_CO2_SEQUENTIAL_MEASUREMENT;
                }

                //copy previous measurement data
                memcpy(&_bc_module_co2.tx_buffer[6], _bc_module_co2.sensor_state, 23);

                _bc_module_co2.tx_buffer[29] = (uint8_t) (_bc_module_co2.pressure >> 8);
                _bc_module_co2.tx_buffer[30] = (uint8_t) _bc_module_co2.pressure;

                crc16 = _bc_module_co2_calculate_crc16(_bc_module_co2.tx_buffer, 31);

                _bc_module_co2.tx_buffer[31] = (uint8_t) crc16;
                _bc_module_co2.tx_buffer[32] = (uint8_t) (crc16 >> 8);

                length = 33;
            }

            if (!bc_sc16is740_reset_fifo(&_bc_module_co2.sc16is740, BC_SC16IS740_FIFO_RX))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (bc_sc16is740_write(&_bc_module_co2.sc16is740, _bc_module_co2.tx_buffer, length) != length)
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_BOOT_READ;
            _bc_module_co2.tick_timeout = bc_tick_get() + 250;
            bc_scheduler_plan_current_now();
            return;
        }
        case BC_MODULE_CO2_STATE_BOOT_READ:
        {
            uint8_t available;
            if (!bc_sc16is740_available(&_bc_module_co2.sc16is740, &available))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (available == 4)
            {

                if (!bc_sc16is740_read(&_bc_module_co2.sc16is740, _bc_module_co2.rx_buffer, 4, 100))
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[0] != BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[1] != _bc_module_co2.tx_buffer[1])
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (_bc_module_co2_calculate_crc16(_bc_module_co2.rx_buffer, 4) != 0)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                _bc_module_co2.state = BC_MODULE_CO2_STATE_MEASURE;
                _bc_module_co2.tick_timeout = bc_tick_get() + (_bc_module_co2.calibration_run ? 1000 : 250);
                bc_scheduler_plan_current_relative(50);
                return;
            }
            else
            {
                if (bc_tick_get() >= _bc_module_co2.tick_timeout)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }
            }
            bc_scheduler_plan_current_now();
            return;

        }
        case BC_MODULE_CO2_STATE_MEASURE:
        {
            bc_tca9534a_state_t rdy_pin_value;

            if (!bc_tca9534a_read_pin(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_RDY, &rdy_pin_value))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (rdy_pin_value == BC_TCA9534A_PIN_STATE_LOW)
            {
                if (bc_tick_get() >= _bc_module_co2.tick_timeout)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }
                else
                {
                    bc_scheduler_plan_current_now();
                    return;
                }
            }

            _bc_module_co2.tx_buffer[0] = BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS;
            _bc_module_co2.tx_buffer[1] = BC_MODULE_CO2_MODBUS_READ;
            _bc_module_co2.tx_buffer[2] = 0x00;
            _bc_module_co2.tx_buffer[3] = 0x80;
            _bc_module_co2.tx_buffer[4] = 0x2C;

            uint16_t crc16 = _bc_module_co2_calculate_crc16(_bc_module_co2.tx_buffer, 5);

            _bc_module_co2.tx_buffer[5] = (uint8_t) crc16;
            _bc_module_co2.tx_buffer[6] = (uint8_t) (crc16 >> 8);


            if (!bc_sc16is740_reset_fifo(&_bc_module_co2.sc16is740, BC_SC16IS740_FIFO_RX))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (!bc_sc16is740_write(&_bc_module_co2.sc16is740, _bc_module_co2.tx_buffer, 7))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_MEASURE_READ;
            _bc_module_co2.tick_timeout = bc_tick_get() + 250;
            bc_scheduler_plan_current_now();
            return;
        }
        case BC_MODULE_CO2_STATE_MEASURE_READ:
        {
            uint8_t available;

            if (!bc_sc16is740_available(&_bc_module_co2.sc16is740, &available))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (available == 49)
            {
                if (!bc_sc16is740_read(&_bc_module_co2.sc16is740, _bc_module_co2.rx_buffer, 49, 100))
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_PIN_DEFAULT))
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[0] != BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[1] != _bc_module_co2.tx_buffer[1])
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (_bc_module_co2_calculate_crc16(_bc_module_co2.rx_buffer, 49) != 0)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[BC_MODULE_CO2_RX_ERROR_STATUS0] != 0)
                {
                    if (_bc_module_co2.calibration_run)
                    {
                        if ((_bc_module_co2.rx_buffer[BC_MODULE_CO2_RX_ERROR_STATUS0] == 8) &&
                                (_bc_module_co2.calibration != BC_MODULE_CO2_CALIBRATION_ABC) &&
                                        (_bc_module_co2.calibration != BC_MODULE_CO2_CALIBRATION_ABC_RF))
                           {
                                _bc_module_co2.state = BC_MODULE_CO2_STATE_CHARGE;
                                bc_scheduler_plan_relative(_bc_module_co2.task_id_measure, 100);
                                return;
                           }
                    }
                    else
                    {

                        _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                        goto start;
                    }
                }

                if (_bc_module_co2.calibration_run)
                {
                    _bc_module_co2.calibration = BC_MODULE_CO2_CALIBRATION_ABC;
                    _bc_module_co2.tick_calibration_timeout = bc_tick_get() + BC_MODULE_CO2_CALIBRATION_TIMEOUT;
                }

                memcpy(_bc_module_co2.sensor_state, &_bc_module_co2.rx_buffer[4], 23);

                _bc_module_co2.first_measurement_done = true;

                _bc_module_co2.concentration = ((int16_t) _bc_module_co2.rx_buffer[29]) << 8;
                _bc_module_co2.concentration |= (int16_t) _bc_module_co2.rx_buffer[30];
                _bc_module_co2.valid = true;

                _bc_module_co2.state = BC_MODULE_CO2_STATE_READY;

                if (_bc_module_co2.event_handler != NULL)
                {
                    _bc_module_co2.event_handler(BC_MODULE_CO2_EVENT_UPDATE, _bc_module_co2.event_param);
                }

                return;
            }
            else
            {
                if (bc_tick_get() >= _bc_module_co2.tick_timeout)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }
            }
            bc_scheduler_plan_current_now();
            return;
        }

        default:
        {
            return;
        }
    }
}

static uint16_t _bc_module_co2_calculate_crc16(uint8_t *buffer, uint8_t length)
{
    uint16_t crc16;

    for (crc16 = 0xFFFF; length != 0; length--, buffer++)
    {
        uint8_t i;

        crc16 ^= *buffer;

        for (i = 0; i < 8; i++)
        {
            if ((crc16 & 0x0001) != 0)
            {
                crc16 >>= 1;
                crc16 ^= 0xA001;
            }
            else
            {
                crc16 >>= 1;
            }
        }
    }

    return crc16;
}
