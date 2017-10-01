#include <bc_led.h>

#define BC_LED_DEFAULT_SLOT_INTERVAL 100

static void _bc_led_task(void *param);

static void _bc_led_task_pulse(void *param);

static void _bc_led_gpio_init(bc_led_t *self);

static void _bc_led_gpio_on(bc_led_t *self);

static void _bc_led_gpio_off(bc_led_t *self);

static const bc_led_driver_t _bc_led_driver_gpio =
{
        .init = _bc_led_gpio_init,
        .on =   _bc_led_gpio_on,
        .off =  _bc_led_gpio_off,
};

void bc_led_init(bc_led_t *self, bc_gpio_channel_t gpio_channel, bool open_drain_output, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.gpio_channel = gpio_channel;

    self->_open_drain_output = open_drain_output;

    self->_idle_state = idle_state;

    self->_driver = &_bc_led_driver_gpio;

    self->_driver->init(self);

    self->_driver->off(self);

    if (self->_open_drain_output)
    {
        bc_gpio_set_mode(self->_channel.gpio_channel, BC_GPIO_MODE_OUTPUT_OD);
    }
    else
    {
        bc_gpio_set_mode(self->_channel.gpio_channel, BC_GPIO_MODE_OUTPUT);
    }

    self->_slot_interval = BC_LED_DEFAULT_SLOT_INTERVAL;

    bc_scheduler_register(_bc_led_task, self, self->_slot_interval);
}

void bc_led_init_virtual(bc_led_t *self, int channel, const bc_led_driver_t *driver, int idle_state)
{
    memset(self, 0, sizeof(*self));

    self->_channel.virtual_channel = channel;

    self->_idle_state = idle_state;

    self->_driver = driver;

    self->_driver->init(self);

    self->_driver->off(self);

    self->_slot_interval = BC_LED_DEFAULT_SLOT_INTERVAL;

    bc_scheduler_register(_bc_led_task, self, self->_slot_interval);
}

void bc_led_set_slot_interval(bc_led_t *self, bc_tick_t interval)
{
    self->_slot_interval = interval;
}

void bc_led_set_mode(bc_led_t *self, bc_led_mode_t mode)
{
    uint32_t pattern = self->_pattern;

    switch (mode)
    {
        case BC_LED_MODE_TOGGLE:
        {
            if (pattern == 0x00000000)
            {
                pattern = 0xffffffff;

                self->_driver->on(self);
            }
            else if (pattern == 0xffffffff)
            {
                pattern = 0x00000000;

                if (!self->_pulse_active)
                {
                    self->_driver->off(self);
                }
            }

            break;
        }
        case BC_LED_MODE_OFF:
        {
            pattern = 0x00000000;

            if (!self->_pulse_active)
            {
                self->_driver->off(self);
            }

            break;
        }
        case BC_LED_MODE_ON:
        {
            pattern = 0xffffffff;

            if (!self->_pulse_active)
            {
                self->_driver->on(self);
            }

            break;
        }
        case BC_LED_MODE_BLINK:
        {
            pattern = 0xf0f0f0f0;
            break;
        }
        case BC_LED_MODE_BLINK_SLOW:
        {
            pattern = 0xffff0000;
            break;
        }
        case BC_LED_MODE_BLINK_FAST:
        {
            pattern = 0xaaaaaaaa;
            break;
        }
        case BC_LED_MODE_FLASH:
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
    }
}

void bc_led_set_pattern(bc_led_t *self, uint32_t pattern)
{
    if (self->_pattern != pattern)
    {
        self->_pattern = pattern;

        self->_selector = 0;
    }
}

void bc_led_pulse(bc_led_t *self, bc_tick_t duration)
{
    if (duration == 0)
    {
        if (self->_pulse_active)
        {
            self->_driver->off(self);

            self->_pulse_active = false;

            bc_scheduler_unregister(self->_pulse_task_id);
        }

        return;
    }

    if (self->_pulse_active)
    {
        bc_scheduler_plan_from_now(self->_pulse_task_id, duration);

        return;
    }

    self->_driver->on(self);

    self->_pulse_active = true;

    self->_pulse_task_id = bc_scheduler_register(_bc_led_task_pulse, self, bc_tick_get() + duration);
}

bool bc_led_is_pulse(bc_led_t *self)
{
    return self->_pulse_active;
}

static void _bc_led_task(void *param)
{
    bc_led_t *self = param;

    if (self->_pulse_active)
    {
        bc_scheduler_plan_current_relative(self->_slot_interval);

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

    bc_scheduler_plan_current_relative(self->_slot_interval);
}

static void _bc_led_task_pulse(void *param)
{
    bc_led_t *self = param;

    self->_driver->off(self);

    self->_pulse_active = false;

    bc_scheduler_unregister(self->_pulse_task_id);
}

static void _bc_led_gpio_init(bc_led_t *self)
{
    bc_gpio_init(self->_channel.gpio_channel);
}

static void _bc_led_gpio_on(bc_led_t *self)
{
    bc_gpio_set_output(self->_channel.gpio_channel, self->_idle_state ? 0 : 1);
}

static void _bc_led_gpio_off(bc_led_t *self)
{
    bc_gpio_set_output(self->_channel.gpio_channel, self->_idle_state ? 1 : 0);
}
