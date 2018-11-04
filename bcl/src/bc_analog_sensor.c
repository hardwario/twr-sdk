#include <bc_analog_sensor.h>

static void _bc_analog_sensor_task_interval(void *param);

static void _bc_analog_sensor_task_measure(void *param);

static void _bc_analog_sensor_adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *param);

void bc_analog_sensor_init(bc_analog_sensor_t *self, bc_adc_channel_t adc_channel, const bc_analog_sensor_driver_t *driver)
{
    memset(self, 0, sizeof(*self));

    self->_adc_channel = adc_channel;
    self->_driver = driver;

    self->_task_id_interval = bc_scheduler_register(_bc_analog_sensor_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_analog_sensor_task_measure, self, BC_TICK_INFINITY);

    bc_adc_init();

    bc_adc_set_event_handler(self->_adc_channel, _bc_analog_sensor_adc_event_handler, self);

    if (self->_driver != NULL && self->_driver->init != NULL)
    {
        self->_driver->init(self);
    }
}

void bc_analog_sensor_set_event_handler(bc_analog_sensor_t *self, void (*event_handler)(bc_analog_sensor_t *, bc_analog_sensor_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_analog_sensor_set_update_interval(bc_analog_sensor_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_analog_sensor_measure(self);
    }
}

bool bc_analog_sensor_measure(bc_analog_sensor_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool bc_analog_sensor_get_value(bc_analog_sensor_t *self, uint16_t *result)
{
    if (self->_state != BC_ANALOG_SENSOR_STATE_UPDATE)
    {
        return false;
    }

    *result = self->_value;

    return true;
}

static void _bc_analog_sensor_task_interval(void *param)
{
    bc_analog_sensor_t *self = param;

    bc_analog_sensor_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_analog_sensor_task_measure(void *param)
{
    bc_analog_sensor_t *self = param;

start:

    switch (self->_state)
    {
        case BC_ANALOG_SENSOR_STATE_ERROR:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_ANALOG_SENSOR_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_ANALOG_SENSOR_STATE_ENABLE;

            return;
        }
        case BC_ANALOG_SENSOR_STATE_ENABLE:
        {
            self->_state = BC_ANALOG_SENSOR_STATE_MEASURE;

            if (self->_driver != NULL && self->_driver->enable != NULL)
            {
                self->_driver->enable(self);

                if (self->_driver->get_settling_interval != NULL)
                {
                    bc_scheduler_plan_current_from_now(self->_driver->get_settling_interval(self));

                    return;
                }
            }

            goto start;
        }
        case BC_ANALOG_SENSOR_STATE_MEASURE:
        {
            if (!bc_adc_get_value(self->_adc_channel, &self->_value))
            {
                self->_state = BC_ANALOG_SENSOR_STATE_ERROR;

                goto start;
            }

            self->_state = BC_ANALOG_SENSOR_STATE_DISABLE;

            goto start;
        }
        case BC_ANALOG_SENSOR_STATE_DISABLE:
        {
            if (self->_driver != NULL && self->_driver->disable != NULL)
            {
                self->_driver->disable(self);
            }

            self->_state = BC_ANALOG_SENSOR_STATE_UPDATE;

            goto start;
        }
        case BC_ANALOG_SENSOR_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_ANALOG_SENSOR_EVENT_UPDATE, self->_event_param);
            }

            self->_state = BC_ANALOG_SENSOR_STATE_ENABLE;

            return;
        }
        default:
        {
            self->_state = BC_ANALOG_SENSOR_STATE_ERROR;

            goto start;
        }
    }
}

static void _bc_analog_sensor_adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *param)
{
    (void) channel;

    bc_analog_sensor_t *self = param;

    if (event == BC_ADC_EVENT_DONE)
    {
        self->_state = BC_ANALOG_SENSOR_STATE_DISABLE;
    }
    else
    {
        self->_state = BC_ANALOG_SENSOR_STATE_ERROR;
    }

    bc_scheduler_plan_now(self->_task_id_measure);
}
