#include <twr_pulse_counter.h>
#include <twr_system.h>

typedef struct
{
    twr_module_sensor_channel_t channel;
    unsigned int count;
    twr_pulse_counter_edge_t edge;
    twr_tick_t update_interval;
    void (*event_handler)(twr_module_sensor_channel_t, twr_pulse_counter_event_t, void *);
    void *event_param;
    bool pending_event_flag;
    twr_pulse_counter_event_t pending_event;
    twr_scheduler_task_id_t task_id;

} twr_pulse_counter_t;

static twr_pulse_counter_t _twr_module_pulse_counter[3];

static void _twr_pulse_counter_channel_task_update(void *param);
static void _twr_pulse_counter_channel_exti(twr_exti_line_t line, void *param);

void twr_pulse_counter_init(twr_module_sensor_channel_t channel, twr_pulse_counter_edge_t edge)
{
    static const twr_exti_line_t twr_pulse_counter_exti_line[3] =
    {
        TWR_EXTI_LINE_PA4,
        TWR_EXTI_LINE_PA5,
        TWR_EXTI_LINE_PA6
    };

    memset(&_twr_module_pulse_counter[channel], 0, sizeof(_twr_module_pulse_counter[channel]));

    twr_module_sensor_init();
    twr_module_sensor_set_mode(channel, TWR_MODULE_SENSOR_MODE_INPUT);
    twr_module_sensor_set_pull(channel, TWR_MODULE_SENSOR_PULL_UP_INTERNAL);

    _twr_module_pulse_counter[channel].channel = channel;
    _twr_module_pulse_counter[channel].edge = edge;
    _twr_module_pulse_counter[channel].update_interval = TWR_TICK_INFINITY;
    _twr_module_pulse_counter[channel].task_id = twr_scheduler_register(_twr_pulse_counter_channel_task_update, &_twr_module_pulse_counter[channel], TWR_TICK_INFINITY);

    twr_exti_register(twr_pulse_counter_exti_line[channel], (twr_exti_edge_t) edge, _twr_pulse_counter_channel_exti, &_twr_module_pulse_counter[channel].channel);
}

void twr_pulse_counter_set_event_handler(twr_module_sensor_channel_t channel, void (*event_handler)(twr_module_sensor_channel_t, twr_pulse_counter_event_t, void *), void *event_param)
{
    _twr_module_pulse_counter[channel].event_handler = event_handler;
    _twr_module_pulse_counter[channel].event_param = event_param;
}

void twr_pulse_counter_set_update_interval(twr_module_sensor_channel_t channel, twr_tick_t interval)
{
    _twr_module_pulse_counter[channel].update_interval = interval;

    if (_twr_module_pulse_counter[channel].update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(_twr_module_pulse_counter[channel].task_id, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(_twr_module_pulse_counter[channel].task_id, _twr_module_pulse_counter[channel].update_interval);
    }
}

void twr_pulse_counter_set(twr_module_sensor_channel_t channel, unsigned int count)
{
    _twr_module_pulse_counter[channel].count = count;
}

unsigned int twr_pulse_counter_get(twr_module_sensor_channel_t channel)
{
    return _twr_module_pulse_counter[channel].count;
}

void twr_pulse_counter_reset(twr_module_sensor_channel_t channel)
{
    _twr_module_pulse_counter[channel].count = 0;
}

static void _twr_pulse_counter_channel_task_update(void *param)
{
    twr_pulse_counter_t *self = param;

    if (self->pending_event_flag)
    {
        self->pending_event_flag = false;

        if (self->event_handler != NULL)
        {
            if (self->pending_event == TWR_PULSE_COUNTER_EVENT_OVERFLOW)
            {
                self->event_handler(self->channel, TWR_PULSE_COUNTER_EVENT_OVERFLOW, self->event_param);

                self->pending_event_flag = true;
                self->pending_event = TWR_PULSE_COUNTER_EVENT_UPDATE;
            }
            else if (self->pending_event == TWR_PULSE_COUNTER_EVENT_UPDATE)
            {
                self->event_handler(self->channel, TWR_PULSE_COUNTER_EVENT_UPDATE, self->event_param);
            }
        }
    }

    twr_scheduler_plan_current_relative(self->update_interval);
}

static void _twr_pulse_counter_channel_exti(twr_exti_line_t line, void *param)
{
    (void) line;
    twr_module_sensor_channel_t channel = *(twr_module_sensor_channel_t *) param;

    _twr_module_pulse_counter[channel].count++;

    _twr_module_pulse_counter[channel].pending_event_flag = true;
    _twr_module_pulse_counter[channel].pending_event = TWR_PULSE_COUNTER_EVENT_UPDATE;

    if (_twr_module_pulse_counter[channel].count == 0)
    {
        _twr_module_pulse_counter[channel].pending_event = TWR_PULSE_COUNTER_EVENT_OVERFLOW;
    }

    twr_scheduler_plan_now(_twr_module_pulse_counter[channel].task_id);
}
