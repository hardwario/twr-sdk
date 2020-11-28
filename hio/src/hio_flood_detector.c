#include <hio_flood_detector.h>

#define _HIO_FLOOD_DETECTOR_DELAY_RUN 50

static void _hio_flood_task_interval(void *param);

static void _hio_flood_task_measure(void *param);

void hio_flood_detector_init(hio_flood_detector_t *self, hio_flood_detector_type_t type)
{
	memset(self, 0, sizeof(&self));
	self->_type = type;
	self->_task_id_interval = hio_scheduler_register(_hio_flood_task_interval, self, HIO_TICK_INFINITY);
	self->_task_id_measure = hio_scheduler_register(_hio_flood_task_measure, self, _HIO_FLOOD_DETECTOR_DELAY_RUN);
}

void hio_flood_detector_set_event_handler(hio_flood_detector_t *self, void (*event_handler)(hio_flood_detector_t *, hio_flood_detector_event_t, void*), void *event_param)
{
	self->_event_handler = event_handler;
	self->_event_param = event_param;
}

void hio_flood_detector_set_update_interval(hio_flood_detector_t *self, hio_tick_t interval)
{
	self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);
    }
}

bool hio_flood_detector_measure(hio_flood_detector_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    hio_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool hio_flood_detector_is_alarm(hio_flood_detector_t *self)
{
	return self->_alarm;
}

static void _hio_flood_task_interval(void *param)
{
	hio_flood_detector_t *self = (hio_flood_detector_t *) param;

	hio_flood_detector_measure(self);

    hio_scheduler_plan_current_relative(self->_update_interval);
}

static void _hio_flood_task_measure(void *param)
{
	hio_flood_detector_t *self = (hio_flood_detector_t *) param;

start:

	switch (self->_state)
	{
		case HIO_FLOOD_DETECTOR_STATE_ERROR:
		{
			self->_measurement_active = false;

			if (self->_event_handler != NULL)
			{
				self->_event_handler(self, HIO_FLOOD_DETECTOR_EVENT_ERROR, self->_event_param);
			}

			self->_state = HIO_FLOOD_DETECTOR_STATE_INITIALIZE;

			return;
		}
		case HIO_FLOOD_DETECTOR_STATE_INITIALIZE:
		{
			self->_state = HIO_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					if (!hio_module_sensor_init())
					{
						goto start;
					}
					hio_gpio_init(HIO_GPIO_P4);
					hio_gpio_set_mode(HIO_GPIO_P4, HIO_GPIO_MODE_INPUT);
					break;
				}
				case HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					if (!hio_module_sensor_init())
					{
						goto start;
					}
					hio_gpio_init(HIO_GPIO_P5);
					hio_gpio_set_mode(HIO_GPIO_P5, HIO_GPIO_MODE_INPUT);
					break;
				}
				default:
				{
					goto start;
				}
			}

			self->_state = HIO_FLOOD_DETECTOR_STATE_READY;

			if (self->_measurement_active)
			{
				hio_scheduler_plan_current_now();
			}
			return;
		}
		case HIO_FLOOD_DETECTOR_STATE_READY:
		{
			self->_state = HIO_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_A, HIO_MODULE_SENSOR_PULL_UP_4K7))
					{
						goto start;
					}
					hio_scheduler_plan_current_from_now(5);
					break;
				}
				case HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_B, HIO_MODULE_SENSOR_PULL_UP_4K7))
					{
						goto start;
					}
					hio_scheduler_plan_current_from_now(5);
					break;
				}
				default:
				{
					goto start;
				}
			}

			self->_state = HIO_FLOOD_DETECTOR_STATE_MEASURE;
			return;
		}
		case HIO_FLOOD_DETECTOR_STATE_MEASURE:
		{
			self->_state = HIO_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					self->_alarm = hio_gpio_get_input(HIO_GPIO_P4) == 1;

					if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_A, HIO_MODULE_SENSOR_PULL_NONE))
					{
						goto start;
					}
					break;
				}
				case HIO_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					self->_alarm = hio_gpio_get_input(HIO_GPIO_P5) == 1;

					if (!hio_module_sensor_set_pull(HIO_MODULE_SENSOR_CHANNEL_B, HIO_MODULE_SENSOR_PULL_NONE))
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
				self->_event_handler(self, HIO_FLOOD_DETECTOR_EVENT_UPDATE, self->_event_param);
			}

			self->_state = HIO_FLOOD_DETECTOR_STATE_READY;

		}
		default:
			break;
	}
}
