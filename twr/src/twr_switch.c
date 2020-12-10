#include <twr_switch.h>
#include <twr_timer.h>

#define _TWR_SWITCH_SCAN_INTERVAL        50
#define _TWR_SWITCH_DEBOUNCE_TIME        20
#define _TWR_SWITCH_PULL_ADVANCE_TIME_US 50

static void _twr_switch_task(void *param);

static const twr_gpio_pull_t _twr_switch_pull_lut[5] = {
        [TWR_SWITCH_PULL_NONE] = TWR_GPIO_PULL_NONE,
        [TWR_SWITCH_PULL_UP] = TWR_GPIO_PULL_UP,
        [TWR_SWITCH_PULL_UP_DYNAMIC] = TWR_GPIO_PULL_UP,
        [TWR_SWITCH_PULL_DOWN] = TWR_GPIO_PULL_DOWN,
        [TWR_SWITCH_PULL_DOWN_DYNAMIC] = TWR_GPIO_PULL_DOWN
};

void twr_switch_init(twr_switch_t *self, twr_gpio_channel_t channel, twr_switch_type_t type, twr_switch_pull_t pull)
{
    memset(self, 0, sizeof(&self));
    self->_channel = channel;
    self->_type = type;
    self->_pull = pull;
    self->_scan_interval = _TWR_SWITCH_SCAN_INTERVAL;
    self->_debounce_time = _TWR_SWITCH_DEBOUNCE_TIME;
    self->_pull_advance_time = _TWR_SWITCH_PULL_ADVANCE_TIME_US;

    twr_gpio_init(channel);

    twr_gpio_set_mode(channel, TWR_GPIO_MODE_INPUT);

    self->_task_id = twr_scheduler_register(_twr_switch_task, self, 0);

    if ((self->_pull == TWR_SWITCH_PULL_UP_DYNAMIC) || (self->_pull == TWR_SWITCH_PULL_DOWN_DYNAMIC))
    {
        if (self->_pull_advance_time > 1000)
        {
            self->_task_state = TWR_SWITCH_TASK_STATE_SET_PULL;
        }

        twr_gpio_set_pull(self->_channel, TWR_GPIO_PULL_NONE);

        twr_timer_init();
    }
    else
    {
        twr_gpio_set_pull(self->_channel, _twr_switch_pull_lut[self->_pull]);
    }
}

bool twr_switch_get_state(twr_switch_t *self)
{
    return self->_pin_state != 0 ? TWR_SWITCH_OPEN : TWR_SWITCH_CLOSE;
}

void twr_switch_set_event_handler(twr_switch_t *self, void (*event_handler)(twr_switch_t *, twr_switch_event_t, void*), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_switch_set_scan_interval(twr_switch_t *self, twr_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;
}

void twr_switch_set_debounce_time(twr_switch_t *self, twr_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

void twr_switch_set_pull_advance_time(twr_switch_t *self, uint16_t pull_advance_time_us)
{
    self->_pull_advance_time = pull_advance_time_us;
}

static void _twr_switch_task(void *param)
{
    twr_switch_t *self = (twr_switch_t *) param;

    switch (self->_task_state)
    {
        case TWR_SWITCH_TASK_STATE_MEASURE:
        {
            bool dynamic = (self->_pull == TWR_SWITCH_PULL_UP_DYNAMIC) || (self->_pull == TWR_SWITCH_PULL_DOWN_DYNAMIC);

            if (dynamic)
            {
                if (self->_pull_advance_time < 1000)
                {
                    twr_gpio_set_pull(self->_channel, _twr_switch_pull_lut[self->_pull]);

                    twr_timer_start();

                    twr_timer_delay(self->_pull_advance_time);

                    twr_timer_stop();
                }
            }

            int pin_state = twr_gpio_get_input(self->_channel);

            if (dynamic)
            {
                twr_gpio_set_pull(self->_channel, TWR_GPIO_PULL_NONE);

                if (self->_pull_advance_time > 1000)
                {
                    self->_task_state = TWR_SWITCH_TASK_STATE_SET_PULL;
                }
            }

            if (self->_type == TWR_SWITCH_TYPE_NC)
            {
                pin_state = pin_state == 0 ? 1 : 0;
            }

            twr_tick_t tick_now = twr_scheduler_get_spin_tick();

            if (self->_pin_state != pin_state)
            {
                if (self->_tick_debounce == TWR_TICK_INFINITY)
                {
                    self->_tick_debounce = tick_now + self->_debounce_time;

                    twr_scheduler_plan_current_relative(self->_debounce_time);

                    return;
                }

                if (tick_now >= self->_tick_debounce)
                {
                    self->_pin_state = pin_state;

                    if (self->_event_handler != NULL)
                    {
                        if (self->_pin_state != 0)
                        {
                            self->_event_handler(self, TWR_SWITCH_EVENT_CLOSED, self->_event_param);
                        }
                        else
                        {
                            self->_event_handler(self, TWR_SWITCH_EVENT_OPENED, self->_event_param);
                        }
                    }
                }
            }
            else
            {
                self->_tick_debounce = TWR_TICK_INFINITY;
            }

            twr_scheduler_plan_current_relative(self->_scan_interval);

            return;

        }
        case TWR_SWITCH_TASK_STATE_SET_PULL:
        {
            twr_gpio_set_pull(self->_channel, _twr_switch_pull_lut[self->_pull]);

            twr_scheduler_plan_current_relative(self->_pull_advance_time / 1000);

            self->_task_state = TWR_SWITCH_TASK_STATE_MEASURE;

            return;
        }
        default:
        {
            return;
        }
    }
}
