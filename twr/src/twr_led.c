#include <twr_led.h>

#define _TWR_LED_DEFAULT_SLOT_INTERVAL 100

static void _twr_led_gpio_init(twr_led_t *self)
{
    twr_gpio_init(self->_channel.gpio);
}

static void _twr_led_gpio_on(twr_led_t *self)
{
    twr_gpio_set_output(self->_channel.gpio, self->_idle_state ? 0 : 1);
}

static void _twr_led_gpio_off(twr_led_t *self)
{
    twr_gpio_set_output(self->_channel.gpio, self->_idle_state ? 1 : 0);
}

static const twr_led_driver_t _twr_led_driver_gpio =
{
    .init = _twr_led_gpio_init,
    .on = _twr_led_gpio_on,
    .off = _twr_led_gpio_off
};

static void _twr_led_task(void *param)
{
    twr_led_t *self = param;

    if (self->_pulse_active)
    {
        self->_driver->off(self);

        self->_pulse_active = false;
        self->_selector = 0;

        twr_scheduler_plan_current_relative(self->_slot_interval);

        return;
    }

    if (self->_pattern == 0 || self->_pattern == 0xffffffff)
    {
        return;
    }

    if (self->_selector == 0)
    {
        self->_selector = 0x80000000;
    }

    if ((self->_pattern & self->_selector) != 0)
    {
        self->_driver->on(self);
    }
    else
    {
        self->_driver->off(self);
    }

    self->_selector >>= 1;

    if (self->_count != 0)
    {
        if (--self->_count == 0)
        {
            self->_pattern = 0;
        }
    }

    twr_scheduler_plan_current_relative(self->_slot_interval);
}

void twr_led_init(twr_led_t *self, twr_gpio_channel_t gpio_channel, bool open_drain_output, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.gpio = gpio_channel;

    self->_open_drain_output = open_drain_output;

    self->_idle_state = idle_state;

    self->_driver = &_twr_led_driver_gpio;
    self->_driver->init(self);
    self->_driver->off(self);

    if (self->_open_drain_output)
    {
        twr_gpio_set_mode(self->_channel.gpio, TWR_GPIO_MODE_OUTPUT_OD);
    }
    else
    {
        twr_gpio_set_mode(self->_channel.gpio, TWR_GPIO_MODE_OUTPUT);
    }

    self->_slot_interval = _TWR_LED_DEFAULT_SLOT_INTERVAL;

    self->_task_id = twr_scheduler_register(_twr_led_task, self, TWR_TICK_INFINITY);
}

void twr_led_init_virtual(twr_led_t *self, int channel, const twr_led_driver_t *driver, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.virtual = channel;

    self->_idle_state = idle_state;

    self->_driver = driver;
    self->_driver->init(self);
    self->_driver->off(self);

    self->_slot_interval = _TWR_LED_DEFAULT_SLOT_INTERVAL;

    self->_task_id = twr_scheduler_register(_twr_led_task, self, TWR_TICK_INFINITY);
}

void twr_led_set_slot_interval(twr_led_t *self, twr_tick_t interval)
{
    self->_slot_interval = interval;
}

void twr_led_set_mode(twr_led_t *self, twr_led_mode_t mode)
{
    uint32_t pattern = self->_pattern;

    switch (mode)
    {
        case TWR_LED_MODE_TOGGLE:
        {
            if (pattern == 0)
            {
                self->_pattern = 0xffffffff;
                self->_count = 0;

                self->_driver->on(self);
            }
            else if (pattern == 0xffffffff)
            {
                self->_pattern = 0;
                self->_count = 0;

                if (!self->_pulse_active)
                {
                    self->_driver->off(self);
                }
            }

            return;
        }
        case TWR_LED_MODE_OFF:
        {
            self->_pattern = 0x00000000;
            self->_count = 0;

            if (!self->_pulse_active)
            {
                self->_driver->off(self);
            }

            return;
        }
        case TWR_LED_MODE_ON:
        {
            self->_pattern = 0xffffffff;
            self->_count = 0;

            if (!self->_pulse_active)
            {
                self->_driver->on(self);
            }

            return;
        }
        case TWR_LED_MODE_BLINK:
        {
            pattern = 0xf0f0f0f0;

            break;
        }
        case TWR_LED_MODE_BLINK_SLOW:
        {
            pattern = 0xffff0000;

            break;
        }
        case TWR_LED_MODE_BLINK_FAST:
        {
            pattern = 0xaaaaaaaa;

            break;
        }
        case TWR_LED_MODE_FLASH:
        {
            pattern = 0x80000000;

            break;
        }
        default:
        {
            break;
        }
    }

    if (self->_pattern != pattern)
    {
        self->_pattern = pattern;
        self->_selector = 0;
        self->_count = 0;
    }

    twr_scheduler_plan_now(self->_task_id);
}

void twr_led_set_pattern(twr_led_t *self, uint32_t pattern)
{
    self->_pattern = pattern;
    self->_selector = 0;

    twr_scheduler_plan_now(self->_task_id);
}

void twr_led_set_count(twr_led_t *self, int count)
{
    self->_count = count;
}

void twr_led_blink(twr_led_t *self, int count)
{
    self->_pattern = 0xf0f0f0f0;
    self->_selector = 0;
    self->_count = count * 8;

    twr_scheduler_plan_now(self->_task_id);
}

void twr_led_pulse(twr_led_t *self, twr_tick_t duration)
{
    if (!self->_pulse_active)
    {
        self->_driver->on(self);

        self->_pulse_active = true;
    }

    twr_scheduler_plan_from_now(self->_task_id, duration);
}

bool twr_led_is_pulse(twr_led_t *self)
{
    return self->_pulse_active;
}
