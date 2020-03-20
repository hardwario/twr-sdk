#include <bc_soil_sensor.h>
#include <bc_module_sensor.h>
#include <bc_timer.h>
#include <bc_i2c.h>
#include <bc_system.h>
#include <bc_tick.h>

#define _BC_SOIL_SENSOR_TMP112_ADDRESS   0x48
#define _BC_SOIL_SENSOR_ZSSC3123_ADDRESS 0x28
#define _BC_SOIL_SENSOR_EEPROM_ADDRESS   0x51
#define _BC_SOIL_SENSOR_EEPROM_BANK_A    0x000
#define _BC_SOIL_SENSOR_EEPROM_BANK_B    0x080
#define _BC_SOIL_SENSOR_EEPROM_BANK_C    0x100

static void _bc_soil_sensor_task_interval(void *param);
static void _bc_soil_sensor_error(bc_soil_sensor_t *self, bc_soil_sensor_error_t error);
static void _bc_soil_sensor_task_measure(void *param);
static bool _bc_soil_sensor_tmp112_init(bc_ds28e17_t *ds28e17);
static bool _bc_soil_sensor_tmp112_measurement_request(bc_ds28e17_t *ds28e17);
static bool _bc_soil_sensor_tmp112_data_fetch(bc_soil_sensor_sensor_t *sensor);
static bool _bc_soil_sensor_zssc3123_measurement_request(bc_ds28e17_t *ds28e17);
static bool _bc_soil_sensor_zssc3123_data_fetch(bc_soil_sensor_sensor_t *sensor);
static bc_soil_sensor_error_t _bc_soil_sensor_eeprom_load(bc_soil_sensor_sensor_t *sensor);
static void _bc_soil_sensor_eeprom_fill(bc_soil_sensor_sensor_t *sensor);
static bool _bc_soil_sensor_eeprom_save(bc_soil_sensor_sensor_t *sensor);
static bool _bc_soil_sensor_eeprom_read(bc_soil_sensor_sensor_t *sensor, uint8_t address, void *buffer, size_t length);
static bool _bc_soil_sensor_eeprom_write(bc_soil_sensor_sensor_t *sensor, uint8_t address, const void *buffer, size_t length);

void bc_soil_sensor_init(bc_soil_sensor_t *self)
{
    static bc_soil_sensor_sensor_t sensor[1];

    bc_soil_sensor_init_multiple(self, sensor, 1);
}

void bc_soil_sensor_init_multiple(bc_soil_sensor_t *self, bc_soil_sensor_sensor_t *sensors, int sensor_count)
{
    memset(self, 0, sizeof(*self));

    self->_channel = BC_GPIO_P5;
    self->_sensor = sensors;
    self->_sensor_count = sensor_count;

    self->_task_id_interval = bc_scheduler_register(_bc_soil_sensor_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_soil_sensor_task_measure, self, 10);

    bc_onewire_init(self->_channel);

    bc_onewire_auto_ds28e17_sleep_mode(true);
}

void bc_soil_sensor_set_event_handler(bc_soil_sensor_t *self, void (*event_handler)(bc_soil_sensor_t *, uint64_t device_address, bc_soil_sensor_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;

    self->_event_param = event_param;
}

void bc_soil_sensor_set_update_interval(bc_soil_sensor_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_soil_sensor_measure(self);
    }
}

int bc_soil_sensor_get_sensor_found(bc_soil_sensor_t *self)
{
    return self->_sensor_found;
}

bool bc_soil_sensor_measure(bc_soil_sensor_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool bc_soil_sensor_get_temperature_raw(bc_soil_sensor_t *self, uint64_t device_address, int16_t *raw)
{
    int i = bc_soil_sensor_get_index_by_device_address(self, device_address);

    if (i == -1)
    {
        return false;
    }

    if (!self->_sensor[i]._temperature_raw)
    {
        return false;
    }

    *raw = (int16_t) self->_sensor[i]._temperature_raw >> 4;

    return true;
}

bool bc_soil_sensor_get_temperature_celsius(bc_soil_sensor_t *self, uint64_t device_address, float *celsius)
{
    int16_t raw;

    if (!bc_soil_sensor_get_temperature_raw(self, device_address, &raw))
    {
        *celsius = NAN;

        return false;
    }

    *celsius = (float) raw / 16.f;

    return true;
}

bool bc_soil_sensor_get_temperature_fahrenheit(bc_soil_sensor_t *self, uint64_t device_address, float *fahrenheit)
{
    float celsius;

    if (!bc_soil_sensor_get_temperature_celsius(self, device_address, &celsius))
    {
        return false;
    }

    *fahrenheit = celsius * 1.8f + 32.f;

    return true;
}

bool bc_soil_sensor_get_temperature_kelvin(bc_soil_sensor_t *self, uint64_t device_address, float *kelvin)
{
    float celsius;

    if (!bc_soil_sensor_get_temperature_celsius(self, device_address, &celsius))
    {
        return false;
    }

    *kelvin = celsius + 273.15f;

    if (*kelvin < 0.f)
    {
        *kelvin = 0.f;
    }

    return true;
}

bool bc_soil_sensor_get_cap_raw(bc_soil_sensor_t *self, uint64_t device_address, uint16_t *raw)
{
    int i = bc_soil_sensor_get_index_by_device_address(self, device_address);

    if (i == -1)
    {
        return false;
    }

    if (!self->_sensor[i]._cap_valid)
    {
        return false;
    }

    *raw = self->_sensor[i]._cap_raw;

    return true;
}

bool bc_soil_sensor_get_moisture(bc_soil_sensor_t *self, uint64_t device_address, int *moisture)
{
    int i = bc_soil_sensor_get_index_by_device_address(self, device_address);

    if (i == -1)
    {
        return false;
    }

    if (!self->_sensor[i]._cap_valid)
    {
        return false;
    }

    uint16_t raw = self->_sensor[i]._cap_raw;
    uint16_t *calibration = self->_sensor[i]._eeprom.calibration;

    for (int i = 0; i < 11; i++)
    {
        if (raw < calibration[i])
        {
            if (i == 0)
            {
                *moisture = 0;

                return true;
            }

            *moisture = (((raw - calibration[i - 1]) * 10) / (calibration[i] - calibration[i - 1])) + (10 * (i - 1));

            return true;
        }
    }

    *moisture = 100;

    return true;
}

int bc_soil_sensor_get_index_by_device_address(bc_soil_sensor_t *self, uint64_t device_address)
{
    for (int i = 0; i < self->_sensor_found; i++)
    {
        if (self->_sensor[i]._ds28e17._device_number == device_address)
        {
            return i;
        }
    }

    return -1;
}

uint64_t bc_soil_sensor_get_device_address_by_index(bc_soil_sensor_t *self, int index)
{
    if (index >= self->_sensor_found)
    {
        return 0;
    }

    return self->_sensor[index]._ds28e17._device_number;
}

char *bc_soil_sensor_get_label(bc_soil_sensor_t *self, uint64_t device_address)
{
    int i = bc_soil_sensor_get_index_by_device_address(self, device_address);

    if (i == -1)
    {
        return false;
    }

    return self->_sensor[i]._eeprom.label;
}

bool bc_soil_sensor_set_label(bc_soil_sensor_t *self, uint64_t device_address, const char *label)
{
    int i = bc_soil_sensor_get_index_by_device_address(self, device_address);

    if (i == -1)
    {
        return false;
    }

    strncpy(self->_sensor[i]._eeprom.label, label, 16);

    return true;
}

bool bc_soil_sensor_calibration_set_point(bc_soil_sensor_t *self, uint64_t device_address, uint8_t point, uint16_t value)
{
    int i = bc_soil_sensor_get_index_by_device_address(self, device_address);

    if (i == -1)
    {
        return false;
    }

    point = point / 10;

    if (point > 10)
    {
        return false;
    }

    self->_sensor[i]._eeprom.calibration[point] = value;

    return true;
}

bool bc_soil_sensor_eeprom_save(bc_soil_sensor_t *self, uint64_t device_address)
{
    int i = bc_soil_sensor_get_index_by_device_address(self, device_address);

    if (i == -1)
    {
        return false;
    }

    return _bc_soil_sensor_eeprom_save(&self->_sensor[i]);
}

bc_soil_sensor_error_t bc_soil_sensor_get_error(bc_soil_sensor_t *self)
{
    return self->_error;
}

static void _bc_soil_sensor_task_interval(void *param)
{
    bc_soil_sensor_t *self = param;

    bc_soil_sensor_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_soil_sensor_error(bc_soil_sensor_t *self, bc_soil_sensor_error_t error)
{
    self->_error = error;
    self->_state = BC_SOIL_SENSOR_STATE_ERROR;
    bc_scheduler_plan_current_now();
}

static void _bc_soil_sensor_task_measure(void *param)
{
    bc_soil_sensor_t *self = param;

    switch (self->_state)
    {
        case BC_SOIL_SENSOR_STATE_ERROR:
        {
            bc_onewire_auto_ds28e17_sleep_mode(true);

            for (int i = 0; i < self->_sensor_found; i++)
            {
                self->_sensor[i]._temperature_valid = false;
                self->_sensor[i]._cap_valid = false;
            }

            self->_measurement_active = false;

            bc_module_sensor_onewire_power_down();

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, 0, BC_SOIL_SENSOR_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_SOIL_SENSOR_STATE_PREINITIALIZE;

            return;
        }
        case BC_SOIL_SENSOR_STATE_PREINITIALIZE:
        {
            if (!bc_module_sensor_init())
            {
                _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_SENSOR_MODULE_INITIALIZE);

                return;
            }

            if (!bc_module_sensor_onewire_power_up())
            {
                _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_SENSOR_MODULE_POWER_UP);

                return;
            }

            self->_state = BC_SOIL_SENSOR_STATE_INITIALIZE;

            bc_scheduler_plan_current_from_now(750);

            return;
        }
        case BC_SOIL_SENSOR_STATE_INITIALIZE:
        {
            uint64_t device_address = 0;

            self->_sensor_found = 0;

            bc_onewire_transaction_start(self->_channel);

            bc_onewire_reset(self->_channel);

            bc_onewire_search_start(0x19);

            while ((self->_sensor_found < self->_sensor_count) && bc_onewire_search_next(self->_channel, &device_address))
            {
                bc_ds28e17_init(&self->_sensor[self->_sensor_found]._ds28e17, self->_channel, device_address);

                self->_sensor_found++;
            }

            bc_onewire_transaction_stop(self->_channel);

            if (self->_sensor_found == 0)
            {
                _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_NO_SENSOR_FOUND);

                return;
            }

            bc_onewire_auto_ds28e17_sleep_mode(false);

            for (int i = 0; i < self->_sensor_found; i++)
            {
                if (!_bc_soil_sensor_tmp112_init(&self->_sensor[i]._ds28e17))
                {
                    _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_TMP112_INITIALIZE);

                    return;
                }
            }

            for (int i = 0; i < self->_sensor_found; i++)
            {
                bc_soil_sensor_error_t error = _bc_soil_sensor_eeprom_load(&self->_sensor[i]);

                if (error)
                {
                    if (error == BC_SOIL_SENSOR_ERROR_EEPROM_HEADER_READ)
                    {
                        _bc_soil_sensor_eeprom_fill(&self->_sensor[i]);

                        continue;
                    }

                    _bc_soil_sensor_error(self, error);

                    return;
                }
            }

            for (int i = 0; i < self->_sensor_found; i++)
            {
                if (i + 1 == self->_sensor_found) // last sensor
                {
                    bc_onewire_auto_ds28e17_sleep_mode(true);
                }

                _bc_soil_sensor_zssc3123_data_fetch(&self->_sensor[i]);

                self->_sensor[i]._cap_valid = false;
            }

            self->_state = BC_SOIL_SENSOR_STATE_READY;

            if (self->_measurement_active)
            {
                bc_scheduler_plan_current_now();
            }

            return;
        }
        case BC_SOIL_SENSOR_STATE_READY:
        {
            self->_state = BC_SOIL_SENSOR_STATE_MEASURE;

            bc_scheduler_plan_current_now();

            return;
        }
        case BC_SOIL_SENSOR_STATE_MEASURE:
        {
            bc_onewire_auto_ds28e17_sleep_mode(false);

            for (int i = 0; i < self->_sensor_found; i++)
            {
                if (!_bc_soil_sensor_zssc3123_measurement_request(&self->_sensor[i]._ds28e17))
                {
                    _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_ZSSC3123_MEASUREMENT_REQUEST);

                    return;
                }

                if (i + 1 == self->_sensor_found) // last sensor
                {
                    bc_onewire_auto_ds28e17_sleep_mode(true);
                }

                if (!_bc_soil_sensor_tmp112_measurement_request(&self->_sensor[i]._ds28e17))
                {
                    _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_TMP112_MEASUREMENT_REQUEST);

                    return;
                }

            }

            self->_state = BC_SOIL_SENSOR_STATE_READ;

            bc_scheduler_plan_current_from_now(50);

            return;
        }
        case BC_SOIL_SENSOR_STATE_READ:
        {
            bc_onewire_auto_ds28e17_sleep_mode(false);

            for (int i = 0; i < self->_sensor_found; i++)
            {
                if (!_bc_soil_sensor_tmp112_data_fetch(&self->_sensor[i]))
                {
                    _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_TMP112_DATA_FETCH);

                    return;
                }

                if (i + 1 == self->_sensor_found) // last sensor
                {
                    bc_onewire_auto_ds28e17_sleep_mode(true);
                }

                if (!_bc_soil_sensor_zssc3123_data_fetch(&self->_sensor[i]))
                {
                    _bc_soil_sensor_error(self, BC_SOIL_SENSOR_ERROR_ZSSC3123_DATA_FETCH);

                    return;
                }
            }

            self->_state = BC_SOIL_SENSOR_STATE_UPDATE;

            bc_scheduler_plan_current_now();

            return;
        }
        case BC_SOIL_SENSOR_STATE_UPDATE:
        {
            self->_measurement_active = false;

            self->_error = BC_SOIL_SENSOR_ERROR_NONE;

            if (self->_event_handler != NULL)
            {
                for (int i = 0; i < self->_sensor_found; i++)
                {
                    self->_event_handler(self, self->_sensor[i]._ds28e17._device_number, BC_SOIL_SENSOR_EVENT_UPDATE, self->_event_param);
                }
            }

            self->_state = BC_SOIL_SENSOR_STATE_READY;

            return;
        }
        default:
        {
            self->_state = BC_SOIL_SENSOR_STATE_ERROR;

            return;
        }
    }
}

static bool _bc_soil_sensor_tmp112_init(bc_ds28e17_t *ds28e17)
{
    const uint8_t buffer[] = { 0x01, 0x80 };

    bc_i2c_memory_transfer_t memory_transfer = {
        .device_address = _BC_SOIL_SENSOR_TMP112_ADDRESS,
        .memory_address = 0x01,
        .buffer = (void *) buffer,
        .length = sizeof(buffer)
    };

    return bc_ds28e17_memory_write(ds28e17, &memory_transfer);
}

static bool _bc_soil_sensor_tmp112_measurement_request(bc_ds28e17_t *ds28e17)
{
    const uint8_t buffer[] = { 0x81 };

    bc_i2c_memory_transfer_t memory_transfer = {
        .device_address = _BC_SOIL_SENSOR_TMP112_ADDRESS,
        .memory_address = 0x01,
        .buffer = (void *) buffer,
        .length = sizeof(buffer)
    };

    return bc_ds28e17_memory_write(ds28e17, &memory_transfer);
}

static bool _bc_soil_sensor_tmp112_data_fetch(bc_soil_sensor_sensor_t *sensor)
{
    uint8_t buffer[2];

    bc_i2c_memory_transfer_t memory_transfer = {
        .device_address = _BC_SOIL_SENSOR_TMP112_ADDRESS,
        .memory_address = 0x01,
        .buffer = buffer,
        .length = 1
    };

    if (!bc_ds28e17_memory_read(&sensor->_ds28e17, &memory_transfer))
    {
        return false;
    }

    if ((buffer[0] & 0x81) != 0x81)
    {
        return false;
    }

    memory_transfer.memory_address = 0x00;
    memory_transfer.length = 2;

    if (!bc_ds28e17_memory_read(&sensor->_ds28e17, &memory_transfer))
    {
        return false;
    }

    sensor->_temperature_raw = (uint16_t) buffer[0] << 8 | buffer[1];
    sensor->_temperature_valid = true;

    return true;
}

static bool _bc_soil_sensor_zssc3123_measurement_request(bc_ds28e17_t *ds28e17)
{
    const uint8_t buffer[] = { _BC_SOIL_SENSOR_ZSSC3123_ADDRESS << 1 };

    bc_i2c_transfer_t transfer = {
        .device_address = _BC_SOIL_SENSOR_ZSSC3123_ADDRESS,
        .buffer = (void *) buffer,
        .length = sizeof(buffer)
    };

    return bc_ds28e17_write(ds28e17, &transfer);
}

static bool _bc_soil_sensor_zssc3123_data_fetch(bc_soil_sensor_sensor_t *sensor)
{
    uint8_t buffer[2];

    bc_i2c_transfer_t transfer = {
        .device_address = _BC_SOIL_SENSOR_ZSSC3123_ADDRESS,
        .buffer = buffer,
        .length = sizeof(buffer)
    };

    if (!bc_ds28e17_read(&sensor->_ds28e17, &transfer))
    {
        return false;
    }

    if ((buffer[0] & 0xc0) == 0)
    {
        sensor->_cap_raw = (uint16_t) (buffer[0] & 0x3f) << 8 | buffer[1];

        sensor->_cap_valid = true;
    }

    return true;
}

static bc_soil_sensor_error_t _bc_soil_sensor_eeprom_load(bc_soil_sensor_sensor_t *sensor)
{
    bc_soil_sensor_eeprom_header_t header;

    if (!_bc_soil_sensor_eeprom_read(sensor, 0, &header, sizeof(header)))
    {
        return BC_SOIL_SENSOR_ERROR_EEPROM_HEADER_READ;
    }

    if (header.signature != 0xdeadbeef)
    {
        return BC_SOIL_SENSOR_ERROR_EEPROM_SIGNATURE;
    }

    if (header.version != 1)
    {
        return BC_SOIL_SENSOR_ERROR_EEPROM_VERSION;
    }

    if (header.length != sizeof(bc_soil_sensor_eeprom_t))
    {
        return BC_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_LENGTH;
    }

    if (!_bc_soil_sensor_eeprom_read(sensor, sizeof(header), &sensor->_eeprom, sizeof(bc_soil_sensor_eeprom_t)))
    {
        return BC_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_READ;
    }

    if (header.crc != bc_onewire_crc16(&sensor->_eeprom, sizeof(bc_soil_sensor_eeprom_t), 0))
    {
        return BC_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_CRC;
    }

    return BC_SOIL_SENSOR_ERROR_NONE;
}

static void _bc_soil_sensor_eeprom_fill(bc_soil_sensor_sensor_t *sensor)
{
    sensor->_eeprom.product = 0;
    sensor->_eeprom.revision = 0x0104;

    sensor->_eeprom.calibration[0] = 1700;
    sensor->_eeprom.calibration[10] = 3000;

    uint16_t step = (sensor->_eeprom.calibration[10] - sensor->_eeprom.calibration[0]) / 11;

    for (int i = 1; i < 10; i++)
    {
        sensor->_eeprom.calibration[i] = sensor->_eeprom.calibration[i - 1] + step;
    }

    memset(sensor->_eeprom.label, 0, sizeof(sensor->_eeprom.label));
}

static bool _bc_soil_sensor_eeprom_save(bc_soil_sensor_sensor_t *sensor)
{
    bc_soil_sensor_eeprom_header_t header = {
        .signature = 0xdeadbeef,
        .version = 1,
        .length = sizeof(bc_soil_sensor_eeprom_t),
        .crc = bc_onewire_crc16(&sensor->_eeprom, sizeof(bc_soil_sensor_eeprom_t), 0)
    };

    if (!_bc_soil_sensor_eeprom_write(sensor, 0, &header, sizeof(header)))
    {
        return false;
    }

    if (!_bc_soil_sensor_eeprom_write(sensor, sizeof(header), &sensor->_eeprom, sizeof(bc_soil_sensor_eeprom_t)))
    {
        return false;
    }

    return true;
}

static bool _bc_soil_sensor_eeprom_read(bc_soil_sensor_sensor_t *sensor, uint8_t address, void *buffer, size_t length)
{
    uint8_t a[8];
    uint8_t b[sizeof(a)];
    uint8_t c[sizeof(a)];

    if ((_BC_SOIL_SENSOR_EEPROM_BANK_A + address + length) >= _BC_SOIL_SENSOR_EEPROM_BANK_B)
    {
        return false;
    }

    bc_i2c_memory_transfer_t memory_transfer = {
        .device_address = _BC_SOIL_SENSOR_EEPROM_ADDRESS,
    };

    size_t j = 0;
    uint8_t *p = buffer;

    for (size_t i = 0; i < length; i += sizeof(a))
    {
        memory_transfer.length = length - i > sizeof(a) ? sizeof(a) : length - i;

        memory_transfer.memory_address = address + i + _BC_SOIL_SENSOR_EEPROM_BANK_A;
        memory_transfer.buffer = a;

        if (!bc_ds28e17_memory_read(&sensor->_ds28e17, &memory_transfer))
        {
            return false;
        }

        memory_transfer.memory_address = address + i + _BC_SOIL_SENSOR_EEPROM_BANK_B;
        memory_transfer.buffer = b;

        if (!bc_ds28e17_memory_read(&sensor->_ds28e17, &memory_transfer))
        {
            return false;
        }

        memory_transfer.memory_address = address + i + _BC_SOIL_SENSOR_EEPROM_BANK_C;
        memory_transfer.buffer = c;

        if (!bc_ds28e17_memory_read(&sensor->_ds28e17, &memory_transfer))
        {
            return false;
        }

        for (j = 0; j < memory_transfer.length; j++)
        {
            *p++ = (a[j] & b[j]) | (a[j] & c[j]) | (b[j] & c[j]);
        }
    }

    return true;
}

static bool _bc_soil_sensor_eeprom_write_chunk(bc_ds28e17_t *ds28e17, bc_i2c_memory_transfer_t *memory_transfer, void *test_buffer)
{
    uint8_t *original_buffer = memory_transfer->buffer;

    for (int i = 0;; i++)
    {
        memory_transfer->buffer = test_buffer;

        if (!bc_ds28e17_memory_read(ds28e17, memory_transfer))
        {
            return false;
        }

        memory_transfer->buffer = original_buffer;

        if (memcmp(original_buffer, test_buffer, memory_transfer->length) == 0)
        {
            return true;
        }

        if (i > 3)
        {
            break;
        }

        if (!bc_ds28e17_memory_write(ds28e17, memory_transfer))
        {
            return false;
        }
    }

    return false;
}

static bool _bc_soil_sensor_eeprom_write(bc_soil_sensor_sensor_t *sensor, uint8_t address, const void *buffer, size_t length)
{
    if ((_BC_SOIL_SENSOR_EEPROM_BANK_A + address + length) >= _BC_SOIL_SENSOR_EEPROM_BANK_B)
    {
        return false;
    }

    bc_i2c_memory_transfer_t memory_transfer = {
        .device_address = _BC_SOIL_SENSOR_EEPROM_ADDRESS,
    };

    uint8_t test[2];

    for (size_t i = 0; i < length; i += sizeof(test))
    {
        memory_transfer.length = length - i > sizeof(test) ? sizeof(test) : length - i;
        memory_transfer.memory_address = address + i + _BC_SOIL_SENSOR_EEPROM_BANK_A;
        memory_transfer.buffer = (uint8_t *) buffer + i;

        if (!_bc_soil_sensor_eeprom_write_chunk(&sensor->_ds28e17, &memory_transfer, test))
        {
            return false;
        }

        memory_transfer.memory_address = address + i + _BC_SOIL_SENSOR_EEPROM_BANK_B;

        if (!_bc_soil_sensor_eeprom_write_chunk(&sensor->_ds28e17, &memory_transfer, test))
        {
            return false;
        }

        memory_transfer.memory_address = address + i + _BC_SOIL_SENSOR_EEPROM_BANK_C;

        if (!_bc_soil_sensor_eeprom_write_chunk(&sensor->_ds28e17, &memory_transfer, test))
        {
            return false;
        }
    }

    return true;
}
