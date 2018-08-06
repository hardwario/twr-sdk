#include <bc_magnetic_switch.h>

#define _BC_MAGNETIC_SWITCH_SCAN_INTERVAL 50
#define _BC_MAGNETIC_SWITCH_DEBOUNCE_TIME 20

static void _bc_magnetic_switch_task(void *param);

void bc_magnetic_switch_init(bc_magnetic_switch_t *self, bc_gpio_channel_t channel, bc_gpio_pull_t gpio_pull)
{
    memset(self, 0, sizeof(&self));
    self->_channel = channel;
    self->_gpio_pull = gpio_pull;
    self->_scan_interval = _BC_MAGNETIC_SWITCH_SCAN_INTERVAL;
    self->_debounce_time = _BC_MAGNETIC_SWITCH_DEBOUNCE_TIME;

    bc_gpio_init(channel);

    bc_gpio_set_pull(self->_channel, self->_gpio_pull);

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

    self->_pin_state = bc_gpio_get_input(self->_channel);

    bc_gpio_set_pull(channel, BC_GPIO_PULL_NONE);

    self->_task_id = bc_scheduler_register(_bc_magnetic_switch_task, self, 0);
}

bool bc_magnetic_switch_is_open(bc_magnetic_switch_t *self)
{
    return self->_pin_state == (self->_gpio_pull == BC_GPIO_PULL_UP ? 1 : 0);
}

void bc_magnetic_switch_set_event_handler(bc_magnetic_switch_t *self, void (*event_handler)(bc_magnetic_switch_t *, bc_magnetic_switch_event_t, void*), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_magnetic_switch_set_scan_interval(bc_magnetic_switch_t *self, bc_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;
}

void bc_magnetic_switch_set_debounce_time(bc_magnetic_switch_t *self, bc_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

static void _bc_magnetic_switch_task(void *param)
{
    bc_magnetic_switch_t *self = (bc_magnetic_switch_t *) param;

    switch (self->_state)
    {
        case BC_MAGNETIC_SWITCH_STATE_READY:
        {
            bc_gpio_set_pull(self->_channel, self->_gpio_pull);

            bc_scheduler_plan_current_relative(10);

            self->_state = BC_MAGNETIC_SWITCH_STATE_MEASURE;

            return;
        }
        case BC_MAGNETIC_SWITCH_STATE_MEASURE:
        {
            int pin_state = bc_gpio_get_input(self->_channel);

            bc_gpio_set_pull(self->_channel, BC_GPIO_PULL_NONE);

            bc_tick_t tick_now = bc_scheduler_get_spin_tick();

            self->_state = BC_MAGNETIC_SWITCH_STATE_READY;

            if (self->_pin_state != pin_state)
            {
                if (self->_tick_debounce == BC_TICK_INFINITY)
                {
                    self->_tick_debounce = tick_now + self->_debounce_time;

                    bc_scheduler_plan_current_relative(self->_debounce_time - 10);

                    return;
                }

                if (tick_now >= self->_tick_debounce)
                {
                    self->_pin_state = pin_state;

                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_MAGNETIC_SWITCH_EVENT_STATE_CHANGE, self->_event_param);
                    }
                }
            }
            else
            {
                self->_tick_debounce = BC_TICK_INFINITY;
            }

            bc_scheduler_plan_current_relative(self->_scan_interval - 10);

            return;

        }
        default:
        {
            return;
        }
    }
}
