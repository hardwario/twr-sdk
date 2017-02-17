#include <bc_module_encoder.h>
#include <bc_scheduler.h>
#include <bc_exti.h>
#include <bc_irq.h>

static uint8_t _encoder_get_state();
static void _bc_module_encoder_task(void *param);

bc_module_encoder_t _bc_module_encoder;
static bc_module_encoder_t *self = &_bc_module_encoder;

static void _bc_module_power_exti_handler(bc_exti_line_t line, void *param);
static void _bc_button_event_handler(bc_button_t *self, bc_button_event_t event, void *param);

void bc_module_encoder_init()
{
    memset(self, 0, sizeof(*self));

    bc_button_init(&self->_button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&self->_button, _bc_button_event_handler, self);

    // Init encoder GPIO
    bc_gpio_init(BC_GPIO_P4);
    bc_gpio_init(BC_GPIO_P5);

    // Set encoder GPIO as inputs
    bc_gpio_set_mode(BC_GPIO_P4, BC_GPIO_MODE_INPUT);
    bc_gpio_set_mode(BC_GPIO_P5, BC_GPIO_MODE_INPUT);

    // Read current initial state
    self->_encoder_last_state = _encoder_get_state();

    // Register interrupts on both GPIO pins
    bc_exti_register(BC_EXTI_LINE_P4, BC_EXTI_EDGE_RISING_AND_FALLING, _bc_module_power_exti_handler, self);
    bc_exti_register(BC_EXTI_LINE_P5, BC_EXTI_EDGE_RISING_AND_FALLING, _bc_module_power_exti_handler, self);

    // Register task
    self->task_id = bc_scheduler_register(_bc_module_encoder_task, self, BC_TICK_INFINITY);
}

void bc_module_encoder_set_event_handler(void (*event_handler)(bc_module_encoder_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

static void _bc_module_power_exti_handler(bc_exti_line_t line, void *param)
{
    (void)line;

    bc_module_encoder_t *self = param;

    // Read current sttate from GPIO
    uint8_t encoder_current_state = _encoder_get_state();

    if(self->_encoder_last_state != encoder_current_state)
    {

        switch((self->_encoder_last_state << 2) | encoder_current_state)
        {
            case 0x07:
                self->value--;
                bc_scheduler_plan_now(self->task_id);
                break;

            case 0x0D:
                self->value++;
                bc_scheduler_plan_now(self->task_id);
                break;

            default:
                break;
        }

        // Save current state
        self->_encoder_last_state = encoder_current_state;

    }
}


static void _bc_button_event_handler(bc_button_t *self, bc_button_event_t event, void *param)
{
    // Přejmenovat bc_button_t parametr na btnSelf, nebo přetypovat param na encoder?
    (void)self;
    bc_module_encoder_t *encoder = param;

    // Pass the buttons event directly because they use the same enum event numbers
    encoder->_event_handler(event, encoder->_event_param);

}

static uint8_t _encoder_get_state()
{
    uint8_t newState;

    newState = bc_gpio_get_input(BC_GPIO_P4) ? 0x01 : 0x00;
    newState |= bc_gpio_get_input(BC_GPIO_P5) ? 0x02 : 0x00;

    return newState;
}

static void _bc_module_encoder_task(void *param)
{
    bc_module_encoder_t *self = param;
    uint32_t i;

    if(self->value == 0)
    {
        // In case user does quick forwward-back encoder motion
        return;
    }

    bc_irq_disable();

    self->eventValue = self->value;
    self->value = 0;

    bc_irq_enable();

    // In case multiple events per task call
    for(i = 0; i < (uint32_t)abs(self->eventValue); i++)
    {
        if (self->_event_handler != NULL)
        {
            self->_event_handler((self->eventValue > 0) ? BC_MODULE_ENCODER_EVENT_UP : BC_MODULE_ENCODER_EVENT_DOWN, self->_event_param);
        }
    }
}

int32_t bc_module_encoder_get_increment(void)
{
    return self->eventValue;
}
