#include <bc_pulse_counter.h>
#include <bc_system.h>

typedef struct
{
    bc_module_sensor_channel_t channel;
    unsigned int count;
    bc_pulse_counter_edge_t edge;
    bc_tick_t update_interval;
    void (*event_handler)(bc_module_sensor_channel_t, bc_pulse_counter_event_t, void *);
    void *event_param;
    bc_pulse_counter_event_t pending_event;
    bc_scheduler_task_id_t task_id;

} bc_pulse_counter_t;

static bc_pulse_counter_t _bc_module_pulse_counter[2];

static void _bc_pulse_counter_channel_task_update(void *param);
static void _bc_pulse_counter_channel_exti(bc_exti_line_t line, void *param);

void bc_pulse_counter_init(bc_module_sensor_channel_t channel, bc_pulse_counter_edge_t edge)
{
    static const bc_exti_line_t bc_pulse_counter_exti_line[2] =
    {
        BC_EXTI_LINE_PA4,
        BC_EXTI_LINE_PA5
    };

    memset(&_bc_module_pulse_counter[channel], 0, sizeof(_bc_module_pulse_counter[channel]));

    bc_module_sensor_init();
    bc_module_sensor_set_mode(channel, BC_MODULE_SENSOR_MODE_INPUT);
    bc_module_sensor_set_pull(channel, BC_MODULE_SENSOR_PULL_UP_INTERNAL);

    _bc_module_pulse_counter[channel].channel = channel;
    _bc_module_pulse_counter[channel].edge = edge;
    _bc_module_pulse_counter[channel].update_interval = BC_TICK_INFINITY;
    _bc_module_pulse_counter[channel].task_id = bc_scheduler_register(_bc_pulse_counter_channel_task_update, &_bc_module_pulse_counter[channel], BC_TICK_INFINITY);

    bc_exti_register(bc_pulse_counter_exti_line[channel], (bc_exti_edge_t) edge, _bc_pulse_counter_channel_exti, &_bc_module_pulse_counter[channel].channel);
}

void bc_pulse_counter_set_event_handler(bc_module_sensor_channel_t channel, void (*event_handler)(bc_module_sensor_channel_t, bc_pulse_counter_event_t, void *), void *event_param)
{
    _bc_module_pulse_counter[channel].event_handler = event_handler;
    _bc_module_pulse_counter[channel].event_param = event_param;
}

void bc_pulse_counter_set_update_interval(bc_module_sensor_channel_t channel, bc_tick_t interval)
{
    _bc_module_pulse_counter[channel].update_interval = interval;

    if (_bc_module_pulse_counter[channel].update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(_bc_module_pulse_counter[channel].task_id, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(_bc_module_pulse_counter[channel].task_id, _bc_module_pulse_counter[channel].update_interval);
    }
}

void bc_pulse_counter_set(bc_module_sensor_channel_t channel, unsigned int count)
{
    _bc_module_pulse_counter[channel].count = count;
}

unsigned int bc_pulse_counter_get(bc_module_sensor_channel_t channel)
{
    return _bc_module_pulse_counter[channel].count;
}

void bc_pulse_counter_reset(bc_module_sensor_channel_t channel)
{
    _bc_module_pulse_counter[channel].count = 0;
}

static void _bc_pulse_counter_channel_task_update(void *param)
{
    bc_pulse_counter_t *self = param;

    if (self->event_handler != NULL)
    {
        if (self->pending_event == BC_PULSE_COUNTER_EVENT_OVERFLOW)
        {
            self->event_handler(self->channel, BC_PULSE_COUNTER_EVENT_OVERFLOW, self->event_param);
            self->pending_event = BC_PULSE_COUNTER_EVENT_UPDATE;
        }
        else
        {
            self->event_handler(self->channel, BC_PULSE_COUNTER_EVENT_UPDATE, self->event_param);
        }
    }

    bc_scheduler_plan_current_relative(self->update_interval);
}

static void _bc_pulse_counter_channel_exti(bc_exti_line_t line, void *param)
{
    (void) line;
    bc_module_sensor_channel_t channel = *(bc_module_sensor_channel_t *) param;

    _bc_module_pulse_counter[channel].count++;
    if (_bc_module_pulse_counter[channel].count == 0)
    {
        _bc_module_pulse_counter[channel].pending_event = BC_PULSE_COUNTER_EVENT_OVERFLOW;
        bc_scheduler_plan_now(_bc_module_pulse_counter[channel].task_id);
    }
}
