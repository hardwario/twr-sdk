#include <hio_analog_sensor.h>

static void _hio_analog_sensor_task_interval(void *param);

static void _hio_analog_sensor_task_measure(void *param);

static void _hio_analog_sensor_adc_event_handler(hio_adc_channel_t channel, hio_adc_event_t event, void *param);

void hio_analog_sensor_init(hio_analog_sensor_t *self, hio_adc_channel_t adc_channel, const hio_analog_sensor_driver_t *driver)
{
    memset(self, 0, sizeof(*self));

    self->_adc_channel = adc_channel;
    self->_driver = driver;

    self->_task_id_interval = hio_scheduler_register(_hio_analog_sensor_task_interval, self, HIO_TICK_INFINITY);
    self->_task_id_measure = hio_scheduler_register(_hio_analog_sensor_task_measure, self, HIO_TICK_INFINITY);

    hio_adc_init();

    hio_adc_set_event_handler(self->_adc_channel, _hio_analog_sensor_adc_event_handler, self);

    if (self->_driver != NULL && self->_driver->init != NULL)
    {
        self->_driver->init(self);
    }
}

void hio_analog_sensor_set_event_handler(hio_analog_sensor_t *self, void (*event_handler)(hio_analog_sensor_t *, hio_analog_sensor_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_analog_sensor_set_update_interval(hio_analog_sensor_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        hio_analog_sensor_measure(self);
    }
}

bool hio_analog_sensor_measure(hio_analog_sensor_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    hio_scheduler_plan_now(self->_task_id_measure);

    return true;
}

bool hio_analog_sensor_get_value(hio_analog_sensor_t *self, uint16_t *result)
{
    if (self->_state != HIO_ANALOG_SENSOR_STATE_UPDATE)
    {
        return false;
    }

    *result = self->_value;

    return true;
}

static void _hio_analog_sensor_task_interval(void *param)
{
    hio_analog_sensor_t *self = param;

    hio_analog_sensor_measure(self);

    hio_scheduler_plan_current_relative(self->_update_interval);
}

static void _hio_analog_sensor_task_measure(void *param)
{
    hio_analog_sensor_t *self = param;

start:

    switch (self->_state)
    {
        case HIO_ANALOG_SENSOR_STATE_ERROR:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_ANALOG_SENSOR_EVENT_ERROR, self->_event_param);
            }

            self->_state = HIO_ANALOG_SENSOR_STATE_ENABLE;

            return;
        }
        case HIO_ANALOG_SENSOR_STATE_ENABLE:
        {
            self->_state = HIO_ANALOG_SENSOR_STATE_MEASURE;

            if (self->_driver != NULL && self->_driver->enable != NULL)
            {
                self->_driver->enable(self);

                if (self->_driver->get_settling_interval != NULL)
                {
                    hio_scheduler_plan_current_from_now(self->_driver->get_settling_interval(self));

                    return;
                }
            }

            goto start;
        }
        case HIO_ANALOG_SENSOR_STATE_MEASURE:
        {
            if (!hio_adc_get_value(self->_adc_channel, &self->_value))
            {
                self->_state = HIO_ANALOG_SENSOR_STATE_ERROR;

                goto start;
            }

            self->_state = HIO_ANALOG_SENSOR_STATE_DISABLE;

            goto start;
        }
        case HIO_ANALOG_SENSOR_STATE_DISABLE:
        {
            if (self->_driver != NULL && self->_driver->disable != NULL)
            {
                self->_driver->disable(self);
            }

            self->_state = HIO_ANALOG_SENSOR_STATE_UPDATE;

            goto start;
        }
        case HIO_ANALOG_SENSOR_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_ANALOG_SENSOR_EVENT_UPDATE, self->_event_param);
            }

            self->_state = HIO_ANALOG_SENSOR_STATE_ENABLE;

            return;
        }
        default:
        {
            self->_state = HIO_ANALOG_SENSOR_STATE_ERROR;

            goto start;
        }
    }
}

static void _hio_analog_sensor_adc_event_handler(hio_adc_channel_t channel, hio_adc_event_t event, void *param)
{
    (void) channel;

    hio_analog_sensor_t *self = param;

    if (event == HIO_ADC_EVENT_DONE)
    {
        self->_state = HIO_ANALOG_SENSOR_STATE_DISABLE;
    }
    else
    {
        self->_state = HIO_ANALOG_SENSOR_STATE_ERROR;
    }

    hio_scheduler_plan_now(self->_task_id_measure);
}
