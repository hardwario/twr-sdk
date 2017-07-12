#include <stm32l0xx.h>
#include <bc_pulse_counter.h>
#include <bc_module_core.h>

#define _BC_PULSE_COUNTER_DEBOUNCE_TIME_DEFAULT_US 20

typedef struct
{
	bc_module_sensor_channel_t _channel;
	int _count;
	bc_pulse_counter_edge_t _edge;
	int _debounce_time;
	bc_tick_t _update_interval;
	void (*_event_handler)(bc_module_sensor_channel_t, bc_pulse_counter_event_t, void *);
	void *_event_param;
	int _idle;
	bc_scheduler_task_id_t _task_id;

} bc_pulse_counter_t;

typedef void (*bc_pulse_counter_exti_callback_t)(bc_exti_line_t, void *);
typedef void (*bc_pulse_counter_task)(void *);

static bc_pulse_counter_t bc_module_pulse_counter[2];

static bc_exti_line_t bc_pulse_counter_exti_line[2] =
{
	BC_EXTI_LINE_PA4,
	BC_EXTI_LINE_PA5
};

static void _bc_pulse_counter_channel_a_task_update(void *param);
static void _bc_pulse_counter_channel_b_task_update(void *param);
static void _bc_pulse_counter_channel_a_exti(bc_exti_line_t line, void *param);
static void _bc_pulse_counter_channel_b_exti(bc_exti_line_t line, void *param);

void bc_pulse_counter_init(bc_module_sensor_channel_t channel, bc_pulse_counter_edge_t edge)
{
	const bc_pulse_counter_exti_callback_t bc_pulse_counter_irq[2] =
	{ [0] = _bc_pulse_counter_channel_a_exti, [1] = _bc_pulse_counter_channel_b_exti };

	const bc_pulse_counter_task bc_pulse_counter_update_task[2] =
	{ [0] = _bc_pulse_counter_channel_a_task_update, [1] = _bc_pulse_counter_channel_b_task_update };

	memset(&bc_module_pulse_counter[channel], 0, sizeof(bc_module_pulse_counter[channel]));

	bc_module_sensor_init();
	bc_module_sensor_set_mode(channel, BC_MODULE_SENSOR_MODE_INPUT);
	bc_module_sensor_set_pull(channel, BC_MODULE_SENSOR_PULL_UP_INTERNAL);

	bc_module_pulse_counter[channel]._edge = edge;
	bc_module_pulse_counter[channel]._debounce_time = _BC_PULSE_COUNTER_DEBOUNCE_TIME_DEFAULT_US;
	bc_module_pulse_counter[channel]._update_interval = BC_TICK_INFINITY;
	bc_module_pulse_counter[channel]._idle = bc_module_sensor_get_input(channel);
	bc_module_pulse_counter[channel]._task_id = bc_scheduler_register(bc_pulse_counter_update_task[channel], NULL, BC_TICK_INFINITY);

	bc_exti_register(bc_pulse_counter_exti_line[channel], (bc_exti_edge_t) edge, (void (*)(bc_exti_line_t, void *)) (bc_pulse_counter_irq[channel]), NULL);
}

void bc_pulse_counter_set_event_handler(bc_module_sensor_channel_t channel, void (*event_handler)(bc_module_sensor_channel_t channel, bc_pulse_counter_event_t, void *), void *event_param)
{
	bc_module_pulse_counter[channel]._event_handler = event_handler;
	bc_module_pulse_counter[channel]._event_param = event_param;
}

void bc_pulse_counter_set_update_interval(bc_module_sensor_channel_t channel, bc_tick_t interval)
{
	bc_module_pulse_counter[channel]._update_interval = interval;

	if (bc_module_pulse_counter[channel]._update_interval == BC_TICK_INFINITY)
	{
		bc_scheduler_plan_absolute(bc_module_pulse_counter[channel]._task_id, BC_TICK_INFINITY);
	}
	else
	{
		bc_scheduler_plan_relative(bc_module_pulse_counter[channel]._task_id, bc_module_pulse_counter[channel]._update_interval);
	}
}

void bc_pulse_counter_set_debounce_time(bc_module_sensor_channel_t channel, int debounce_time)
{
	bc_module_pulse_counter[channel]._debounce_time = debounce_time;
}

void bc_pulse_counter_set(bc_module_sensor_channel_t channel, int count)
{
	bc_module_pulse_counter[channel]._count = count;
}

unsigned int bc_pulse_counter_get(bc_module_sensor_channel_t channel)
{
	return bc_module_pulse_counter[channel]._count;
}

void bc_pulse_counter_reset(bc_module_sensor_channel_t channel)
{
	bc_module_pulse_counter[channel]._count = 0;
}

static void _bc_pulse_counter_channel_a_task_update(void *param)
{
	(void) param;

	if (bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_A_P4]._event_handler != NULL)
	{
		bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_A_P4]._event_handler(BC_MODULE_SENSOR_CHANNEL_A_P4, BC_COUNTER_EVENT_UPDATE, bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_A_P4]._event_param);
	}

	bc_scheduler_plan_current_relative(bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_A_P4]._update_interval);
}

static void _bc_pulse_counter_channel_b_task_update(void *param)
{
	(void) param;

	if (bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_B_P5]._event_handler != NULL)
	{
		bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_B_P5]._event_handler(BC_MODULE_SENSOR_CHANNEL_B_P5, BC_COUNTER_EVENT_UPDATE, bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_B_P5]._event_param);
	}

	bc_scheduler_plan_current_relative(bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_B_P5]._update_interval);
}

static void _bc_pulse_counter_channel_a_exti(bc_exti_line_t line, void *param)
{
	(void) line;
	(void) param;

	bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_A_P4]._count++;
}

static void _bc_pulse_counter_channel_b_exti(bc_exti_line_t line, void *param)
{
	(void) line;
	(void) param;

	bc_module_pulse_counter[BC_MODULE_SENSOR_CHANNEL_B_P5]._count++;
}
