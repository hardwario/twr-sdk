#include <hio_lp8.h>

#define _HIO_LP8_MODBUS_DEVICE_ADDRESS 0xfe
#define _HIO_LP8_MODBUS_WRITE 0x41
#define _HIO_LP8_MODBUS_READ 0x44
#define _HIO_LP8_INITIAL_MEASUREMENT 0x10
#define _HIO_LP8_SEQUENTIAL_MEASUREMENT 0x20
#define _HIO_LP8_RX_ERROR_STATUS0 (3 + 0xa7 - 0x80)
#define _HIO_LP8_RX_ERROR_STATUS1 (3 + 0xa6 - 0x80)
#define _HIO_LP8_RX_CONC (3 + 0x9a - 0x80)

#define _HIO_LP8_CALIBRATION_TIMEOUT (8 * 24 * 60 * 60 * 1000)

static void _hio_lp8_task_interval(void *param);

static void _hio_lp8_task_measure(void *param);

static uint16_t _hio_lp8_calculate_crc16(uint8_t *buffer, uint8_t length);

void hio_lp8_init(hio_lp8_t *self, const hio_lp8_driver_t *driver)
{
    memset(self, 0, sizeof(*self));

    self->_driver = driver;

    self->_pressure = 10124;
    self->_calibration = HIO_LP8_CALIBRATION_ABC;
    self->_tick_calibration = hio_tick_get() + _HIO_LP8_CALIBRATION_TIMEOUT;

    self->_task_id_interval = hio_scheduler_register(_hio_lp8_task_interval, self, HIO_TICK_INFINITY);
    self->_task_id_measure = hio_scheduler_register(_hio_lp8_task_measure, self, 0);

    self->_driver->init();
}

void hio_lp8_set_event_handler(hio_lp8_t *self, void (*event_handler)(hio_lp8_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_lp8_set_update_interval(hio_lp8_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        hio_lp8_measure(self);
    }
}

bool hio_lp8_measure(hio_lp8_t *self)
{
    if (self->_state == HIO_LP8_STATE_READY)
    {
        self->_state = HIO_LP8_STATE_PRECHARGE;

        hio_scheduler_plan_now(self->_task_id_measure);

        return true;
    }
    else if (self->_state == HIO_LP8_STATE_INITIALIZE)
    {
        hio_scheduler_plan_now(self->_task_id_measure);
    }

    return false;
}

bool hio_lp8_get_concentration_ppm(hio_lp8_t *self, float *ppm)
{
    if (!self->_valid)
    {
        *ppm = NAN;

        return false;
    }

    *ppm = (float) self->_concentration;

    return true;
}

bool hio_lp8_get_error(hio_lp8_t *self, hio_lp8_error_t *error)
{
    *error = self->_error;

    return true;
}

void hio_lp8_calibration(hio_lp8_t *self, hio_lp8_calibration_t calibration)
{
    self->_calibration = calibration;

    self->_tick_calibration = 0;

    hio_lp8_measure(self);
}

static void _hio_lp8_task_interval(void *param)
{
    hio_lp8_t *self = (hio_lp8_t *) param;

    hio_lp8_measure(self);

    hio_scheduler_plan_current_relative(self->_update_interval);
}

static void _hio_lp8_error(hio_lp8_t *self, hio_lp8_error_t error)
{
    self->_state = HIO_LP8_STATE_ERROR;
    self->_error = error;
    hio_scheduler_plan_current_now();
}

static void _hio_lp8_task_measure(void *param)
{
    hio_lp8_t *self = (hio_lp8_t *) param;

start:

    switch (self->_state)
    {
        case HIO_LP8_STATE_ERROR:
        {
            self->_valid = false;

            self->_first_measurement_done = false;

            self->_driver->uart_enable(false);
            self->_driver->device_enable(false);
            self->_driver->charge_enable(false);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(HIO_LP8_EVENT_ERROR, self->_event_param);
            }

            self->_state = HIO_LP8_STATE_INITIALIZE;

            hio_scheduler_plan_current_from_now(500);

            return;
        }
        case HIO_LP8_STATE_INITIALIZE:
        {
            if (!self->_driver->charge_enable(true))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_INITIALIZE);
                goto start;
            }

            self->_state = HIO_LP8_STATE_CHARGE;

            hio_scheduler_plan_current_from_now(60000);

            return;
        }
        case HIO_LP8_STATE_READY:
        {
            return;
        }
        case HIO_LP8_STATE_PRECHARGE:
        {
            if (!self->_driver->charge_enable(true))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_PRECHARGE);
                goto start;
            }

            self->_state = HIO_LP8_STATE_CHARGE;

            hio_scheduler_plan_current_from_now(5000);

            return;
        }
        case HIO_LP8_STATE_CHARGE:
        {
            if (!self->_driver->charge_enable(false))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_CHARGE_CHARGE_ENABLE);
                goto start;
            }

            if (!self->_driver->device_enable(true))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_CHARGE_DEVICE_ENABLE);
                goto start;
            }

            self->_state = HIO_LP8_STATE_BOOT;

            self->_tick_timeout = hio_tick_get() + 300;

            hio_scheduler_plan_current_from_now(140);

            return;
        }
        case HIO_LP8_STATE_BOOT:
        {
            int signal_rdy_value;

            size_t length;

            if (!self->_driver->read_signal_rdy(&signal_rdy_value))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_SIGNAL_READY);
                goto start;
            }

            if (signal_rdy_value != 0)
            {
                if (hio_tick_get() >= self->_tick_timeout)
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_TIMEOUT);
                    goto start;
                }
                else
                {
                    hio_scheduler_plan_current_from_now(10);

                    return;
                }
            }

            if (!self->_first_measurement_done)
            {
                self->_tx_buffer[0] = _HIO_LP8_MODBUS_DEVICE_ADDRESS;
                self->_tx_buffer[1] = _HIO_LP8_MODBUS_WRITE;
                self->_tx_buffer[2] = 0x00;
                self->_tx_buffer[3] = 0x80;
                self->_tx_buffer[4] = 0x01;
                self->_tx_buffer[5] = _HIO_LP8_INITIAL_MEASUREMENT;
                self->_tx_buffer[6] = 0x28;
                self->_tx_buffer[7] = 0x7e;

                length = 8;
            }
            else
            {
                uint16_t crc16;

                self->_tx_buffer[0] = _HIO_LP8_MODBUS_DEVICE_ADDRESS;
                self->_tx_buffer[1] = _HIO_LP8_MODBUS_WRITE;
                self->_tx_buffer[2] = 0x00;
                self->_tx_buffer[3] = 0x80;
                self->_tx_buffer[4] = 0x1a;

                self->_calibration_run = self->_tick_calibration < hio_tick_get();

                if (self->_calibration_run)
                {
                    self->_tx_buffer[5] = self->_calibration;
                }
                else
                {
                    self->_tx_buffer[5] = _HIO_LP8_SEQUENTIAL_MEASUREMENT;
                }

                memcpy(&self->_tx_buffer[6], self->_sensor_state, 23);

                self->_tx_buffer[29] = self->_pressure >> 8;
                self->_tx_buffer[30] = self->_pressure;

                crc16 = _hio_lp8_calculate_crc16(self->_tx_buffer, 31);

                self->_tx_buffer[31] = crc16;
                self->_tx_buffer[32] = crc16 >> 8;

                length = 33;
            }

            if (!self->_driver->uart_enable(true))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_UART_ENABLE);

                goto start;
            }

            if (self->_driver->uart_write(self->_tx_buffer, length) != length)
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_UART_WRITE);

                goto start;
            }

            self->_rx_buffer_length = 0;

            self->_state = HIO_LP8_STATE_BOOT_READ;

            self->_tick_timeout = hio_tick_get() + 80;

            hio_scheduler_plan_current_from_now(10);

            return;
        }
        case HIO_LP8_STATE_BOOT_READ:
        {
            self->_rx_buffer_length += self->_driver->uart_read(self->_rx_buffer + self->_rx_buffer_length, 4 - self->_rx_buffer_length);

            if (self->_rx_buffer_length == 4)
            {
                if (!self->_driver->uart_enable(false))
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_READ_UART_ENABLE);

                    goto start;
                }

                if (self->_rx_buffer[0] != _HIO_LP8_MODBUS_DEVICE_ADDRESS)
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_READ_DEVICE_ADDRESS);

                    goto start;
                }

                if (self->_rx_buffer[1] != self->_tx_buffer[1])
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_READ_COMMAND);

                    goto start;
                }

                if (_hio_lp8_calculate_crc16(self->_rx_buffer, 4) != 0)
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_READ_CRC);

                    goto start;
                }

                self->_state = HIO_LP8_STATE_MEASURE;

                self->_tick_timeout = hio_tick_get() + (self->_calibration_run ? 1000 : 250);

                hio_scheduler_plan_current_from_now(50);

                return;
            }

            if (hio_tick_get() >= self->_tick_timeout)
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_BOOT_READ_TIMEOUT);

                goto start;
            }

            hio_scheduler_plan_current_from_now(10);

            return;
        }
        case HIO_LP8_STATE_MEASURE:
        {
            int signal_rdy_value;

            if (!self->_driver->read_signal_rdy(&signal_rdy_value))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_SIGNAL_RDY);

                goto start;
            }

            if (signal_rdy_value == 0)
            {
                if (hio_tick_get() >= self->_tick_timeout)
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_SIGNAL_RDY_TIMEOUT);

                    goto start;
                }
                else
                {
                    hio_scheduler_plan_current_from_now(10);

                    return;
                }
            }

            self->_tx_buffer[0] = _HIO_LP8_MODBUS_DEVICE_ADDRESS;
            self->_tx_buffer[1] = _HIO_LP8_MODBUS_READ;
            self->_tx_buffer[2] = 0x00;
            self->_tx_buffer[3] = 0x80;
            self->_tx_buffer[4] = 0x2c;

            uint16_t crc16 = _hio_lp8_calculate_crc16(self->_tx_buffer, 5);

            self->_tx_buffer[5] = crc16;
            self->_tx_buffer[6] = crc16 >> 8;

            if (!self->_driver->uart_enable(true))
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_UART_ENABLE);

                goto start;
            }

            if (self->_driver->uart_write(self->_tx_buffer,  7) !=  7)
            {
                _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_UART_WRITE);

                goto start;
            }

            self->_rx_buffer_length = 0;

            self->_state = HIO_LP8_STATE_MEASURE_READ;

            self->_tick_timeout = hio_tick_get() + 100;

            hio_scheduler_plan_current_from_now(10);

            return;
        }
        case HIO_LP8_STATE_MEASURE_READ:
        {
            self->_rx_buffer_length += self->_driver->uart_read(self->_rx_buffer + self->_rx_buffer_length, 49 - self->_rx_buffer_length);

            if (self->_rx_buffer_length == 49)
            {
                if (!self->_driver->uart_enable(false))
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_UART_ENABLE);

                    goto start;
                }

                if (!self->_driver->device_enable(false))
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_DEVICE_ENABLE);

                    goto start;
                }

                if (self->_rx_buffer[0] != _HIO_LP8_MODBUS_DEVICE_ADDRESS)
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_DEVICE_ADDRESS);

                    goto start;
                }

                if (self->_rx_buffer[1] != self->_tx_buffer[1])
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_COMMAND);

                    goto start;
                }

                if (_hio_lp8_calculate_crc16(self->_rx_buffer, 49) != 0)
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_CRC);

                    goto start;
                }

                if ((self->_rx_buffer[_HIO_LP8_RX_ERROR_STATUS0] & 0xdd) != 0)
                {

                    if (self->_calibration_run)
                    {
                        if ((self->_rx_buffer[_HIO_LP8_RX_ERROR_STATUS0] == 8) &&
                            (self->_calibration != HIO_LP8_CALIBRATION_ABC) &&
                            (self->_calibration != HIO_LP8_CALIBRATION_AHIO_RF))
                        {
                            self->_state = HIO_LP8_STATE_CHARGE;

                            hio_scheduler_plan_relative(self->_task_id_measure, 100);

                            return;
                        }
                    }
                    else
                    {
                        _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_CALIBRATION_RUN);

                        goto start;
                    }
                }

                if ((self->_rx_buffer[_HIO_LP8_RX_ERROR_STATUS1] & 0xf7) != 0)
                {
                     _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_STATUS1);

                     goto start;
                }

                if (self->_calibration_run)
                {
                    self->_calibration = HIO_LP8_CALIBRATION_ABC;

                    self->_tick_calibration = hio_tick_get() + _HIO_LP8_CALIBRATION_TIMEOUT;
                }

                memcpy(self->_sensor_state, &self->_rx_buffer[4], 23);

                self->_first_measurement_done = true;

                self->_concentration = (int16_t) self->_rx_buffer[_HIO_LP8_RX_CONC] << 8;
                self->_concentration |= (int16_t) self->_rx_buffer[_HIO_LP8_RX_CONC + 1];

                self->_valid = (self->_concentration >= 0) && (self->_concentration <= 10000);

                self->_state = HIO_LP8_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(HIO_LP8_EVENT_UPDATE, self->_event_param);
                }

                return;
            }
            else
            {
                if (hio_tick_get() >= self->_tick_timeout)
                {
                    _hio_lp8_error(self, HIO_LP8_ERROR_MEASURE_READ_TIMEOUT);

                    goto start;
                }
            }

            hio_scheduler_plan_current_from_now(10);

            return;
        }
        default:
        {
            return;
        }
    }
}

static uint16_t _hio_lp8_calculate_crc16(uint8_t *buffer, uint8_t length)
{
    uint16_t crc16;

    for (crc16 = 0xffff; length != 0; length--, buffer++)
    {
        crc16 ^= *buffer;

        for (int i = 0; i < 8; i++)
        {
            if ((crc16 & 1) != 0)
            {
                crc16 >>= 1;
                crc16 ^= 0xa001;
            }
            else
            {
                crc16 >>= 1;
            }
        }
    }

    return crc16;
}
