#include <twr_flood_detector.h>

#define _TWR_FLOOD_DETECTOR_DELAY_RUN 50

static void _twr_flood_task_interval(void *param);

static void _twr_flood_task_measure(void *param);

void twr_flood_detector_init(twr_flood_detector_t *self, twr_flood_detector_type_t type)
{
	memset(self, 0, sizeof(&self));
	self->_type = type;
	self->_task_id_interval = twr_scheduler_register(_twr_flood_task_interval, self, TWR_TICK_INFINITY);
	self->_task_id_measure = twr_scheduler_register(_twr_flood_task_measure, self, _TWR_FLOOD_DETECTOR_DELAY_RUN);
}

void twr_flood_detector_set_event_handler(twr_flood_detector_t *self, void (*event_handler)(twr_flood_detector_t *, twr_flood_detector_event_t, void*), void *event_param)
{
	self->_event_handler = event_handler;
	self->_event_param = event_param;
}

void twr_flood_detector_set_update_interval(twr_flood_detector_t *self, twr_tick_t interval)
{
	self->_update_interval = interval;

    if (self->_update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(self->_task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);
    }
}

bool twr_flood_detector_measure(twr_flood_detector_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    twr_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool twr_flood_detector_is_alarm(twr_flood_detector_t *self)
{
	return self->_alarm;
}

static void _twr_flood_task_interval(void *param)
{
	twr_flood_detector_t *self = (twr_flood_detector_t *) param;

	twr_flood_detector_measure(self);

    twr_scheduler_plan_current_relative(self->_update_interval);
}

static void _twr_flood_task_measure(void *param)
{
	twr_flood_detector_t *self = (twr_flood_detector_t *) param;

start:

	switch (self->_state)
	{
		case TWR_FLOOD_DETECTOR_STATE_ERROR:
		{
			self->_measurement_active = false;

			if (self->_event_handler != NULL)
			{
				self->_event_handler(self, TWR_FLOOD_DETECTOR_EVENT_ERROR, self->_event_param);
			}

			self->_state = TWR_FLOOD_DETECTOR_STATE_INITIALIZE;

			return;
		}
		case TWR_FLOOD_DETECTOR_STATE_INITIALIZE:
		{
			self->_state = TWR_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					if (!twr_module_sensor_init())
					{
						goto start;
					}
					twr_gpio_init(TWR_GPIO_P4);
					twr_gpio_set_mode(TWR_GPIO_P4, TWR_GPIO_MODE_INPUT);
					break;
				}
				case TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					if (!twr_module_sensor_init())
					{
						goto start;
					}
					twr_gpio_init(TWR_GPIO_P5);
					twr_gpio_set_mode(TWR_GPIO_P5, TWR_GPIO_MODE_INPUT);
					break;
				}
				default:
				{
					goto start;
				}
			}

			self->_state = TWR_FLOOD_DETECTOR_STATE_READY;

			if (self->_measurement_active)
			{
				twr_scheduler_plan_current_now();
			}
			return;
		}
		case TWR_FLOOD_DETECTOR_STATE_READY:
		{
			self->_state = TWR_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_A, TWR_MODULE_SENSOR_PULL_UP_4K7))
					{
						goto start;
					}
					twr_scheduler_plan_current_from_now(5);
					break;
				}
				case TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_B, TWR_MODULE_SENSOR_PULL_UP_4K7))
					{
						goto start;
					}
					twr_scheduler_plan_current_from_now(5);
					break;
				}
				default:
				{
					goto start;
				}
			}

			self->_state = TWR_FLOOD_DETECTOR_STATE_MEASURE;
			return;
		}
		case TWR_FLOOD_DETECTOR_STATE_MEASURE:
		{
			self->_state = TWR_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					self->_alarm = twr_gpio_get_input(TWR_GPIO_P4) == 1;

					if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_A, TWR_MODULE_SENSOR_PULL_NONE))
					{
						goto start;
					}
					break;
				}
				case TWR_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					self->_alarm = twr_gpio_get_input(TWR_GPIO_P5) == 1;

					if (!twr_module_sensor_set_pull(TWR_MODULE_SENSOR_CHANNEL_B, TWR_MODULE_SENSOR_PULL_NONE))
					{
						goto start;
					}
					break;
				}
				default:
				{
					goto start;
				}
			}
			self->_measurement_active = false;

			if (self->_event_handler != NULL)
			{
				self->_event_handler(self, TWR_FLOOD_DETECTOR_EVENT_UPDATE, self->_event_param);
			}

			self->_state = TWR_FLOOD_DETECTOR_STATE_READY;

		}
		default:
			break;
	}
}
