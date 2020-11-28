#include <hio_switch.h>
#include <hio_timer.h>

#define _HIO_SWITCH_SCAN_INTERVAL        50
#define _HIO_SWITCH_DEBOUNCE_TIME        20
#define _HIO_SWITCH_PULL_ADVANCE_TIME_US 50

static void _hio_switch_task(void *param);

static const hio_gpio_pull_t _hio_switch_pull_lut[5] = {
        [HIO_SWITCH_PULL_NONE] = HIO_GPIO_PULL_NONE,
        [HIO_SWITCH_PULL_UP] = HIO_GPIO_PULL_UP,
        [HIO_SWITCH_PULL_UP_DYNAMIC] = HIO_GPIO_PULL_UP,
        [HIO_SWITCH_PULL_DOWN] = HIO_GPIO_PULL_DOWN,
        [HIO_SWITCH_PULL_DOWN_DYNAMIC] = HIO_GPIO_PULL_DOWN
};

void hio_switch_init(hio_switch_t *self, hio_gpio_channel_t channel, hio_switch_type_t type, hio_switch_pull_t pull)
{
    memset(self, 0, sizeof(&self));
    self->_channel = channel;
    self->_type = type;
    self->_pull = pull;
    self->_scan_interval = _HIO_SWITCH_SCAN_INTERVAL;
    self->_debounce_time = _HIO_SWITCH_DEBOUNCE_TIME;
    self->_pull_advance_time = _HIO_SWITCH_PULL_ADVANCE_TIME_US;

    hio_gpio_init(channel);

    hio_gpio_set_mode(channel, HIO_GPIO_MODE_INPUT);

    self->_task_id = hio_scheduler_register(_hio_switch_task, self, 0);

    if ((self->_pull == HIO_SWITCH_PULL_UP_DYNAMIC) || (self->_pull == HIO_SWITCH_PULL_DOWN_DYNAMIC))
    {
        if (self->_pull_advance_time > 1000)
        {
            self->_task_state = HIO_SWITCH_TASK_STATE_SET_PULL;
        }

        hio_gpio_set_pull(self->_channel, HIO_GPIO_PULL_NONE);

        hio_timer_init();
    }
    else
    {
        hio_gpio_set_pull(self->_channel, _hio_switch_pull_lut[self->_pull]);
    }
}

bool hio_switch_get_state(hio_switch_t *self)
{
    return self->_pin_state != 0 ? HIO_SWITCH_OPEN : HIO_SWITCH_CLOSE;
}

void hio_switch_set_event_handler(hio_switch_t *self, void (*event_handler)(hio_switch_t *, hio_switch_event_t, void*), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_switch_set_scan_interval(hio_switch_t *self, hio_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;
}

void hio_switch_set_debounce_time(hio_switch_t *self, hio_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

void hio_switch_set_pull_advance_time(hio_switch_t *self, uint16_t pull_advance_time_us)
{
    self->_pull_advance_time = pull_advance_time_us;
}

static void _hio_switch_task(void *param)
{
    hio_switch_t *self = (hio_switch_t *) param;

    switch (self->_task_state)
    {
        case HIO_SWITCH_TASK_STATE_MEASURE:
        {
            bool dynamic = (self->_pull == HIO_SWITCH_PULL_UP_DYNAMIC) || (self->_pull == HIO_SWITCH_PULL_DOWN_DYNAMIC);

            if (dynamic)
            {
                if (self->_pull_advance_time < 1000)
                {
                    hio_gpio_set_pull(self->_channel, _hio_switch_pull_lut[self->_pull]);

                    hio_timer_start();

                    hio_timer_delay(self->_pull_advance_time);

                    hio_timer_stop();
                }
            }

            int pin_state = hio_gpio_get_input(self->_channel);

            if (dynamic)
            {
                hio_gpio_set_pull(self->_channel, HIO_GPIO_PULL_NONE);

                if (self->_pull_advance_time > 1000)
                {
                    self->_task_state = HIO_SWITCH_TASK_STATE_SET_PULL;
                }
            }

            if (self->_type == HIO_SWITCH_TYPE_NC)
            {
                pin_state = pin_state == 0 ? 1 : 0;
            }

            hio_tick_t tick_now = hio_scheduler_get_spin_tick();

            if (self->_pin_state != pin_state)
            {
                if (self->_tick_debounce == HIO_TICK_INFINITY)
                {
                    self->_tick_debounce = tick_now + self->_debounce_time;

                    hio_scheduler_plan_current_relative(self->_debounce_time);

                    return;
                }

                if (tick_now >= self->_tick_debounce)
                {
                    self->_pin_state = pin_state;

                    if (self->_event_handler != NULL)
                    {
                        if (self->_pin_state != 0)
                        {
                            self->_event_handler(self, HIO_SWITCH_EVENT_CLOSED, self->_event_param);
                        }
                        else
                        {
                            self->_event_handler(self, HIO_SWITCH_EVENT_OPENED, self->_event_param);
                        }
                    }
                }
            }
            else
            {
                self->_tick_debounce = HIO_TICK_INFINITY;
            }

            hio_scheduler_plan_current_relative(self->_scan_interval);

            return;

        }
        case HIO_SWITCH_TASK_STATE_SET_PULL:
        {
            hio_gpio_set_pull(self->_channel, _hio_switch_pull_lut[self->_pull]);

            hio_scheduler_plan_current_relative(self->_pull_advance_time / 1000);

            self->_task_state = HIO_SWITCH_TASK_STATE_MEASURE;

            return;
        }
        default:
        {
            return;
        }
    }
}
