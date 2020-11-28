#include <hio_ds18b20.h>
#include <hio_onewire.h>
#include <hio_gpio.h>
#include <hio_i2c.h>
#include <hio_module_sensor.h>

#define _HIO_DS18B20_SCRATCHPAD_SIZE 9
#define _HIO_DS18B20_DELAY_RUN 5000

static hio_tick_t _hio_ds18b20_lut_delay[] = {
    [HIO_DS18B20_RESOLUTION_BITS_9] = 100,
    [HIO_DS18B20_RESOLUTION_BITS_10] = 190,
    [HIO_DS18B20_RESOLUTION_BITS_11] = 380,
    [HIO_DS18B20_RESOLUTION_BITS_12] = 760
};

static void _hio_ds18b20_task_interval(void *param);

static void _hio_ds18b20_task_measure(void *param);

static int _hio_ds18b20_power_semaphore = 0;

void hio_ds18b20_init_single(hio_ds18b20_t *self, hio_ds18b20_resolution_bits_t resolution)
{
    static hio_ds18b20_sensor_t sensors[1];
    hio_module_sensor_onewire_power_up();
    hio_ds18b20_init(self, hio_module_sensor_get_onewire(), sensors, 1, resolution);
    self->_power = true;
}

void hio_ds18b20_init_multiple(hio_ds18b20_t *self, hio_ds18b20_sensor_t *sensors, int sensor_count, hio_ds18b20_resolution_bits_t resolution)
{
    hio_module_sensor_onewire_power_up();
    hio_ds18b20_init(self, hio_module_sensor_get_onewire(), sensors, sensor_count, resolution);
    self->_power = true;
}

void hio_ds18b20_init(hio_ds18b20_t *self, hio_onewire_t *onewire, hio_ds18b20_sensor_t *sensors, int sensor_count, hio_ds18b20_resolution_bits_t resolution)
{
    memset(self, 0, sizeof(*self));

    self->_onewire = onewire;

    self->_resolution = resolution;
    self->_sensor = sensors;
    self->_sensor_count = sensor_count;

    self->_task_id_interval = hio_scheduler_register(_hio_ds18b20_task_interval, self, HIO_TICK_INFINITY);
    self->_task_id_measure = hio_scheduler_register(_hio_ds18b20_task_measure, self, _HIO_DS18B20_DELAY_RUN);
}

void hio_ds18b20_set_event_handler(hio_ds18b20_t *self,
        void (*event_handler)(hio_ds18b20_t *, uint64_t _device_address, hio_ds18b20_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_ds18b20_set_update_interval(hio_ds18b20_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        hio_ds18b20_measure(self);
    }
}

bool hio_ds18b20_measure(hio_ds18b20_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    hio_scheduler_plan_absolute(self->_task_id_measure, _HIO_DS18B20_DELAY_RUN);

    return true;
}

int hio_ds18b20_get_index_by_device_address(hio_ds18b20_t *self, uint64_t device_address)
{
    for (int i = 0; i < self->_sensor_found; i++)
    {
        if (self->_sensor[i]._device_address == device_address)
        {
            return i;
        }
    }

    return -1;
}

uint64_t hio_ds182b0_get_short_address(hio_ds18b20_t *self, uint8_t index)
{
    if (index >= self->_sensor_found)
    {
        return 0;
    }

    uint64_t short_address = self->_sensor[index]._device_address;
    short_address &= ~(((uint64_t) 0xff) << 56);
    short_address >>= 8;

    return short_address;
}

int hio_ds18b20_get_sensor_found(hio_ds18b20_t *self)
{
    return self->_sensor_found;
}

void hio_ds18b20_set_power_dynamic(hio_ds18b20_t *self, bool on)
{
    self->_power_dynamic = on;
}

bool hio_ds18b20_get_temperature_raw(hio_ds18b20_t *self, uint64_t device_address, int16_t *raw)
{
    int sensor_index = hio_ds18b20_get_index_by_device_address(self, device_address);

    if (sensor_index == -1)
    {
        return false;
    }

	if (!self->_sensor[sensor_index]._temperature_valid)
	{
		return false;
	}

    *raw = self->_sensor[sensor_index]._temperature_raw;

	return true;
}

bool hio_ds18b20_get_temperature_celsius(hio_ds18b20_t *self, uint64_t device_address, float *celsius)
{
    int sensor_index = hio_ds18b20_get_index_by_device_address(self, device_address);

    if (sensor_index == -1)
    {
        return false;
    }

    if (!self->_sensor[sensor_index]._temperature_valid)
    {
        return false;
    }

    if ((device_address & 0xFF) == 0x10)
    {
        // If sensor is DS18S20 with family code 0x10, divide by 2 because it is only 9bit sensor
        *celsius = (float) self->_sensor[sensor_index]._temperature_raw / 2;
    }
    else
    {
        *celsius = (float) self->_sensor[sensor_index]._temperature_raw / 16;
    }

    return true;
}

static void _hio_ds18b20_task_interval(void *param)
{
    hio_ds18b20_t *self = param;

    hio_ds18b20_measure(self);

    hio_scheduler_plan_current_relative(self->_update_interval);
}

static bool _hio_ds18b20_power_up(hio_ds18b20_t *self)
{
    if (!self->_power_dynamic) // If power dynamic equal False, can't power down
    {
        return true;
    }

    if (self->_power)
    {
        return true;
    }

    if (_hio_ds18b20_power_semaphore == 0)
    {
        hio_module_sensor_init();

        if (hio_module_sensor_get_revision() == HIO_MODULE_SENSOR_REVISION_R1_1)
        {
            hio_module_sensor_set_vdd(1);
        }
        else
        {
            hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_A, HIO_MODULE_SENSOR_PULL_UP_56R);
        }

        if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_B, HIO_MODULE_SENSOR_PULL_UP_4K7))
        {
            return false;
        }
    }

    _hio_ds18b20_power_semaphore++;

    self->_power = true;

    return true;
}

static bool _hio_ds18b20_power_down(hio_ds18b20_t *self)
{
    if (!self->_power_dynamic) // If power dynamic equal False, can't power down
    {
        return true;
    }

    if (!self->_power)
    {
        return true;
    }

    if (_hio_ds18b20_power_semaphore == 1)
    {
        hio_module_sensor_init();

        if (hio_module_sensor_get_revision() == HIO_MODULE_SENSOR_REVISION_R1_1)
        {
            hio_module_sensor_set_vdd(0);
        }
        else
        {
            hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_A, HIO_MODULE_SENSOR_PULL_NONE);
        }

        if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_B, HIO_MODULE_SENSOR_PULL_NONE))
        {
            return false;
        }
    }

    _hio_ds18b20_power_semaphore--;

    self->_power = false;

    return true;
}

static bool _hio_ds18b20_is_scratchpad_valid(uint8_t *scratchpad)
{
    #ifdef HIO_DS18B20_STRICT_VALIDATION
    if (scratchpad[5] != 0xff)
    {
        return false;
    }

    if (scratchpad[7] != 0x10)
    {
        return false;
    }
    #endif

    if (hio_onewire_crc8(scratchpad, _HIO_DS18B20_SCRATCHPAD_SIZE, 0) != 0)
    {
        return false;
    }

    return true;
}

static void _hio_ds18b20_task_measure(void *param)
{
    hio_ds18b20_t *self = param;

    start:

    switch (self->_state)
    {
        case HIO_DS18B20_STATE_ERROR:
        {
            for (int i = 0; i < self->_sensor_found; i++)
            {
                self->_sensor[i]._temperature_valid = false;
            }

            self->_measurement_active = false;

            _hio_ds18b20_power_down(self);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, 0, HIO_DS18B20_EVENT_ERROR, self->_event_param);
            }

            self->_state = HIO_DS18B20_STATE_PREINITIALIZE;

            return;
        }
        case HIO_DS18B20_STATE_PREINITIALIZE:
        {
            self->_state = HIO_DS18B20_STATE_ERROR;

            if (self->_power_dynamic)
            {
                hio_module_sensor_set_mode(HIO_MODULE_SENSOR_CHANNEL_B, HIO_MODULE_SENSOR_MODE_INPUT);

                if (!_hio_ds18b20_power_up(self))
                {
                    goto start;
                }
            }

            self->_state = HIO_DS18B20_STATE_INITIALIZE;

            hio_scheduler_plan_current_from_now(10);

            break;
        }
        case HIO_DS18B20_STATE_INITIALIZE:
        {
            self->_state = HIO_DS18B20_STATE_ERROR;

            uint64_t _device_address = 0;
            self->_sensor_found = 0;


            hio_onewire_search_start(self->_onewire, 0);
            while ((self->_sensor_found < self->_sensor_count) && hio_onewire_search_next(self->_onewire, &_device_address))
            {
                self->_sensor[self->_sensor_found]._device_address = _device_address;

                _device_address++;
                self->_sensor_found++;
            }

            if (self->_sensor_found == 0)
            {
                // hio_onewire_transaction_stop(self->_onewire);
                goto start;
            }

            hio_onewire_transaction_start(self->_onewire);

            // Write Scratchpad
            if (!hio_onewire_reset(self->_onewire))
            {
                hio_onewire_transaction_stop(self->_onewire);

                goto start;
            }

            hio_onewire_skip_rom(self->_onewire);

            uint8_t buffer[] = {0x4e, 0x75, 0x70, self->_resolution << 5 | 0x1f};

            hio_onewire_write(self->_onewire, buffer, sizeof(buffer));

            hio_onewire_transaction_stop(self->_onewire);

            if (self->_measurement_active)
            {
                hio_scheduler_plan_current_now();
            }
            else
            {
                if (!_hio_ds18b20_power_down(self))
                {
                    goto start;
                }
            }

            self->_state = HIO_DS18B20_STATE_READY;

            return;
        }
        case HIO_DS18B20_STATE_READY:
        {
            self->_state = HIO_DS18B20_STATE_ERROR;

            if (!_hio_ds18b20_power_up(self))
            {
                goto start;
            }

            self->_state = HIO_DS18B20_STATE_MEASURE;

            hio_scheduler_plan_current_from_now(10);

            return;
        }
        case HIO_DS18B20_STATE_MEASURE:
        {
            self->_state = HIO_DS18B20_STATE_ERROR;

            hio_onewire_transaction_start(self->_onewire);

            if (!hio_onewire_reset(self->_onewire))
            {
            	hio_onewire_transaction_stop(self->_onewire);
                goto start;
            }

            //hio_onewire_select(self->_onewire, &self->_device_address);
            hio_onewire_skip_rom(self->_onewire);

            hio_onewire_write_byte(self->_onewire, 0x44);

            hio_onewire_transaction_stop(self->_onewire);

            self->_state = HIO_DS18B20_STATE_READ;

            hio_scheduler_plan_current_from_now(_hio_ds18b20_lut_delay[self->_resolution]);

            return;
        }
        case HIO_DS18B20_STATE_READ:
        {
            self->_state = HIO_DS18B20_STATE_ERROR;

            uint8_t scratchpad[_HIO_DS18B20_SCRATCHPAD_SIZE];

            for (int i = 0; i < self->_sensor_found; i++)
            {
                hio_onewire_transaction_start(self->_onewire);

                if (!hio_onewire_reset(self->_onewire))
                {
                    hio_onewire_transaction_stop(self->_onewire);

                    goto start;
                }

                hio_onewire_select(self->_onewire, &self->_sensor[i]._device_address);

                hio_onewire_write_byte(self->_onewire, 0xBE);

                hio_onewire_read(self->_onewire, scratchpad, sizeof(scratchpad));

                hio_onewire_transaction_stop(self->_onewire);


                if (!_hio_ds18b20_is_scratchpad_valid(scratchpad))
                {
                    goto start;
                }

                self->_sensor[i]._temperature_raw = ((int16_t) scratchpad[1]) << 8 | ((int16_t) scratchpad[0]);
                self->_sensor[i]._temperature_valid = true;
            }

            if (!_hio_ds18b20_power_down(self))
            {
                goto start;
            }

            self->_state = HIO_DS18B20_STATE_UPDATE;

            goto start;
        }
        case HIO_DS18B20_STATE_UPDATE:
        {
            self->_measurement_active = false;

            self->_state = HIO_DS18B20_STATE_READY;

            for (int i = 0; i < self->_sensor_found; i++)
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, self->_sensor[i]._device_address, HIO_DS18B20_EVENT_UPDATE, self->_event_param);
                }
            }

            return;
        }
        default:
        {
            self->_state = HIO_DS18B20_STATE_ERROR;

            goto start;
        }
    }
}

