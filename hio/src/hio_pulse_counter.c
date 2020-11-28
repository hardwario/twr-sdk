#include <hio_pulse_counter.h>
#include <hio_system.h>

typedef struct
{
    hio_module_sensor_channel_t channel;
    unsigned int count;
    hio_pulse_counter_edge_t edge;
    hio_tick_t update_interval;
    void (*event_handler)(hio_module_sensor_channel_t, hio_pulse_counter_event_t, void *);
    void *event_param;
    bool pending_event_flag;
    hio_pulse_counter_event_t pending_event;
    hio_scheduler_task_id_t task_id;

} hio_pulse_counter_t;

static hio_pulse_counter_t _hio_module_pulse_counter[3];

static void _hio_pulse_counter_channel_task_update(void *param);
static void _hio_pulse_counter_channel_exti(hio_exti_line_t line, void *param);

void hio_pulse_counter_init(hio_module_sensor_channel_t channel, hio_pulse_counter_edge_t edge)
{
    static const hio_exti_line_t hio_pulse_counter_exti_line[3] =
    {
        HIO_EXTI_LINE_PA4,
        HIO_EXTI_LINE_PA5,
        HIO_EXTI_LINE_PA6
    };

    memset(&_hio_module_pulse_counter[channel], 0, sizeof(_hio_module_pulse_counter[channel]));

    hio_module_sensor_init();
    hio_module_sensor_set_mode(channel, HIO_MODULE_SENSOR_MODE_INPUT);
    hio_module_sensor_set_pull(channel, HIO_MODULE_SENSOR_PULL_UP_INTERNAL);

    _hio_module_pulse_counter[channel].channel = channel;
    _hio_module_pulse_counter[channel].edge = edge;
    _hio_module_pulse_counter[channel].update_interval = HIO_TICK_INFINITY;
    _hio_module_pulse_counter[channel].task_id = hio_scheduler_register(_hio_pulse_counter_channel_task_update, &_hio_module_pulse_counter[channel], HIO_TICK_INFINITY);

    hio_exti_register(hio_pulse_counter_exti_line[channel], (hio_exti_edge_t) edge, _hio_pulse_counter_channel_exti, &_hio_module_pulse_counter[channel].channel);
}

void hio_pulse_counter_set_event_handler(hio_module_sensor_channel_t channel, void (*event_handler)(hio_module_sensor_channel_t, hio_pulse_counter_event_t, void *), void *event_param)
{
    _hio_module_pulse_counter[channel].event_handler = event_handler;
    _hio_module_pulse_counter[channel].event_param = event_param;
}

void hio_pulse_counter_set_update_interval(hio_module_sensor_channel_t channel, hio_tick_t interval)
{
    _hio_module_pulse_counter[channel].update_interval = interval;

    if (_hio_module_pulse_counter[channel].update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(_hio_module_pulse_counter[channel].task_id, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(_hio_module_pulse_counter[channel].task_id, _hio_module_pulse_counter[channel].update_interval);
    }
}

void hio_pulse_counter_set(hio_module_sensor_channel_t channel, unsigned int count)
{
    _hio_module_pulse_counter[channel].count = count;
}

unsigned int hio_pulse_counter_get(hio_module_sensor_channel_t channel)
{
    return _hio_module_pulse_counter[channel].count;
}

void hio_pulse_counter_reset(hio_module_sensor_channel_t channel)
{
    _hio_module_pulse_counter[channel].count = 0;
}

static void _hio_pulse_counter_channel_task_update(void *param)
{
    hio_pulse_counter_t *self = param;

    if (self->pending_event_flag)
    {
        self->pending_event_flag = false;

        if (self->event_handler != NULL)
        {
            if (self->pending_event == HIO_PULSE_COUNTER_EVENT_OVERFLOW)
            {
                self->event_handler(self->channel, HIO_PULSE_COUNTER_EVENT_OVERFLOW, self->event_param);

                self->pending_event_flag = true;
                self->pending_event = HIO_PULSE_COUNTER_EVENT_UPDATE;
            }
            else if (self->pending_event == HIO_PULSE_COUNTER_EVENT_UPDATE)
            {
                self->event_handler(self->channel, HIO_PULSE_COUNTER_EVENT_UPDATE, self->event_param);
            }
        }
    }

    hio_scheduler_plan_current_relative(self->update_interval);
}

static void _hio_pulse_counter_channel_exti(hio_exti_line_t line, void *param)
{
    (void) line;
    hio_module_sensor_channel_t channel = *(hio_module_sensor_channel_t *) param;

    _hio_module_pulse_counter[channel].count++;

    _hio_module_pulse_counter[channel].pending_event_flag = true;
    _hio_module_pulse_counter[channel].pending_event = HIO_PULSE_COUNTER_EVENT_UPDATE;

    if (_hio_module_pulse_counter[channel].count == 0)
    {
        _hio_module_pulse_counter[channel].pending_event = HIO_PULSE_COUNTER_EVENT_OVERFLOW;
    }

    hio_scheduler_plan_now(_hio_module_pulse_counter[channel].task_id);
}
