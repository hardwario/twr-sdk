#include <hio_module_encoder.h>
#include <hio_scheduler.h>
#include <hio_exti.h>
#include <hio_irq.h>
#include <hio_system.h>
#include <hio_timer.h>

static struct
{
    bool initialized;
    hio_scheduler_task_id_t task_id;
    void (*event_handler)(hio_module_encoder_event_t, void *);
    void *event_param;
    hio_button_t button;
    int increment;
    int increment_shadow;
    int samples;

} _hio_module_encoder;

static void _hio_module_encoder_task(void *param);

static void _hio_module_encoder_exti_handler(hio_exti_line_t line, void *param);

static void _hio_module_encoder_button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param);

static bool _hio_module_encoder_present_test(void);

void hio_module_encoder_init(void)
{
    memset(&_hio_module_encoder, 0, sizeof(_hio_module_encoder));

    // Initialize encoder button
    hio_button_init(&_hio_module_encoder.button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&_hio_module_encoder.button, _hio_module_encoder_button_event_handler, NULL);

    // Init encoder GPIO pins
    hio_gpio_init(HIO_GPIO_P4);

    hio_gpio_init(HIO_GPIO_P5);

    // Register task
    _hio_module_encoder.task_id = hio_scheduler_register(_hio_module_encoder_task, NULL, 0);
}

void hio_module_encoder_deinit(void)
{
    // Unregister task
    hio_scheduler_unregister(_hio_module_encoder.task_id);

    hio_gpio_set_mode(HIO_GPIO_P4, HIO_GPIO_MODE_ANALOG);

    hio_gpio_set_mode(HIO_GPIO_P5, HIO_GPIO_MODE_ANALOG);

    if (_hio_module_encoder.initialized)
    {
        hio_exti_unregister(HIO_EXTI_LINE_P4);

        hio_exti_unregister(HIO_EXTI_LINE_P5);
    }
}

void hio_module_encoder_set_event_handler(void (*event_handler)(hio_module_encoder_event_t, void *), void *event_param)
{
    _hio_module_encoder.event_handler = event_handler;

    _hio_module_encoder.event_param = event_param;
}

hio_button_t *hio_module_encoder_get_button_instance(void)
{
    return &_hio_module_encoder.button;
}

int hio_module_encoder_get_increment(void)
{
    return _hio_module_encoder.increment_shadow;
}

bool hio_module_encoder_is_present(void)
{
    if (_hio_module_encoder.initialized)
    {
        return true;
    }

    return _hio_module_encoder_present_test();
}

static void _hio_module_encoder_task(void *param)
{
    (void) param;

    if (_hio_module_encoder.initialized)
    {
        // Disable interrupts
        hio_irq_disable();

        _hio_module_encoder.increment_shadow = _hio_module_encoder.increment;
        _hio_module_encoder.increment = 0;

        // Enable interrupts
        hio_irq_enable();

        if (_hio_module_encoder.increment_shadow != 0)
        {
            if (_hio_module_encoder.event_handler != NULL)
            {
                _hio_module_encoder.event_handler(HIO_MODULE_ENCODER_EVENT_ROTATION, _hio_module_encoder.event_param);
            }
        }
    }
    else
    {
        if (_hio_module_encoder_present_test())
        {
            // Set encoder GPIO pins as inputs
            hio_gpio_set_mode(HIO_GPIO_P4, HIO_GPIO_MODE_INPUT);
            hio_gpio_set_mode(HIO_GPIO_P5, HIO_GPIO_MODE_INPUT);

            // Remember initial GPIO states
            _hio_module_encoder.samples |= hio_gpio_get_input(HIO_GPIO_P4) ? 0x1 : 0;
            _hio_module_encoder.samples |= hio_gpio_get_input(HIO_GPIO_P5) ? 0x2 : 0;

            // Register interrupts on both GPIO pins
            hio_exti_register(HIO_EXTI_LINE_P4, HIO_EXTI_EDGE_RISING_AND_FALLING, _hio_module_encoder_exti_handler, NULL);
            hio_exti_register(HIO_EXTI_LINE_P5, HIO_EXTI_EDGE_RISING_AND_FALLING, _hio_module_encoder_exti_handler, NULL);

            _hio_module_encoder.initialized = true;
        }
        else
        {
            if (_hio_module_encoder.event_handler != NULL)
            {
                _hio_module_encoder.event_handler(HIO_MODULE_ENCODER_EVENT_ERROR, _hio_module_encoder.event_param);
            }
        }
    }
}

static void _hio_module_encoder_exti_handler(hio_exti_line_t line, void *param)
{
    (void) line;
    (void) param;

    // Read current GPIO state
    _hio_module_encoder.samples <<= 2;
    _hio_module_encoder.samples |= hio_gpio_get_input(HIO_GPIO_P4) ? 0x1 : 0;
    _hio_module_encoder.samples |= hio_gpio_get_input(HIO_GPIO_P5) ? 0x2 : 0;
    _hio_module_encoder.samples &= 0xf;

    if (_hio_module_encoder.samples == 0x7)
    {
        _hio_module_encoder.increment--;

        hio_scheduler_plan_now(_hio_module_encoder.task_id);
    }
    else if (_hio_module_encoder.samples == 0xd)
    {
        _hio_module_encoder.increment++;

        hio_scheduler_plan_now(_hio_module_encoder.task_id);
    }
}

static void _hio_module_encoder_button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_hio_module_encoder.event_handler == NULL)
    {
        return;
    }

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        _hio_module_encoder.event_handler(HIO_MODULE_ENCODER_EVENT_PRESS, _hio_module_encoder.event_param);
    }
    else if (event == HIO_BUTTON_EVENT_RELEASE)
    {
        _hio_module_encoder.event_handler(HIO_MODULE_ENCODER_EVENT_RELEASE, _hio_module_encoder.event_param);
    }
    else if (event == HIO_BUTTON_EVENT_CLICK)
    {
        _hio_module_encoder.event_handler(HIO_MODULE_ENCODER_EVENT_CLICK, _hio_module_encoder.event_param);
    }
    else if (event == HIO_BUTTON_EVENT_HOLD)
    {
        _hio_module_encoder.event_handler(HIO_MODULE_ENCODER_EVENT_HOLD, _hio_module_encoder.event_param);
    }
}

static bool _hio_module_encoder_test_pin(hio_gpio_channel_t channel)
{
    hio_gpio_set_mode(channel, HIO_GPIO_MODE_OUTPUT);

    hio_gpio_set_output(channel, 1);

    hio_gpio_set_pull(channel, HIO_GPIO_PULL_DOWN);

    hio_gpio_set_mode(channel, HIO_GPIO_MODE_INPUT);

    hio_timer_start();

    hio_timer_delay(20);

    hio_timer_stop();

    int value = hio_gpio_get_input(channel);

    hio_gpio_set_mode(channel, HIO_GPIO_MODE_ANALOG);

    hio_gpio_set_pull(channel, HIO_GPIO_PULL_NONE);

    return value != 0;
}

static bool _hio_module_encoder_present_test(void)
{
    hio_system_pll_enable();

    hio_timer_init();

    hio_gpio_init(HIO_GPIO_P4);

    hio_gpio_init(HIO_GPIO_P5);

    bool is_present = _hio_module_encoder_test_pin(HIO_GPIO_P4) && _hio_module_encoder_test_pin(HIO_GPIO_P5);

    hio_system_pll_disable();

    return is_present;
}
