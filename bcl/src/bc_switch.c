#include <bc_switch.h>
#include <bc_timer.h>

#define _BC_SWITCH_SCAN_INTERVAL        50
#define _BC_SWITCH_DEBOUNCE_TIME        20
#define _BC_SWITCH_PULL_ADVANCE_TIME_US 50

static void _bc_switch_task(void *param);

static const bc_gpio_pull_t _bc_switch_pull_lut[5] = {
        [BC_SWITCH_PULL_NONE] = BC_GPIO_PULL_NONE,
        [BC_SWITCH_PULL_UP] = BC_GPIO_PULL_UP,
        [BC_SWITCH_PULL_UP_DYNAMIC] = BC_GPIO_PULL_UP,
        [BC_SWITCH_PULL_DOWN] = BC_GPIO_PULL_DOWN,
        [BC_SWITCH_PULL_DOWN_DYNAMIC] = BC_GPIO_PULL_DOWN
};

void bc_switch_init(bc_switch_t *self, bc_gpio_channel_t channel, bc_switch_type_t type, bc_switch_pull_t pull)
{
    memset(self, 0, sizeof(&self));
    self->_channel = channel;
    self->_type = type;
    self->_pull = pull;
    self->_scan_interval = _BC_SWITCH_SCAN_INTERVAL;
    self->_debounce_time = _BC_SWITCH_DEBOUNCE_TIME;
    self->_pull_advance_time = _BC_SWITCH_PULL_ADVANCE_TIME_US;

    bc_gpio_init(channel);

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

    self->_task_id = bc_scheduler_register(_bc_switch_task, self, 0);

    if ((self->_pull == BC_SWITCH_PULL_UP_DYNAMIC) || (self->_pull == BC_SWITCH_PULL_DOWN_DYNAMIC))
    {
        if (self->_pull_advance_time > 1000)
        {
            self->_task_state = BC_SWITCH_TASK_STATE_SET_PULL;
        }

        bc_gpio_set_pull(self->_channel, BC_GPIO_PULL_NONE);

        bc_timer_init();
    }
    else
    {
        bc_gpio_set_pull(self->_channel, _bc_switch_pull_lut[self->_pull]);
    }
}

bool bc_switch_get_state(bc_switch_t *self)
{
    return self->_pin_state != 0 ? BC_SWITCH_OPEN : BC_SWITCH_CLOSE;
}

void bc_switch_set_event_handler(bc_switch_t *self, void (*event_handler)(bc_switch_t *, bc_switch_event_t, void*), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_switch_set_scan_interval(bc_switch_t *self, bc_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;
}

void bc_switch_set_debounce_time(bc_switch_t *self, bc_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

void bc_switch_set_pull_advance_time(bc_switch_t *self, uint16_t pull_advance_time_us)
{
    self->_pull_advance_time = pull_advance_time_us;
}

static void _bc_switch_task(void *param)
{
    bc_switch_t *self = (bc_switch_t *) param;

    switch (self->_task_state)
    {
        case BC_SWITCH_TASK_STATE_MEASURE:
        {
            bool dynamic = (self->_pull == BC_SWITCH_PULL_UP_DYNAMIC) || (self->_pull == BC_SWITCH_PULL_DOWN_DYNAMIC);

            if (dynamic)
            {
                if (self->_pull_advance_time < 1000)
                {
                    bc_gpio_set_pull(self->_channel, _bc_switch_pull_lut[self->_pull]);

                    bc_timer_start();

                    bc_timer_delay(self->_pull_advance_time);

                    bc_timer_stop();
                }
            }

            int pin_state = bc_gpio_get_input(self->_channel);

            if (dynamic)
            {
                bc_gpio_set_pull(self->_channel, BC_GPIO_PULL_NONE);

                if (self->_pull_advance_time > 1000)
                {
                    self->_task_state = BC_SWITCH_TASK_STATE_SET_PULL;
                }
            }

            if (self->_type == BC_SWITCH_TYPE_NC)
            {
                pin_state = pin_state == 0 ? 1 : 0;
            }

            bc_tick_t tick_now = bc_scheduler_get_spin_tick();

            if (self->_pin_state != pin_state)
            {
                if (self->_tick_debounce == BC_TICK_INFINITY)
                {
                    self->_tick_debounce = tick_now + self->_debounce_time;

                    bc_scheduler_plan_current_relative(self->_debounce_time);

                    return;
                }

                if (tick_now >= self->_tick_debounce)
                {
                    self->_pin_state = pin_state;

                    if (self->_event_handler != NULL)
                    {
                        if (self->_pin_state != 0)
                        {
                            self->_event_handler(self, BC_SWITCH_EVENT_CLOSED, self->_event_param);
                        }
                        else
                        {
                            self->_event_handler(self, BC_SWITCH_EVENT_OPENED, self->_event_param);
                        }
                    }
                }
            }
            else
            {
                self->_tick_debounce = BC_TICK_INFINITY;
            }

            bc_scheduler_plan_current_relative(self->_scan_interval);

            return;

        }
        case BC_SWITCH_TASK_STATE_SET_PULL:
        {
            bc_gpio_set_pull(self->_channel, _bc_switch_pull_lut[self->_pull]);

            bc_scheduler_plan_current_relative(self->_pull_advance_time / 1000);

            self->_task_state = BC_SWITCH_TASK_STATE_MEASURE;

            return;
        }
        default:
        {
            return;
        }
    }
}
