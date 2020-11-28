#include <bc_flood_detector.h>

#define _BC_FLOOD_DETECTOR_DELAY_RUN 50

static void _bc_flood_task_interval(void *param);

static void _bc_flood_task_measure(void *param);

void bc_flood_detector_init(bc_flood_detector_t *self, bc_flood_detector_type_t type)
{
	memset(self, 0, sizeof(&self));
	self->_type = type;
	self->_task_id_interval = bc_scheduler_register(_bc_flood_task_interval, self, BC_TICK_INFINITY);
	self->_task_id_measure = bc_scheduler_register(_bc_flood_task_measure, self, _BC_FLOOD_DETECTOR_DELAY_RUN);
}

void bc_flood_detector_set_event_handler(bc_flood_detector_t *self, void (*event_handler)(bc_flood_detector_t *, bc_flood_detector_event_t, void*), void *event_param)
{
	self->_event_handler = event_handler;
	self->_event_param = event_param;
}

void bc_flood_detector_set_update_interval(bc_flood_detector_t *self, bc_tick_t interval)
{
	self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);
    }
}

bool bc_flood_detector_measure(bc_flood_detector_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool bc_flood_detector_is_alarm(bc_flood_detector_t *self)
{
	return self->_alarm;
}

static void _bc_flood_task_interval(void *param)
{
	bc_flood_detector_t *self = (bc_flood_detector_t *) param;

	bc_flood_detector_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_flood_task_measure(void *param)
{
	bc_flood_detector_t *self = (bc_flood_detector_t *) param;

start:

	switch (self->_state)
	{
		case BC_FLOOD_DETECTOR_STATE_ERROR:
		{
			self->_measurement_active = false;

			if (self->_event_handler != NULL)
			{
				self->_event_handler(self, BC_FLOOD_DETECTOR_EVENT_ERROR, self->_event_param);
			}

			self->_state = BC_FLOOD_DETECTOR_STATE_INITIALIZE;

			return;
		}
		case BC_FLOOD_DETECTOR_STATE_INITIALIZE:
		{
			self->_state = BC_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					if (!bc_module_sensor_init())
					{
						goto start;
					}
					bc_gpio_init(BC_GPIO_P4);
					bc_gpio_set_mode(BC_GPIO_P4, BC_GPIO_MODE_INPUT);
					break;
				}
				case BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					if (!bc_module_sensor_init())
					{
						goto start;
					}
					bc_gpio_init(BC_GPIO_P5);
					bc_gpio_set_mode(BC_GPIO_P5, BC_GPIO_MODE_INPUT);
					break;
				}
				default:
				{
					goto start;
				}
			}

			self->_state = BC_FLOOD_DETECTOR_STATE_READY;

			if (self->_measurement_active)
			{
				bc_scheduler_plan_current_now();
			}
			return;
		}
		case BC_FLOOD_DETECTOR_STATE_READY:
		{
			self->_state = BC_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_A, BC_MODULE_SENSOR_PULL_UP_4K7))
					{
						goto start;
					}
					bc_scheduler_plan_current_from_now(5);
					break;
				}
				case BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_B, BC_MODULE_SENSOR_PULL_UP_4K7))
					{
						goto start;
					}
					bc_scheduler_plan_current_from_now(5);
					break;
				}
				default:
				{
					goto start;
				}
			}

			self->_state = BC_FLOOD_DETECTOR_STATE_MEASURE;
			return;
		}
		case BC_FLOOD_DETECTOR_STATE_MEASURE:
		{
			self->_state = BC_FLOOD_DETECTOR_STATE_ERROR;

			switch (self->_type)
			{
				case BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_A:
				{
					self->_alarm = bc_gpio_get_input(BC_GPIO_P4) == 1;

					if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_A, BC_MODULE_SENSOR_PULL_NONE))
					{
						goto start;
					}
					break;
				}
				case BC_FLOOD_DETECTOR_TYPE_LD_81_SENSOR_MODULE_CHANNEL_B:
				{
					self->_alarm = bc_gpio_get_input(BC_GPIO_P5) == 1;

					if (!bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_B, BC_MODULE_SENSOR_PULL_NONE))
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
				self->_event_handler(self, BC_FLOOD_DETECTOR_EVENT_UPDATE, self->_event_param);
			}

			self->_state = BC_FLOOD_DETECTOR_STATE_READY;

		}
		default:
			break;
	}
}
