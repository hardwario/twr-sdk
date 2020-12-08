#include <twr_analog_sensor.h>

static void _twr_analog_sensor_task_interval(void *param);

static void _twr_analog_sensor_task_measure(void *param);

static void _twr_analog_sensor_adc_event_handler(twr_adc_channel_t channel, twr_adc_event_t event, void *param);

void twr_analog_sensor_init(twr_analog_sensor_t *self, twr_adc_channel_t adc_channel, const twr_analog_sensor_driver_t *driver)
{
    memset(self, 0, sizeof(*self));

    self->_adc_channel = adc_channel;
    self->_driver = driver;

    self->_task_id_interval = twr_scheduler_register(_twr_analog_sensor_task_interval, self, TWR_TICK_INFINITY);
    self->_task_id_measure = twr_scheduler_register(_twr_analog_sensor_task_measure, self, TWR_TICK_INFINITY);

    twr_adc_init();

    twr_adc_set_event_handler(self->_adc_channel, _twr_analog_sensor_adc_event_handler, self);

    if (self->_driver != NULL && self->_driver->init != NULL)
    {
        self->_driver->init(self);
    }
}

void twr_analog_sensor_set_event_handler(twr_analog_sensor_t *self, void (*event_handler)(twr_analog_sensor_t *, twr_analog_sensor_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_analog_sensor_set_update_interval(twr_analog_sensor_t *self, twr_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(self->_task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        twr_analog_sensor_measure(self);
    }
}

bool twr_analog_sensor_measure(twr_analog_sensor_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    twr_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool twr_analog_sensor_get_value(twr_analog_sensor_t *self, uint16_t *result)
{
    if (self->_state != TWR_ANALOG_SENSOR_STATE_UPDATE)
    {
        return false;
    }

    *result = self->_value;

    return true;
}

static void _twr_analog_sensor_task_interval(void *param)
{
    twr_analog_sensor_t *self = param;

    twr_analog_sensor_measure(self);

    twr_scheduler_plan_current_relative(self->_update_interval);
}

static void _twr_analog_sensor_task_measure(void *param)
{
    twr_analog_sensor_t *self = param;

start:

    switch (self->_state)
    {
        case TWR_ANALOG_SENSOR_STATE_ERROR:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_ANALOG_SENSOR_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_ANALOG_SENSOR_STATE_ENABLE;

            return;
        }
        case TWR_ANALOG_SENSOR_STATE_ENABLE:
        {
            self->_state = TWR_ANALOG_SENSOR_STATE_MEASURE;

            if (self->_driver != NULL && self->_driver->enable != NULL)
            {
                self->_driver->enable(self);

                if (self->_driver->get_settling_interval != NULL)
                {
                    twr_scheduler_plan_current_from_now(self->_driver->get_settling_interval(self));

                    return;
                }
            }

            goto start;
        }
        case TWR_ANALOG_SENSOR_STATE_MEASURE:
        {
            if (!twr_adc_get_value(self->_adc_channel, &self->_value))
            {
                self->_state = TWR_ANALOG_SENSOR_STATE_ERROR;

                goto start;
            }

            self->_state = TWR_ANALOG_SENSOR_STATE_DISABLE;

            goto start;
        }
        case TWR_ANALOG_SENSOR_STATE_DISABLE:
        {
            if (self->_driver != NULL && self->_driver->disable != NULL)
            {
                self->_driver->disable(self);
            }

            self->_state = TWR_ANALOG_SENSOR_STATE_UPDATE;

            goto start;
        }
        case TWR_ANALOG_SENSOR_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_ANALOG_SENSOR_EVENT_UPDATE, self->_event_param);
            }

            self->_state = TWR_ANALOG_SENSOR_STATE_ENABLE;

            return;
        }
        default:
        {
            self->_state = TWR_ANALOG_SENSOR_STATE_ERROR;

            goto start;
        }
    }
}

static void _twr_analog_sensor_adc_event_handler(twr_adc_channel_t channel, twr_adc_event_t event, void *param)
{
    (void) channel;

    twr_analog_sensor_t *self = param;

    if (event == TWR_ADC_EVENT_DONE)
    {
        self->_state = TWR_ANALOG_SENSOR_STATE_DISABLE;
    }
    else
    {
        self->_state = TWR_ANALOG_SENSOR_STATE_ERROR;
    }

    twr_scheduler_plan_now(self->_task_id_measure);
}
