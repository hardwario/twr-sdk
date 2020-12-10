#include <twr_module_encoder.h>
#include <twr_scheduler.h>
#include <twr_exti.h>
#include <twr_irq.h>
#include <twr_system.h>
#include <twr_timer.h>

static struct
{
    bool initialized;
    twr_scheduler_task_id_t task_id;
    void (*event_handler)(twr_module_encoder_event_t, void *);
    void *event_param;
    twr_button_t button;
    int increment;
    int increment_shadow;
    int samples;

} _twr_module_encoder;

static void _twr_module_encoder_task(void *param);

static void _twr_module_encoder_exti_handler(twr_exti_line_t line, void *param);

static void _twr_module_encoder_button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param);

static bool _twr_module_encoder_present_test(void);

void twr_module_encoder_init(void)
{
    memset(&_twr_module_encoder, 0, sizeof(_twr_module_encoder));

    // Initialize encoder button
    twr_button_init(&_twr_module_encoder.button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&_twr_module_encoder.button, _twr_module_encoder_button_event_handler, NULL);

    // Init encoder GPIO pins
    twr_gpio_init(TWR_GPIO_P4);

    twr_gpio_init(TWR_GPIO_P5);

    // Register task
    _twr_module_encoder.task_id = twr_scheduler_register(_twr_module_encoder_task, NULL, 0);
}

void twr_module_encoder_deinit(void)
{
    // Unregister task
    twr_scheduler_unregister(_twr_module_encoder.task_id);

    twr_gpio_set_mode(TWR_GPIO_P4, TWR_GPIO_MODE_ANALOG);

    twr_gpio_set_mode(TWR_GPIO_P5, TWR_GPIO_MODE_ANALOG);

    if (_twr_module_encoder.initialized)
    {
        twr_exti_unregister(TWR_EXTI_LINE_P4);

        twr_exti_unregister(TWR_EXTI_LINE_P5);
    }
}

void twr_module_encoder_set_event_handler(void (*event_handler)(twr_module_encoder_event_t, void *), void *event_param)
{
    _twr_module_encoder.event_handler = event_handler;

    _twr_module_encoder.event_param = event_param;
}

twr_button_t *twr_module_encoder_get_button_instance(void)
{
    return &_twr_module_encoder.button;
}

int twr_module_encoder_get_increment(void)
{
    return _twr_module_encoder.increment_shadow;
}

bool twr_module_encoder_is_present(void)
{
    if (_twr_module_encoder.initialized)
    {
        return true;
    }

    return _twr_module_encoder_present_test();
}

static void _twr_module_encoder_task(void *param)
{
    (void) param;

    if (_twr_module_encoder.initialized)
    {
        // Disable interrupts
        twr_irq_disable();

        _twr_module_encoder.increment_shadow = _twr_module_encoder.increment;
        _twr_module_encoder.increment = 0;

        // Enable interrupts
        twr_irq_enable();

        if (_twr_module_encoder.increment_shadow != 0)
        {
            if (_twr_module_encoder.event_handler != NULL)
            {
                _twr_module_encoder.event_handler(TWR_MODULE_ENCODER_EVENT_ROTATION, _twr_module_encoder.event_param);
            }
        }
    }
    else
    {
        if (_twr_module_encoder_present_test())
        {
            // Set encoder GPIO pins as inputs
            twr_gpio_set_mode(TWR_GPIO_P4, TWR_GPIO_MODE_INPUT);
            twr_gpio_set_mode(TWR_GPIO_P5, TWR_GPIO_MODE_INPUT);

            // Remember initial GPIO states
            _twr_module_encoder.samples |= twr_gpio_get_input(TWR_GPIO_P4) ? 0x1 : 0;
            _twr_module_encoder.samples |= twr_gpio_get_input(TWR_GPIO_P5) ? 0x2 : 0;

            // Register interrupts on both GPIO pins
            twr_exti_register(TWR_EXTI_LINE_P4, TWR_EXTI_EDGE_RISING_AND_FALLING, _twr_module_encoder_exti_handler, NULL);
            twr_exti_register(TWR_EXTI_LINE_P5, TWR_EXTI_EDGE_RISING_AND_FALLING, _twr_module_encoder_exti_handler, NULL);

            _twr_module_encoder.initialized = true;
        }
        else
        {
            if (_twr_module_encoder.event_handler != NULL)
            {
                _twr_module_encoder.event_handler(TWR_MODULE_ENCODER_EVENT_ERROR, _twr_module_encoder.event_param);
            }
        }
    }
}

static void _twr_module_encoder_exti_handler(twr_exti_line_t line, void *param)
{
    (void) line;
    (void) param;

    // Read current GPIO state
    _twr_module_encoder.samples <<= 2;
    _twr_module_encoder.samples |= twr_gpio_get_input(TWR_GPIO_P4) ? 0x1 : 0;
    _twr_module_encoder.samples |= twr_gpio_get_input(TWR_GPIO_P5) ? 0x2 : 0;
    _twr_module_encoder.samples &= 0xf;

    if (_twr_module_encoder.samples == 0x7)
    {
        _twr_module_encoder.increment--;

        twr_scheduler_plan_now(_twr_module_encoder.task_id);
    }
    else if (_twr_module_encoder.samples == 0xd)
    {
        _twr_module_encoder.increment++;

        twr_scheduler_plan_now(_twr_module_encoder.task_id);
    }
}

static void _twr_module_encoder_button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_twr_module_encoder.event_handler == NULL)
    {
        return;
    }

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        _twr_module_encoder.event_handler(TWR_MODULE_ENCODER_EVENT_PRESS, _twr_module_encoder.event_param);
    }
    else if (event == TWR_BUTTON_EVENT_RELEASE)
    {
        _twr_module_encoder.event_handler(TWR_MODULE_ENCODER_EVENT_RELEASE, _twr_module_encoder.event_param);
    }
    else if (event == TWR_BUTTON_EVENT_CLICK)
    {
        _twr_module_encoder.event_handler(TWR_MODULE_ENCODER_EVENT_CLICK, _twr_module_encoder.event_param);
    }
    else if (event == TWR_BUTTON_EVENT_HOLD)
    {
        _twr_module_encoder.event_handler(TWR_MODULE_ENCODER_EVENT_HOLD, _twr_module_encoder.event_param);
    }
}

static bool _twr_module_encoder_test_pin(twr_gpio_channel_t channel)
{
    twr_gpio_set_mode(channel, TWR_GPIO_MODE_OUTPUT);

    twr_gpio_set_output(channel, 1);

    twr_gpio_set_pull(channel, TWR_GPIO_PULL_DOWN);

    twr_gpio_set_mode(channel, TWR_GPIO_MODE_INPUT);

    twr_timer_start();

    twr_timer_delay(20);

    twr_timer_stop();

    int value = twr_gpio_get_input(channel);

    twr_gpio_set_mode(channel, TWR_GPIO_MODE_ANALOG);

    twr_gpio_set_pull(channel, TWR_GPIO_PULL_NONE);

    return value != 0;
}

static bool _twr_module_encoder_present_test(void)
{
    twr_system_pll_enable();

    twr_timer_init();

    twr_gpio_init(TWR_GPIO_P4);

    twr_gpio_init(TWR_GPIO_P5);

    bool is_present = _twr_module_encoder_test_pin(TWR_GPIO_P4) && _twr_module_encoder_test_pin(TWR_GPIO_P5);

    twr_system_pll_disable();

    return is_present;
}
