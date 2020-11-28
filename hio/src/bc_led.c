#include <hio_led.h>

#define _HIO_LED_DEFAULT_SLOT_INTERVAL 100

static void _hio_led_gpio_init(hio_led_t *self)
{
    hio_gpio_init(self->_channel.gpio);
}

static void _hio_led_gpio_on(hio_led_t *self)
{
    hio_gpio_set_output(self->_channel.gpio, self->_idle_state ? 0 : 1);
}

static void _hio_led_gpio_off(hio_led_t *self)
{
    hio_gpio_set_output(self->_channel.gpio, self->_idle_state ? 1 : 0);
}

static const hio_led_driver_t _hio_led_driver_gpio =
{
    .init = _hio_led_gpio_init,
    .on = _hio_led_gpio_on,
    .off = _hio_led_gpio_off
};

static void _hio_led_task(void *param)
{
    hio_led_t *self = param;

    if (self->_pulse_active)
    {
        self->_driver->off(self);

        self->_pulse_active = false;
        self->_selector = 0;

        hio_scheduler_plan_current_relative(self->_slot_interval);

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

    hio_scheduler_plan_current_relative(self->_slot_interval);
}

void hio_led_init(hio_led_t *self, hio_gpio_channel_t gpio_channel, bool open_drain_output, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.gpio = gpio_channel;

    self->_open_drain_output = open_drain_output;

    self->_idle_state = idle_state;

    self->_driver = &_hio_led_driver_gpio;
    self->_driver->init(self);
    self->_driver->off(self);

    if (self->_open_drain_output)
    {
        hio_gpio_set_mode(self->_channel.gpio, HIO_GPIO_MODE_OUTPUT_OD);
    }
    else
    {
        hio_gpio_set_mode(self->_channel.gpio, HIO_GPIO_MODE_OUTPUT);
    }

    self->_slot_interval = _HIO_LED_DEFAULT_SLOT_INTERVAL;

    self->_task_id = hio_scheduler_register(_hio_led_task, self, HIO_TICK_INFINITY);
}

void hio_led_init_virtual(hio_led_t *self, int channel, const hio_led_driver_t *driver, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.virtual = channel;

    self->_idle_state = idle_state;

    self->_driver = driver;
    self->_driver->init(self);
    self->_driver->off(self);

    self->_slot_interval = _HIO_LED_DEFAULT_SLOT_INTERVAL;

    self->_task_id = hio_scheduler_register(_hio_led_task, self, HIO_TICK_INFINITY);
}

void hio_led_set_slot_interval(hio_led_t *self, hio_tick_t interval)
{
    self->_slot_interval = interval;
}

void hio_led_set_mode(hio_led_t *self, hio_led_mode_t mode)
{
    uint32_t pattern = self->_pattern;

    switch (mode)
    {
        case HIO_LED_MODE_TOGGLE:
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
        case HIO_LED_MODE_OFF:
        {
            self->_pattern = 0x00000000;
            self->_count = 0;

            if (!self->_pulse_active)
            {
                self->_driver->off(self);
            }

            return;
        }
        case HIO_LED_MODE_ON:
        {
            self->_pattern = 0xffffffff;
            self->_count = 0;

            if (!self->_pulse_active)
            {
                self->_driver->on(self);
            }

            return;
        }
        case HIO_LED_MODE_BLINK:
        {
            pattern = 0xf0f0f0f0;

            break;
        }
        case HIO_LED_MODE_BLINK_SLOW:
        {
            pattern = 0xffff0000;

            break;
        }
        case HIO_LED_MODE_BLINK_FAST:
        {
            pattern = 0xaaaaaaaa;

            break;
        }
        case HIO_LED_MODE_FLASH:
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

    hio_scheduler_plan_now(self->_task_id);
}

void hio_led_set_pattern(hio_led_t *self, uint32_t pattern)
{
    self->_pattern = pattern;
    self->_selector = 0;

    hio_scheduler_plan_now(self->_task_id);
}

void hio_led_set_count(hio_led_t *self, int count)
{
    self->_count = count;
}

void hio_led_blink(hio_led_t *self, int count)
{
    self->_pattern = 0xf0f0f0f0;
    self->_selector = 0;
    self->_count = count * 8;

    hio_scheduler_plan_now(self->_task_id);
}

void hio_led_pulse(hio_led_t *self, hio_tick_t duration)
{
    if (!self->_pulse_active)
    {
        self->_driver->on(self);

        self->_pulse_active = true;
    }

    hio_scheduler_plan_from_now(self->_task_id, duration);
}

bool hio_led_is_pulse(hio_led_t *self)
{
    return self->_pulse_active;
}
