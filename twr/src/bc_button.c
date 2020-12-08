#include <twr_button.h>

#define _TWR_BUTTON_SCAN_INTERVAL 20
#define _TWR_BUTTON_DEBOUNCE_TIME 50
#define _TWR_BUTTON_CLICK_TIMEOUT 500
#define _TWR_BUTTON_HOLD_TIME 2000

static void _twr_button_task(void *param);

static void _twr_button_gpio_init(twr_button_t *self);

static int _twr_button_gpio_get_input(twr_button_t *self);

static const twr_button_driver_t _twr_button_driver_gpio =
{
    .init = _twr_button_gpio_init,
    .get_input = _twr_button_gpio_get_input,
};

void twr_button_init(twr_button_t *self, twr_gpio_channel_t gpio_channel, twr_gpio_pull_t gpio_pull, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.gpio = gpio_channel;
    self->_gpio_pull = gpio_pull;
    self->_idle_state = idle_state;

    self->_scan_interval = _TWR_BUTTON_SCAN_INTERVAL;
    self->_debounce_time = _TWR_BUTTON_DEBOUNCE_TIME;
    self->_click_timeout = _TWR_BUTTON_CLICK_TIMEOUT;
    self->_hold_time = _TWR_BUTTON_HOLD_TIME;
    self->_tick_debounce = TWR_TICK_INFINITY;

    self->_driver = &_twr_button_driver_gpio;
    self->_driver->init(self);

    twr_gpio_set_pull(self->_channel.gpio, self->_gpio_pull);
    twr_gpio_set_mode(self->_channel.gpio, TWR_GPIO_MODE_INPUT);

    self->_task_id = twr_scheduler_register(_twr_button_task, self, TWR_TICK_INFINITY);
}

void twr_button_init_virtual(twr_button_t *self, int channel, const twr_button_driver_t *driver, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.virtual = channel;
    self->_idle_state = idle_state;

    self->_scan_interval = _TWR_BUTTON_SCAN_INTERVAL;
    self->_debounce_time = _TWR_BUTTON_DEBOUNCE_TIME;
    self->_click_timeout = _TWR_BUTTON_CLICK_TIMEOUT;
    self->_hold_time = _TWR_BUTTON_HOLD_TIME;
    self->_tick_debounce = TWR_TICK_INFINITY;

    self->_driver = driver;

    if (self->_driver->init != NULL)
    {
        self->_driver->init(self);
    }

    self->_task_id = twr_scheduler_register(_twr_button_task, self, TWR_TICK_INFINITY);
}

void twr_button_set_event_handler(twr_button_t *self, void (*event_handler)(twr_button_t *, twr_button_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;

    if (event_handler == NULL)
    {
        self->_tick_debounce = TWR_TICK_INFINITY;

        twr_scheduler_plan_absolute(self->_task_id, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_now(self->_task_id);
    }
}

void twr_button_set_scan_interval(twr_button_t *self, twr_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;
}

void twr_button_set_debounce_time(twr_button_t *self, twr_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

void twr_button_set_click_timeout(twr_button_t *self, twr_tick_t click_timeout)
{
    self->_click_timeout = click_timeout;
}

void twr_button_set_hold_time(twr_button_t *self, twr_tick_t hold_time)
{
    self->_hold_time = hold_time;
}

static void _twr_button_task(void *param)
{
    twr_button_t *self = param;

    twr_tick_t tick_now = twr_scheduler_get_spin_tick();

    int pin_state;

    if (self->_driver->get_input != NULL)
    {
        pin_state = self->_driver->get_input(self);
    }
    else
    {
        pin_state = self->_idle_state;
    }

    if (self->_idle_state)
    {
        pin_state = pin_state == 0 ? 1 : 0;
    }

    if ((self->_state == 0 && pin_state != 0) || (self->_state != 0 && pin_state == 0))
    {
        if (self->_tick_debounce == TWR_TICK_INFINITY)
        {
            self->_tick_debounce = tick_now + self->_debounce_time;
        }

        if (tick_now >= self->_tick_debounce)
        {
            self->_state = self->_state == 0 ? 1 : 0;

            if (self->_state != 0)
            {
                self->_tick_click_timeout = tick_now + self->_click_timeout;
                self->_tick_hold_threshold = tick_now + self->_hold_time;
                self->_hold_signalized = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_BUTTON_EVENT_PRESS, self->_event_param);
                }
            }
            else
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_BUTTON_EVENT_RELEASE, self->_event_param);
                }

                if (tick_now < self->_tick_click_timeout)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, TWR_BUTTON_EVENT_CLICK, self->_event_param);
                    }
                }
            }
        }
    }
    else
    {
        self->_tick_debounce = TWR_TICK_INFINITY;
    }

    if (self->_state != 0)
    {
        if (!self->_hold_signalized)
        {
            if (tick_now >= self->_tick_hold_threshold)
            {
                self->_hold_signalized = true;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_BUTTON_EVENT_HOLD, self->_event_param);
                }
            }
        }
    }

    twr_scheduler_plan_current_relative(self->_scan_interval);
}

static void _twr_button_gpio_init(twr_button_t *self)
{
    twr_gpio_init(self->_channel.gpio);
}

static int _twr_button_gpio_get_input(twr_button_t *self)
{
    return twr_gpio_get_input(self->_channel.gpio);
}
