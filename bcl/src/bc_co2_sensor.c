#include <bc_co2_sensor.h>

// TODO Delete
#include <bc_log.h>

#define _BC_CO2_SENSOR_MODBUS_DEVICE_ADDRESS 0xfe
#define _BC_CO2_SENSOR_MODBUS_WRITE 0x41
#define _BC_CO2_SENSOR_MODBUS_READ 0x44
#define _BC_CO2_SENSOR_INITIAL_MEASUREMENT 0x10
#define _BC_CO2_SENSOR_SEQUENTIAL_MEASUREMENT 0x20
#define _BC_CO2_SENSOR_RX_ERROR_STATUS0 (3 + 39)
#define _BC_CO2_SENSOR_CALIBRATION_TIMEOUT (8 * 24 * 60 * 60 * 1000)

static void _bc_co2_sensor_task_interval(void *param);

static void _bc_co2_sensor_task_measure(void *param);

static uint16_t _bc_co2_sensor_calculate_crc16(uint8_t *buffer, uint8_t length);

void bc_co2_sensor_init(bc_co2_sensor_t *self, const bc_co2_sensor_driver_t *driver)
{
    memset(self, 0, sizeof(*self));

    self->_driver = driver;

    self->_pressure = 10124;
    self->_calibration = BC_CO2_SENSOR_CALIBRATION_ABC;
    self->_tick_calibration_timeout = bc_tick_get() + _BC_CO2_SENSOR_CALIBRATION_TIMEOUT;

    self->_task_id_interval = bc_scheduler_register(_bc_co2_sensor_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_co2_sensor_task_measure, self, 0);

    self->_driver->init();
}

void bc_co2_sensor_set_event_handler(bc_co2_sensor_t *self, void (*event_handler)(bc_co2_sensor_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_co2_sensor_set_update_interval(bc_co2_sensor_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_co2_sensor_measure(self);
    }
}

bool bc_co2_sensor_measure(bc_co2_sensor_t *self)
{
    if (self->_state == BC_CO2_SENSOR_STATE_READY)
    {
        self->_state = BC_CO2_SENSOR_STATE_CHARGE;

        bc_scheduler_plan_now(self->_task_id_measure);

        return true;
    }
    else if (self->_state == BC_CO2_SENSOR_STATE_INITIALIZE)
    {
        bc_scheduler_plan_now(self->_task_id_measure);
    }

    return false;
}

bool bc_co2_sensor_get_concentration_ppm(bc_co2_sensor_t *self, float *ppm)
{
    if (!self->_valid)
    {
        return false;
    }

    *ppm = (float) self->_concentration;

    if (*ppm > 10000.f)
    {
        return false;
    }

    return true;
}

void bc_co2_sensor_calibration(bc_co2_sensor_t *self, bc_co2_sensor_calibration_t calibration)
{
    self->_calibration = calibration;

    self->_tick_calibration_timeout = 0;

    bc_co2_sensor_measure(self);
}

static void _bc_co2_sensor_task_interval(void *param)
{
    bc_co2_sensor_t *self = (bc_co2_sensor_t *) param;

    bc_co2_sensor_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_co2_sensor_task_measure(void *param)
{
    bc_co2_sensor_t *self = (bc_co2_sensor_t *) param;

    volatile int line = -1; line;

start:

    switch (self->_state)
    {
        case BC_CO2_SENSOR_STATE_ERROR:
        {
            self->_valid = false;

            self->_first_measurement_done = false;

            self->_driver->uart_enable(false);
            self->_driver->enable(false);
            self->_driver->charge(false);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(BC_CO2_SENSOR_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_CO2_SENSOR_STATE_INITIALIZE;

            return;
        }
        case BC_CO2_SENSOR_STATE_INITIALIZE:
        {
            bc_log_debug("LP8: Processing state BC_CO2_SENSOR_STATE_INITIALIZE");

            self->_state = BC_CO2_SENSOR_STATE_ERROR;

            if (!self->_driver->charge(true))
            {
                bc_log_error("LP8: Charge enable failed");

                line = __LINE__; goto start;
            }

            self->_state = BC_CO2_SENSOR_STATE_CHARGE;

            bc_scheduler_plan_current_from_now(45000);

            return;
        }
        case BC_CO2_SENSOR_STATE_READY:
        {
            bc_log_debug("LP8: Processing state BC_CO2_SENSOR_STATE_READY");

            return;
        }
        case BC_CO2_SENSOR_STATE_CHARGE:
        {
            bc_log_debug("LP8: Processing state BC_CO2_SENSOR_STATE_CHARGE");

            self->_state = BC_CO2_SENSOR_STATE_ERROR;

            if (!self->_driver->charge(false))
            {
                bc_log_error("LP8: Charge disable failed");

                line = __LINE__; goto start;
            }

            if (!self->_driver->enable(true))
            {
                bc_log_error("LP8: Power enable failed");

                line = __LINE__; goto start;
            }

            self->_state = BC_CO2_SENSOR_STATE_BOOT;

            self->_tick_timeout = bc_tick_get() + 300;

            bc_scheduler_plan_current_from_now(140);

            return;
        }
        case BC_CO2_SENSOR_STATE_BOOT:
        {
            bc_log_debug("LP8: Processing state BC_CO2_SENSOR_STATE_BOOT");

            bool rdy_pin_value;

            size_t length;

            if (!self->_driver->rdy(&rdy_pin_value))
            {
                bc_log_error("LP8: Signal RDY read failed");

                self->_state = BC_CO2_SENSOR_STATE_ERROR;

                line = __LINE__; goto start;
            }

            if (rdy_pin_value)
            {
                if (bc_tick_get() >= self->_tick_timeout)
                {
                    bc_log_error("LP8: Signal RDY timed out");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }
                else
                {
                    bc_log_debug("LP8: Signal RDY not asserted yet...");

                    bc_scheduler_plan_current_from_now(10);

                    return;
                }
            }

            if (!self->_first_measurement_done)
            {
                bc_log_debug("LP8: Preparing challenge (first time)");

                self->_tx_buffer[0] = _BC_CO2_SENSOR_MODBUS_DEVICE_ADDRESS;
                self->_tx_buffer[1] = _BC_CO2_SENSOR_MODBUS_WRITE;
                self->_tx_buffer[2] = 0x00;
                self->_tx_buffer[3] = 0x80;
                self->_tx_buffer[4] = 0x01;
                self->_tx_buffer[5] = _BC_CO2_SENSOR_INITIAL_MEASUREMENT;
                self->_tx_buffer[6] = 0x28;
                self->_tx_buffer[7] = 0x7e;

                length = 8;
            }
            else
            {
                bc_log_debug("LP8: Preparing challenge");

                uint16_t crc16;

                self->_tx_buffer[0] = _BC_CO2_SENSOR_MODBUS_DEVICE_ADDRESS;
                self->_tx_buffer[1] = _BC_CO2_SENSOR_MODBUS_WRITE;
                self->_tx_buffer[2] = 0x00;
                self->_tx_buffer[3] = 0x80;
                self->_tx_buffer[4] = 0x1a;

                self->_calibration_run = self->_tick_calibration_timeout < bc_tick_get();

                if (self->_calibration_run)
                {
                    self->_tx_buffer[5] = self->_calibration;
                }
                else
                {
                    self->_tx_buffer[5] = _BC_CO2_SENSOR_SEQUENTIAL_MEASUREMENT;
                }

                memcpy(&self->_tx_buffer[6], self->_sensor_state, 23);

                self->_tx_buffer[29] = self->_pressure >> 8;
                self->_tx_buffer[30] = self->_pressure;

                crc16 = _bc_co2_sensor_calculate_crc16(self->_tx_buffer, 31);

                self->_tx_buffer[31] = crc16;
                self->_tx_buffer[32] = crc16 >> 8;

                length = 33;
            }

            if (!self->_driver->uart_enable(true))
            {
                bc_log_error("LP8: UART enable failed");

                self->_state = BC_CO2_SENSOR_STATE_ERROR;

                line = __LINE__; goto start;
            }

            if (self->_driver->uart_write(self->_tx_buffer, length) != length)
            {
                bc_log_error("LP8: UART write failed");

                self->_state = BC_CO2_SENSOR_STATE_ERROR;

                line = __LINE__; goto start;
            }

            self->_rx_buffer_length = 0;

            self->_state = BC_CO2_SENSOR_STATE_BOOT_READ;

            // TODO This timeout has been changed from 70 ms - test - BC_CO2_SENSOR_STATE_BOOT_READ timed out
            self->_tick_timeout = bc_tick_get() + 80;

            bc_scheduler_plan_current_from_now(10);

            return;
        }
        case BC_CO2_SENSOR_STATE_BOOT_READ:
        {
            bc_log_debug("LP8: Processing state BC_CO2_SENSOR_STATE_BOOT_READ");

            self->_rx_buffer_length += self->_driver->uart_read(self->_rx_buffer + self->_rx_buffer_length, 4 - self->_rx_buffer_length);

            if (self->_rx_buffer_length == 4)
            {
                if (!self->_driver->uart_enable(false))
                {
                    bc_log_error("LP8: UART disable failed");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (self->_rx_buffer[0] != _BC_CO2_SENSOR_MODBUS_DEVICE_ADDRESS)
                {
                    bc_log_error("LP8: Unexpected answer");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (self->_rx_buffer[1] != self->_tx_buffer[1])
                {
                    bc_log_error("LP8: Unexpected answer");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (_bc_co2_sensor_calculate_crc16(self->_rx_buffer, 4) != 0)
                {
                    bc_log_error("LP8: Unexpected answer");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                self->_state = BC_CO2_SENSOR_STATE_MEASURE;

                self->_tick_timeout = bc_tick_get() + (self->_calibration_run ? 1000 : 250);

                bc_scheduler_plan_current_from_now(50);

                return;
            }

            if (bc_tick_get() >= self->_tick_timeout)
            {
                bc_log_error("LP8: State timeout");

                self->_state = BC_CO2_SENSOR_STATE_ERROR;

                line = __LINE__; goto start;
            }

            bc_scheduler_plan_current_from_now(10);

            return;
        }
        case BC_CO2_SENSOR_STATE_MEASURE:
        {
            bc_log_debug("LP8: Processing state BC_CO2_SENSOR_STATE_MEASURE");

            bool rdy_pin_value;

            if (!self->_driver->rdy(&rdy_pin_value))
            {
                bc_log_error("LP8: Signal RDY read failed");

                self->_state = BC_CO2_SENSOR_STATE_ERROR;

                line = __LINE__; goto start;
            }

            if (!rdy_pin_value)
            {
                if (bc_tick_get() >= self->_tick_timeout)
                {
                    bc_log_error("LP8: Signal RDY timed out");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }
                else
                {
                    bc_log_debug("LP8: Signal RDY not asserted yet...");

                    bc_scheduler_plan_current_from_now(10);

                    return;
                }
            }

            self->_tx_buffer[0] = _BC_CO2_SENSOR_MODBUS_DEVICE_ADDRESS;
            self->_tx_buffer[1] = _BC_CO2_SENSOR_MODBUS_READ;
            self->_tx_buffer[2] = 0x00;
            self->_tx_buffer[3] = 0x80;
            self->_tx_buffer[4] = 0x2c;

            uint16_t crc16 = _bc_co2_sensor_calculate_crc16(self->_tx_buffer, 5);

            self->_tx_buffer[5] = crc16;
            self->_tx_buffer[6] = crc16 >> 8;

            if (!self->_driver->uart_enable(true))
            {
                self->_state = BC_CO2_SENSOR_STATE_ERROR;

                line = __LINE__; goto start;
            }

            if (self->_driver->uart_write(self->_tx_buffer,  7) !=  7)
            {
                self->_state = BC_CO2_SENSOR_STATE_ERROR;

                line = __LINE__; goto start;
            }

            self->_rx_buffer_length = 0;

            self->_state = BC_CO2_SENSOR_STATE_MEASURE_READ;

            self->_tick_timeout = bc_tick_get() + 100;

            bc_scheduler_plan_current_from_now(10);

            return;
        }
        case BC_CO2_SENSOR_STATE_MEASURE_READ:
        {
            bc_log_debug("LP8: Processing state BC_CO2_SENSOR_STATE_MEASURE_READ");

            self->_rx_buffer_length += self->_driver->uart_read(self->_rx_buffer + self->_rx_buffer_length, 49 - self->_rx_buffer_length);

            if (self->_rx_buffer_length == 49)
            {
                if (!self->_driver->uart_enable(false))
                {
                    bc_log_error("LP8: UART enable failed");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (!self->_driver->enable(false))
                {
                    bc_log_error("LP8: Power enable failed");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (self->_rx_buffer[0] != _BC_CO2_SENSOR_MODBUS_DEVICE_ADDRESS)
                {
                    bc_log_error("LP8: Unexpected answer");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (self->_rx_buffer[1] != self->_tx_buffer[1])
                {
                    bc_log_error("LP8: Unexpected answer");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (_bc_co2_sensor_calculate_crc16(self->_rx_buffer, 49) != 0)
                {
                    bc_log_error("LP8: CRC mismatch");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }

                if (self->_rx_buffer[_BC_CO2_SENSOR_RX_ERROR_STATUS0] != 0)
                {
                    if (self->_calibration_run)
                    {
                        bc_log_debug("LP8: Calibration run (error status not clear)");
                        if ((self->_rx_buffer[_BC_CO2_SENSOR_RX_ERROR_STATUS0] == 8) &&
                            (self->_calibration != BC_CO2_SENSOR_CALIBRATION_ABC) &&
                            (self->_calibration != BC_CO2_SENSOR_CALIBRATION_ABC_RF))
                        {
                            bc_log_debug("LP8: Rescheduling task...");

                            self->_state = BC_CO2_SENSOR_STATE_CHARGE;

                            bc_scheduler_plan_relative(self->_task_id_measure, 100);

                            return;
                        }
                    }
                    else
                    {
                        bc_log_error("LP8: Error status = %02x", self->_rx_buffer[_BC_CO2_SENSOR_RX_ERROR_STATUS0]);

                        self->_state = BC_CO2_SENSOR_STATE_ERROR;

                        line = __LINE__; goto start;
                    }
                }

                if (self->_calibration_run)
                {
                    self->_calibration = BC_CO2_SENSOR_CALIBRATION_ABC;

                    self->_tick_calibration_timeout = bc_tick_get() + _BC_CO2_SENSOR_CALIBRATION_TIMEOUT;
                }

                memcpy(self->_sensor_state, &self->_rx_buffer[4], 23);

                self->_first_measurement_done = true;

                self->_concentration = (int16_t) self->_rx_buffer[29] << 8;
                self->_concentration |= (int16_t) self->_rx_buffer[30];

                self->_valid = true;

                self->_state = BC_CO2_SENSOR_STATE_READY;

                bc_log_debug("LP8: Successful measurement");

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(BC_CO2_SENSOR_EVENT_UPDATE, self->_event_param);
                }

                return;
            }
            else
            {
                if (bc_tick_get() >= self->_tick_timeout)
                {
                    bc_log_error("LP8: State timeout");

                    self->_state = BC_CO2_SENSOR_STATE_ERROR;

                    line = __LINE__; goto start;
                }
            }

            bc_log_debug("LP8: Rescheduling task...");

            bc_scheduler_plan_current_from_now(10);

            return;
        }
        default:
        {
            bc_log_error("LP8: Unknown state");

            return;
        }
    }
}

static uint16_t _bc_co2_sensor_calculate_crc16(uint8_t *buffer, uint8_t length)
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
