#include <bc_ds18b20.h>
#include <bc_scheduler.h>

#define _BC_DS18B20_DELAY_RUN 50
#define _BC_DS18B20_DELAY_INITIALIZATION 10

static void _bc_ds18b20_task_interval(void *param);
static void _bc_ds18b20_task_measure(void *param);
static bool _bc_ds18b20_read_scratchpad(bc_ds18b20_t *self);

static bc_tick_t _bc_ds18b20_conversion_time_tab[] =
{
	[BC_DS18B20_BIT_RESOLUTION_9]  = 94,
	[BC_DS18B20_BIT_RESOLUTION_10] = 187,
	[BC_DS18B20_BIT_RESOLUTION_11] = 375,
	[BC_DS18B20_BIT_RESOLUTION_12] = 750
};

bool bc_ds18b20_init(bc_ds18b20_t *self, uint64_t device_number, bc_module_sensor_channel_t channel)
{
	memset(self, 0, sizeof(*self));
	self->_device_number = device_number;
	self->_channel = channel;
	self->_gpio_channel = channel == BC_MODULE_SENSOR_CHANNEL_A ? BC_GPIO_P4 : BC_GPIO_P5;

	bc_1wire_init(self->_gpio_channel);

    self->_task_id_interval = bc_scheduler_register(_bc_ds18b20_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_ds18b20_task_measure, self, _BC_DS18B20_DELAY_RUN);

	return true;
}

void bc_ds18b20_set_event_handler(bc_ds18b20_t *self, void (*event_handler)(bc_ds18b20_t *, bc_ds18b20_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_ds18b20_set_update_interval(bc_ds18b20_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_ds18b20_measure(self);
    }
}

bool bc_ds18b20_measure(bc_ds18b20_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

bool bc_ds18b20_get_temperature_raw(bc_ds18b20_t *self, int16_t *raw)
{
	if (!self->_temperature_valid)
	{
		return false;
	}

	*raw = (((int16_t) self->_scratchpad[1]) << 11) | ((int16_t) self->_scratchpad[3]);

	return true;
}

bool bc_ds18b20_get_temperature_celsius(bc_ds18b20_t *self, float *celsius)
{
	int16_t raw;

	if (!bc_ds18b20_get_temperature_raw(self, &raw))
	{
		return false;
	}

	*celsius = (float) raw / 64.f;

	return true;
}

bool bc_ds18b20_get_resolution(bc_ds18b20_t *self, bc_ds18b20_bit_resolution_t *resolution)
{
	if (self->_scratchpad[4] == 0x00)
	{
		return false;
	}

	*resolution = (bc_ds18b20_bit_resolution_t)((self->_scratchpad[4] >> 5) & 0x03);

	return true;
}

bool bc_ds18b20_get_conversion_time(bc_ds18b20_t *self, bc_tick_t *conversion_time)
{
	bc_ds18b20_bit_resolution_t resolution;

	if (!bc_ds18b20_get_resolution(self, &resolution))
	{
		return false;
	}

	*conversion_time = _bc_ds18b20_conversion_time_tab[resolution];

	return true;
}

static void _bc_ds18b20_task_interval(void *param)
{
	bc_ds18b20_t *self = param;

    bc_ds18b20_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_ds18b20_task_measure(void *param)
{
	bc_ds18b20_t *self = param;

start:

	switch (self->_state)
	{
		case BC_DS18B20_STATE_ERROR:
		{
			self->_temperature_valid = false;

			self->_measurement_active = false;

			if (self->_event_handler != NULL)
			{
				self->_event_handler(self, BC_DS18B20_EVENT_ERROR, self->_event_param);
			}

			self->_state = BC_DS18B20_STATE_INITIALIZE;

			return;
		}
		case BC_DS18B20_STATE_INITIALIZE:
		{
			self->_state = BC_DS18B20_STATE_ERROR;

			if (!bc_module_sensor_init())
			{
				goto start;
			}

			if (!bc_module_sensor_set_pull(self->_channel, BC_MODULE_SENSOR_PULL_UP_4K7))
			{
				goto start;
			}

			self->_state = BC_DS18B20_STATE_READ_SCRATCHPAD;

			bc_scheduler_plan_current_absolute(bc_tick_get() + _BC_DS18B20_DELAY_INITIALIZATION);

			return;
		}
		case BC_DS18B20_STATE_READ_SCRATCHPAD:
		{
			self->_state = BC_DS18B20_STATE_ERROR;

			if (!_bc_ds18b20_read_scratchpad(self))
			{
				goto start;
			}

			self->_state = BC_DS18B20_STATE_MEASURE;

			self->_tick_ready = bc_tick_get() + _BC_DS18B20_DELAY_INITIALIZATION;

			if (self->_measurement_active)
			{
				bc_scheduler_plan_current_absolute(0);
			}

			return;
		}
		case BC_DS18B20_STATE_MEASURE:
		{
			self->_state = BC_DS18B20_STATE_ERROR;

			bc_tick_t conversion_time;
			if (!bc_ds18b20_get_conversion_time(self, &conversion_time))
			{
				goto start;
			}

			if (!bc_1wire_reset(self->_gpio_channel))
			{
				goto start;
			}

			bc_1wire_select(self->_gpio_channel, &self->_device_number);

			bc_1wire_write_8b(self->_gpio_channel, 0x44);

			self->_state = BC_DS18B20_STATE_READ;

			bc_scheduler_plan_current_relative(conversion_time);

			return;
		}
		case BC_DS18B20_STATE_READ:
		{
			self->_state = BC_DS18B20_STATE_ERROR;

			if (!_bc_ds18b20_read_scratchpad(self))
			{
				goto start;
			}

			self->_temperature_valid = true;

			self->_state = BC_DS18B20_STATE_UPDATE;

			goto start;
		}

		case BC_DS18B20_STATE_UPDATE:
		{
			self->_measurement_active = false;

			if (self->_event_handler != NULL)
			{
				self->_event_handler(self, BC_DS18B20_EVENT_UPDATE, self->_event_param);
			}

			self->_state = BC_DS18B20_STATE_MEASURE;

			return;
		}
		default:
		{
			self->_state = BC_DS18B20_STATE_ERROR;

			goto start;
		}
	}
}

static bool _bc_ds18b20_read_scratchpad(bc_ds18b20_t *self)
{
	if (!bc_1wire_reset(self->_gpio_channel))
	{
		return false;
	}

	bc_1wire_select(self->_gpio_channel, &self->_device_number);

	bc_1wire_write_8b(self->_gpio_channel, 0xBE);

	bc_1wire_read(self->_gpio_channel, self->_scratchpad, sizeof(self->_scratchpad));

	return self->_scratchpad[4] != 0x00;
}
