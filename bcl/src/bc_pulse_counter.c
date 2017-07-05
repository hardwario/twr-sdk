#include <bc_pulse_counter.h>

#define _BC_PULSE_COUNTER_SCAN_INTERVAL 20
#define _BC_PULSE_COUNTER_DEBOUNCE_TIME 50
#define _BC_PULSE_COUNTER_UPDATE_INTERVAL 1000

static void _bc_pulse_counter_task_update(void *param);

static void _bc_pulse_counter_task(void *param);

static void _bc_pulse_counter_gpio_init(bc_pulse_counter_t *self);

static int _bc_pulse_counter_gpio_get_input(bc_pulse_counter_t *self);

static const bc_pulse_counter_driver_t _bc_counter_driver_gpio =
{
    .init = _bc_pulse_counter_gpio_init,
    .get_input = _bc_pulse_counter_gpio_get_input
};

void bc_pulse_counter_init(bc_pulse_counter_t *self, bc_gpio_channel_t gpio_channel, bc_pulse_counter_edge_t edge)
{
    memset(self, 0, sizeof(*self));

    self->_channel.gpio = gpio_channel;
    self->_edge = edge;
    self->_debounce_time = _BC_PULSE_COUNTER_DEBOUNCE_TIME;
    self->_state = BC_COUNTER_STATE_IDLE;

    self->_driver = &_bc_counter_driver_gpio;
    self->_driver->init(self);
    self->_idle = self->_driver->get_input(self);

    self->_task_id_interval = bc_scheduler_register(_bc_pulse_counter_task_update, self, BC_TICK_INFINITY);
    bc_scheduler_register(_bc_pulse_counter_task, self, _BC_PULSE_COUNTER_SCAN_INTERVAL);
}

void bc_pulse_counter_init_virtual(bc_pulse_counter_t *self, int channel, const bc_pulse_counter_driver_t *driver, bc_pulse_counter_edge_t edge)
{
    memset(self, 0, sizeof(*self));

    self->_channel.virtual = channel;
    self->_edge = edge;
    self->_debounce_time = _BC_PULSE_COUNTER_DEBOUNCE_TIME;
    self->_state = BC_COUNTER_STATE_IDLE;

    self->_driver = driver;
    self->_driver->init(self);
    self->_idle = self->_driver->get_input(self);

    self->_task_id_interval = bc_scheduler_register(_bc_pulse_counter_task_update, self, BC_TICK_INFINITY);
    bc_scheduler_register(_bc_pulse_counter_task, self, _BC_PULSE_COUNTER_SCAN_INTERVAL);
}

void bc_pulse_counter_set_event_handler(bc_pulse_counter_t *self, void (*event_handler)(bc_pulse_counter_t *, bc_pulse_counter_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_pulse_counter_set_update_interval(bc_pulse_counter_t *self, bc_tick_t interval)
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

void bc_pulse_counter_set_debounce_time(bc_pulse_counter_t *self, bc_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

void bc_pulse_counter_set(bc_pulse_counter_t *self, int count)
{
    self->_count = count;
}

int bc_pulse_counter_get(bc_pulse_counter_t *self)
{
    return self->_count;
}

void bc_pulse_counter_reset(bc_pulse_counter_t *self)
{
    self->_count = 0;
}

static void _bc_pulse_counter_task_update(void *param)
{
    bc_pulse_counter_t *self = param;

    self->_event_handler(self, BC_COUNTER_EVENT_UPDATE, self->_event_param);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_pulse_counter_task(void *param)
{
    bc_pulse_counter_t *self = param;

    // Get state of present counter input
    int value = self->_driver->get_input(self);

    switch (self->_state)
    {
        case BC_COUNTER_STATE_IDLE:
        {
            // If the state of input has changed...
            if (value != self->_idle)
            {
                self->_changed = value;
                self->_state = BC_COUNTER_STATE_DEBOUNCE;
                bc_scheduler_plan_current_relative(self->_debounce_time);
                return;
            }

            break;
        }
        case BC_COUNTER_STATE_DEBOUNCE:
        {
            // If state is valid...
            if (self->_changed == value)
            {
                // ...increment on watched edge
                if (self->_changed != 0)
                {
                    if ((self->_edge == BC_COUNTER_EDGE_RISE) || (self->_edge == BC_COUNTER_EDGE_RISE_FALL))
                    {
                        self->_count++;
                    }
                }
                else
                {
                    if ((self->_edge == BC_COUNTER_EDGE_RISE_FALL) || (self->_edge == BC_COUNTER_EDGE_FALL))
                    {
                        self->_count++;
                    }
                }

                // Update idle state
                self->_idle = value;
            }

            // Update internal stae to idle
            self->_state = BC_COUNTER_STATE_IDLE;

            break;
        }
        default:
        {
            for(;;);
            break;
        }
    }

    bc_scheduler_plan_current_relative(_BC_PULSE_COUNTER_SCAN_INTERVAL);

    return;
}

static void _bc_pulse_counter_gpio_init(bc_pulse_counter_t *self)
{
    bc_gpio_init(self->_channel.gpio);
    bc_gpio_set_mode(self->_channel.gpio, BC_GPIO_MODE_INPUT);
}

static int _bc_pulse_counter_gpio_get_input(bc_pulse_counter_t *self)
{
    return bc_gpio_get_input(self->_channel.gpio);
}

