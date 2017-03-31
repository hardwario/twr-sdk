#include <bc_module_co2.h>
#include <bc_scheduler.h>
#include <bc_i2c.h>
#include <bc_tca9534a.h>
#include <bc_sc16is740.h>
#include <bc_tick.h>

#define _BC_MODULE_CO2_BOOST_PIN              (1 << 1)
#define _BC_MODULE_CO2_VDD2_PIN               (1 << 2)
#define _BC_MODULE_CO2_EN_PIN                 (1 << 3)
#define _BC_MODULE_CO2_UART_RESET_PIN         (1 << 6)
#define _BC_MODULE_CO2_RDY_PIN                BC_TCA9534A_PIN_P7

#define BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS  0xFE
#define BC_MODULE_CO2_MODBUS_WRITE           0x41
#define BC_MODULE_CO2_MODBUS_READ            0x44
#define BC_MODULE_CO2_INITIAL_MEASUREMENT    0x10
#define BC_MODULE_CO2_SEQUENTIAL_MEASUREMENT 0x20
#define BC_MODULE_CO2_RX_ERROR_STATUS0       (3+39)
#define BC_MODULE_CO2_CALIBRATION_ABC        0x70
#define BC_MODULE_CO2_CALIBRATION_ABC_RF     0x72
#define BC_MODULE_CO2_CALIBRATION_TIMEOUT    7 * 27 * 3600

typedef enum
{
    BC_MODULE_CO2_STATE_ERROR = -1,
    BC_MODULE_CO2_STATE_INITIALIZE = 0,
    BC_MODULE_CO2_STATE_INITIALIZE1,
    BC_MODULE_CO2_STATE_INITIALIZE2,
    BC_MODULE_CO2_STATE_PRECHARGE,
    BC_MODULE_CO2_STATE_READY,
    BC_MODULE_CO2_STATE_CHARGE,
    BC_MODULE_CO2_STATE_BOOT,
    BC_MODULE_CO2_STATE_BOOT_READ,
    BC_MODULE_CO2_STATE_MEASURE,
    BC_MODULE_CO2_STATE_MEASURE_READ,

    BC_MODULE_CO2_STATE_CALIBRATION_START,
    BC_MODULE_CO2_STATE_CALIBRATION_READ,
    BC_MODULE_CO2_STATE_CALIBRATION_DONE


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
    bc_tick_t start;
    bc_tick_t next_calibration;
    uint8_t rx_buffer[45];
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
    return false;
}

bool bc_module_co2_get_concentration(int16_t *concentration)
{
    if (!_bc_module_co2.valid)
    {
        return false;
    }

    *concentration = _bc_module_co2.concentration;
    return true;
}

void bc_module_co2_calibration()
{
    _bc_module_co2.next_calibration = 0;
    bc_module_co2_measure();
}

static void _bc_module_co2_task_interval(void *param)
{
    (void) param;

    bc_module_co2_measure();

    bc_scheduler_plan_current_relative(_bc_module_co2.update_interval);
}

static void _bc_module_co2_task_measure(void *param)
{

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
//            bc_scheduler_plan_current_relative(_bc_module_co2.update_interval);
            bc_scheduler_plan_current_relative(0);
            return;
        }
        case BC_MODULE_CO2_STATE_INITIALIZE:
        {
            _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;

            if (!bc_tca9534a_init(&_bc_module_co2.tca9534a, BC_I2C_I2C0, BC_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS))
            {
                goto start;
            }

            if (!bc_tca9534a_write_port(&_bc_module_co2.tca9534a, 0x00))
            {
                goto start;
            }

            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, ~_BC_MODULE_CO2_UART_RESET_PIN))
            {
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_INITIALIZE1;
            bc_scheduler_plan_current_now();
            return;
        }
        case BC_MODULE_CO2_STATE_INITIALIZE1:
        {
            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, 0xFF))
            {
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_INITIALIZE2;
            bc_scheduler_plan_current_now();
            return;

        }
        case BC_MODULE_CO2_STATE_INITIALIZE2:
        {
            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, ~(_BC_MODULE_CO2_VDD2_PIN | _BC_MODULE_CO2_BOOST_PIN)))
            {
                goto start;
            }

            if (!bc_sc16is740_init(&_bc_module_co2.sc16is740, BC_I2C_I2C0, BC_MODULE_CO2_I2C_UART_ADDRESS))
            {
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_PRECHARGE;
            bc_scheduler_plan_current_relative(45000);
            return;
        }
        case BC_MODULE_CO2_STATE_PRECHARGE:
        {
            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, 0xFF))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_CHARGE;
            goto start;
            return;
        }
        case BC_MODULE_CO2_STATE_READY:
        {
            return;
        }
        case BC_MODULE_CO2_STATE_CHARGE:
        {
            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, ~(_BC_MODULE_CO2_VDD2_PIN | _BC_MODULE_CO2_BOOST_PIN | _BC_MODULE_CO2_EN_PIN)))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_BOOT;
            _bc_module_co2.start = bc_tick_get();
            bc_scheduler_plan_current_relative(134);
            return;
        }
        case BC_MODULE_CO2_STATE_BOOT:
        {
            bc_tca9534a_state_t rdy_pin_value;
            uint8_t length;

            if (!bc_tca9534a_read_pin(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_RDY_PIN, &rdy_pin_value))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (rdy_pin_value == BC_TCA9534A_PIN_STATE_HIGH)
            {
                if ((_bc_module_co2.start + 205) < bc_tick_get())
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
                _bc_module_co2.tx_buffer[5] = BC_MODULE_CO2_SEQUENTIAL_MEASUREMENT;

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
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;

                if (!bc_sc16is740_read(&_bc_module_co2.sc16is740, _bc_module_co2.rx_buffer, 4, 100))
                {
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[0] != BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS)
                {
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[1] != _bc_module_co2.tx_buffer[1])
                {
                    goto start;
                }

                if (_bc_module_co2_calculate_crc16(_bc_module_co2.rx_buffer, 4) != 0)
                {
                    goto start;
                }

                _bc_module_co2.state = BC_MODULE_CO2_STATE_MEASURE;
                bc_scheduler_plan_current_relative(310 - (bc_tick_get() - _bc_module_co2.start));
                return;
            }
            else
            {
                if ((_bc_module_co2.start + 310) < bc_tick_get())
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

            if (!bc_tca9534a_read_pin(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_RDY_PIN, &rdy_pin_value))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (rdy_pin_value == BC_TCA9534A_PIN_STATE_LOW)
            {
                if ((_bc_module_co2.start + 355) < bc_tick_get())
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
            _bc_module_co2.tx_buffer[4] = 0x28;//40
            _bc_module_co2.tx_buffer[5] = 0x78;
            _bc_module_co2.tx_buffer[6] = 0xFA;

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
            if (available == 45)
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;

                if (!bc_sc16is740_read(&_bc_module_co2.sc16is740, _bc_module_co2.rx_buffer, 45, 100))
                {
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[0] != BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS)
                {
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[1] != _bc_module_co2.tx_buffer[1])
                {
                    goto start;
                }

                if (_bc_module_co2_calculate_crc16(_bc_module_co2.rx_buffer, 45) != 0)
                {
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[BC_MODULE_CO2_RX_ERROR_STATUS0] != 0)
                {
                    goto start;
                }

                memcpy(_bc_module_co2.sensor_state, &_bc_module_co2.rx_buffer[4], 23);

                _bc_module_co2.first_measurement_done = true;

                _bc_module_co2.concentration = ((int16_t) _bc_module_co2.rx_buffer[29]) << 8;
                _bc_module_co2.concentration |= (int16_t) _bc_module_co2.rx_buffer[30];
                _bc_module_co2.valid = true;

                bool calibration = _bc_module_co2.next_calibration < bc_tick_get();

                if (!calibration)
                {
                    if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, 0xFF))
                    {
                        goto start;
                    }
                }

                _bc_module_co2.state = BC_MODULE_CO2_STATE_READY;

                if (_bc_module_co2.event_handler != NULL)
                {
                    _bc_module_co2.event_handler(BC_MODULE_CO2_EVENT_UPDATE, _bc_module_co2.event_param);
                }

                if (calibration)
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_CALIBRATION_START;
                    goto start;
                }

                return;
            }
            else
            {
                if ((_bc_module_co2.start + 580) < bc_tick_get())
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }
            }
            bc_scheduler_plan_current_now();
            return;
        }
        case BC_MODULE_CO2_STATE_CALIBRATION_START:
        {
            uint16_t crc16;

            _bc_module_co2.tx_buffer[0] = BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS;
            _bc_module_co2.tx_buffer[1] = BC_MODULE_CO2_MODBUS_WRITE;
            _bc_module_co2.tx_buffer[2] = 0x00;
            _bc_module_co2.tx_buffer[3] = 0x80;
            _bc_module_co2.tx_buffer[4] = 0x01;
            _bc_module_co2.tx_buffer[5] = BC_MODULE_CO2_CALIBRATION_ABC;

            crc16 = _bc_module_co2_calculate_crc16(_bc_module_co2.tx_buffer, 6);

            _bc_module_co2.tx_buffer[6] = (uint8_t) crc16;
            _bc_module_co2.tx_buffer[7] = (uint8_t) (crc16 >> 8);

            if (!bc_sc16is740_reset_fifo(&_bc_module_co2.sc16is740, BC_SC16IS740_FIFO_RX))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (!bc_sc16is740_write(&_bc_module_co2.sc16is740, _bc_module_co2.tx_buffer, 8))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            _bc_module_co2.state = BC_MODULE_CO2_STATE_CALIBRATION_READ;
            bc_scheduler_plan_current_now();
            return;
        }
        case BC_MODULE_CO2_STATE_CALIBRATION_READ:
        {
            uint8_t available;
            if (!bc_sc16is740_available(&_bc_module_co2.sc16is740, &available))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (available == 4)
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;

                if (!bc_sc16is740_read(&_bc_module_co2.sc16is740, _bc_module_co2.rx_buffer, 4, 100))
                {
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[0] != BC_MODULE_CO2_MODBUS_DEVICE_ADDRESS)
                {
                    goto start;
                }

                if (_bc_module_co2.rx_buffer[1] != _bc_module_co2.tx_buffer[1])
                {
                    goto start;
                }

                if (_bc_module_co2_calculate_crc16(_bc_module_co2.rx_buffer, 4) != 0)
                {
                    goto start;
                }

                _bc_module_co2.state = BC_MODULE_CO2_STATE_CALIBRATION_DONE;
            }
            else
            {
                if ((_bc_module_co2.start + 580) < bc_tick_get())
                {
                    _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                    goto start;
                }
            }
            bc_scheduler_plan_current_now();
            return;
        }
        case BC_MODULE_CO2_STATE_CALIBRATION_DONE:
        {
            bc_tca9534a_state_t rdy_pin_value;

            if (!bc_tca9534a_read_pin(&_bc_module_co2.tca9534a, _BC_MODULE_CO2_RDY_PIN, &rdy_pin_value))
            {
                _bc_module_co2.state = BC_MODULE_CO2_STATE_ERROR;
                goto start;
            }

            if (rdy_pin_value == BC_TCA9534A_PIN_STATE_LOW)
            {
                if ((_bc_module_co2.start + 580) < bc_tick_get())
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

            if (!bc_tca9534a_set_port_direction(&_bc_module_co2.tca9534a, 0xFF))
            {
                goto start;
            }

            _bc_module_co2.next_calibration = bc_tick_get() + BC_MODULE_CO2_CALIBRATION_TIMEOUT;
            _bc_module_co2.state = BC_MODULE_CO2_STATE_CHARGE;
            bc_scheduler_plan_current_now();
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
