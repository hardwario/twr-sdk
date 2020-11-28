#include <hio_button.h>

#define _HIO_BUTTON_SCAN_INTERVAL 20
#define _HIO_BUTTON_DEBOUNCE_TIME 50
#define _HIO_BUTTON_CLICK_TIMEOUT 500
#define _HIO_BUTTON_HOLD_TIME 2000

static void _hio_button_task(void *param);

static void _hio_button_gpio_init(hio_button_t *self);

static int _hio_button_gpio_get_input(hio_button_t *self);

static const hio_button_driver_t _hio_button_driver_gpio =
{
    .init = _hio_button_gpio_init,
    .get_input = _hio_button_gpio_get_input,
};

void hio_button_init(hio_button_t *self, hio_gpio_channel_t gpio_channel, hio_gpio_pull_t gpio_pull, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.gpio = gpio_channel;
    self->_gpio_pull = gpio_pull;
    self->_idle_state = idle_state;

    self->_scan_interval = _HIO_BUTTON_SCAN_INTERVAL;
    self->_debounce_time = _HIO_BUTTON_DEBOUNCE_TIME;
    self->_click_timeout = _HIO_BUTTON_CLICK_TIMEOUT;
    self->_hold_time = _HIO_BUTTON_HOLD_TIME;
    self->_tick_debounce = HIO_TICK_INFINITY;

    self->_driver = &_hio_button_driver_gpio;
    self->_driver->init(self);

    hio_gpio_set_pull(self->_channel.gpio, self->_gpio_pull);
    hio_gpio_set_mode(self->_channel.gpio, HIO_GPIO_MODE_INPUT);

    self->_task_id = hio_scheduler_register(_hio_button_task, self, HIO_TICK_INFINITY);
}

void hio_button_init_virtual(hio_button_t *self, int channel, const hio_button_driver_t *driver, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.virtual = channel;
    self->_idle_state = idle_state;

    self->_scan_interval = _HIO_BUTTON_SCAN_INTERVAL;
    self->_debounce_time = _HIO_BUTTON_DEBOUNCE_TIME;
    self->_click_timeout = _HIO_BUTTON_CLICK_TIMEOUT;
    self->_hold_time = _HIO_BUTTON_HOLD_TIME;
    self->_tick_debounce = HIO_TICK_INFINITY;

    self->_driver = driver;

    if (self->_driver->init != NULL)
    {
        self->_driver->init(self);
    }

    self->_task_id = hio_scheduler_register(_hio_button_task, self, HIO_TICK_INFINITY);
}

void hio_button_set_event_handler(hio_button_t *self, void (*event_handler)(hio_button_t *, hio_button_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;

    if (event_handler == NULL)
    {
        self->_tick_debounce = HIO_TICK_INFINITY;

        hio_scheduler_plan_absolute(self->_task_id, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_now(self->_task_id);
    }
}

void hio_button_set_scan_interval(hio_button_t *self, hio_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;
}

void hio_button_set_debounce_time(hio_button_t *self, hio_tick_t debounce_time)
{
    self->_debounce_time = debounce_time;
}

void hio_button_set_click_timeout(hio_button_t *self, hio_tick_t click_timeout)
{
    self->_click_timeout = click_timeout;
}

void hio_button_set_hold_time(hio_button_t *self, hio_tick_t hold_time)
{
    self->_hold_time = hold_time;
}

static void _hio_button_task(void *param)
{
    hio_button_t *self = param;

    hio_tick_t tick_now = hio_scheduler_get_spin_tick();

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
        if (self->_tick_debounce == HIO_TICK_INFINITY)
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
                    self->_event_handler(self, HIO_BUTTON_EVENT_PRESS, self->_event_param);
                }
            }
            else
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, HIO_BUTTON_EVENT_RELEASE, self->_event_param);
                }

                if (tick_now < self->_tick_click_timeout)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, HIO_BUTTON_EVENT_CLICK, self->_event_param);
                    }
                }
            }
        }
    }
    else
    {
        self->_tick_debounce = HIO_TICK_INFINITY;
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
                    self->_event_handler(self, HIO_BUTTON_EVENT_HOLD, self->_event_param);
                }
            }
        }
    }

    hio_scheduler_plan_current_relative(self->_scan_interval);
}

static void _hio_button_gpio_init(hio_button_t *self)
{
    hio_gpio_init(self->_channel.gpio);
}

static int _hio_button_gpio_get_input(hio_button_t *self)
{
    return hio_gpio_get_input(self->_channel.gpio);
}
