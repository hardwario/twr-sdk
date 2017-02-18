#include <bc_module_encoder.h>
#include <bc_scheduler.h>
#include <bc_exti.h>
#include <bc_irq.h>

static struct
{
    bc_scheduler_task_id_t task_id;
    void (*event_handler)(bc_module_encoder_event_t, void *);
    void *event_param;
    bc_button_t button;
    int increment;
    int increment_shadow;
    int samples;

} _bc_module_encoder;

static void _bc_module_encoder_task(void *param);

static void _bc_module_encoder_exti_handler(bc_exti_line_t line, void *param);

static void _bc_module_encoder_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

void bc_module_encoder_init(void)
{
    memset(&_bc_module_encoder, 0, sizeof(_bc_module_encoder));

    // Initialize encoder button
    bc_button_init(&_bc_module_encoder.button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&_bc_module_encoder.button, _bc_module_encoder_button_event_handler, NULL);

    // Init encoder GPIO pins
    bc_gpio_init(BC_GPIO_P4);
    bc_gpio_init(BC_GPIO_P5);

    // Set encoder GPIO pins as inputs
    bc_gpio_set_mode(BC_GPIO_P4, BC_GPIO_MODE_INPUT);
    bc_gpio_set_mode(BC_GPIO_P5, BC_GPIO_MODE_INPUT);

    // Remember initial GPIO states
    _bc_module_encoder.samples |= bc_gpio_get_input(BC_GPIO_P4) ? 0x1 : 0;
    _bc_module_encoder.samples |= bc_gpio_get_input(BC_GPIO_P5) ? 0x2 : 0;

    // Register interrupts on both GPIO pins
    bc_exti_register(BC_EXTI_LINE_P4, BC_EXTI_EDGE_RISING_AND_FALLING, _bc_module_encoder_exti_handler, NULL);
    bc_exti_register(BC_EXTI_LINE_P5, BC_EXTI_EDGE_RISING_AND_FALLING, _bc_module_encoder_exti_handler, NULL);

    // Register task
    _bc_module_encoder.task_id = bc_scheduler_register(_bc_module_encoder_task, NULL, BC_TICK_INFINITY);
}

void bc_module_encoder_set_event_handler(void (*event_handler)(bc_module_encoder_event_t, void *), void *event_param)
{
    _bc_module_encoder.event_handler = event_handler;
    _bc_module_encoder.event_param = event_param;
}

bc_button_t *bc_module_encoder_get_button_instance(void)
{
    return &_bc_module_encoder.button;
}

int bc_module_encoder_get_increment(void)
{
    return _bc_module_encoder.increment_shadow;
}

static void _bc_module_encoder_task(void *param)
{
    (void) param;

    // Disable interrupts
    bc_irq_disable();

    _bc_module_encoder.increment_shadow = _bc_module_encoder.increment;
    _bc_module_encoder.increment = 0;

    // Enable interrupts
    bc_irq_enable();

    if (_bc_module_encoder.increment_shadow != 0)
    {
        if (_bc_module_encoder.event_handler != NULL)
        {
            _bc_module_encoder.event_handler(BC_MODULE_ENCODER_EVENT_ROTATION, _bc_module_encoder.event_param);
        }
    }
}

static void _bc_module_encoder_exti_handler(bc_exti_line_t line, void *param)
{
    (void) line;
    (void) param;

    // Read current GPIO state
    _bc_module_encoder.samples <<= 2;
    _bc_module_encoder.samples |= bc_gpio_get_input(BC_GPIO_P4) ? 0x1 : 0;
    _bc_module_encoder.samples |= bc_gpio_get_input(BC_GPIO_P5) ? 0x2 : 0;
    _bc_module_encoder.samples &= 0xf;

    if (_bc_module_encoder.samples == 0x7)
    {
        _bc_module_encoder.increment--;

        bc_scheduler_plan_now(_bc_module_encoder.task_id);
    }
    else if (_bc_module_encoder.samples == 0xd)
    {
        _bc_module_encoder.increment++;

        bc_scheduler_plan_now(_bc_module_encoder.task_id);
    }
}

static void _bc_module_encoder_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_bc_module_encoder.event_handler == NULL)
    {
        return;
    }

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        _bc_module_encoder.event_handler(BC_MODULE_ENCODER_EVENT_PRESS, _bc_module_encoder.event_param);
    }
    else if (event == BC_BUTTON_EVENT_RELEASE)
    {
        _bc_module_encoder.event_handler(BC_MODULE_ENCODER_EVENT_RELEASE, _bc_module_encoder.event_param);
    }
    else if (event == BC_BUTTON_EVENT_CLICK)
    {
        _bc_module_encoder.event_handler(BC_MODULE_ENCODER_EVENT_CLICK, _bc_module_encoder.event_param);
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        _bc_module_encoder.event_handler(BC_MODULE_ENCODER_EVENT_HOLD, _bc_module_encoder.event_param);
    }
}
