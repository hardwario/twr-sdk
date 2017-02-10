#include <bc_button.h>
#include <bc_scheduler.h>

#define _BC_BUTTON_SCAN_INTERVAL 20
#define _BC_BUTTON_DEBOUNCE_TIME 50
#define _BC_BUTTON_CLICK_TIMEOUT 500
#define _BC_BUTTON_HOLD_TIME 2000

static void _bc_button_task(void *param);

void bc_button_init(bc_button_t *self, bc_gpio_channel_t gpio_channel, bc_gpio_pull_t gpio_pull, bool idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_gpio_channel = gpio_channel;
    self->_gpio_pull = gpio_pull;
    self->_idle_state = idle_state;

    self->_scan_interval = _BC_BUTTON_SCAN_INTERVAL;
    self->_debounce_time = _BC_BUTTON_DEBOUNCE_TIME;
    self->_click_timeout = _BC_BUTTON_CLICK_TIMEOUT;
    self->_hold_time = _BC_BUTTON_HOLD_TIME;

    bc_gpio_init(self->_gpio_channel);
    bc_gpio_set_pull(self->_gpio_channel, self->_gpio_pull);
    bc_gpio_set_mode(self->_gpio_channel, BC_GPIO_MODE_INPUT);

    bc_scheduler_register(_bc_button_task, self, self->_scan_interval);
}

void bc_button_set_event_handler(bc_button_t *self, void (*event_handler)(bc_button_t *, bc_button_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_button_set_scan_interval(bc_button_t *self, bc_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;
}

void bc_button_set_debounce_time(bc_button_t *self, bc_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

void bc_button_set_click_timeout(bc_button_t *self, bc_tick_t click_timeout)
{
    self->_click_timeout = click_timeout;
}

void bc_button_set_hold_time(bc_button_t *self, bc_tick_t hold_time)
{
    self->_hold_time = hold_time;
}

static void _bc_button_task(void *param)
{
    bc_button_t *self = param;

    bc_tick_t tick_now = bc_scheduler_get_spin_tick();

    bool pin_state = bc_gpio_get_input(self->_gpio_channel);

    if (self->_idle_state)
    {
        pin_state = !pin_state;
    }

    if ((!self->_state && pin_state) || (self->_state && !pin_state))
    {
        if (tick_now >= self->_tick_debounce)
        {
            self->_state = !self->_state;

            if (self->_state)
            {
                self->_tick_click_timeout = tick_now + self->_click_timeout;
                self->_tick_hold_threshold = tick_now + self->_hold_time;
                self->_hold_signalized = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_BUTTON_EVENT_PRESS, self->_event_param);
                }
            }
            else
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_BUTTON_EVENT_RELEASE, self->_event_param);
                }

                if (tick_now < self->_tick_click_timeout)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_BUTTON_EVENT_CLICK, self->_event_param);
                    }
                }
            }
        }
    }
    else
    {
        self->_tick_debounce = tick_now + self->_debounce_time;
    }

    if (self->_state)
    {
        if (!self->_hold_signalized)
        {
            if (tick_now >= self->_tick_hold_threshold)
            {
                self->_hold_signalized = true;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_BUTTON_EVENT_HOLD, self->_event_param);
                }
            }
        }
    }

    bc_scheduler_plan_current_relative(self->_scan_interval);
}
