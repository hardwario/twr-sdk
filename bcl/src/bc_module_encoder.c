#include <bc_module_encoder.h>
#include <bc_scheduler.h>
#include <bc_exti.h>


/*

example app.c code

#include <application.h>
#include <usb_talk.h>
#include <bc_module_encoder.h>

bc_module_encoder_t encoder;
uint32_t cnt = 1000;

static void encoder_event_handler(bc_module_encoder_t *self, bc_module_encoder_event_t event)
{

    if(event == BC_MODULE_ENCODER_EVENT_UP)
    {
        cnt++;
    }
    else if(event == BC_MODULE_ENCODER_EVENT_DOWN)
    {
        cnt--;
    }
    else if(event == BC_MODULE_ENCODER_EVENT_PRESS)
    {
        cnt++;
    }

    // Debug output
    usb_talk_publish_push_button("", &cnt);
}

	void application_init(void)
	{
		usb_talk_init();

        bc_module_encoder_init(&encoder);
        bc_module_encoder_set_event_handler(&encoder, encoder_event_handler, NULL);

	}


*/


#define _BC_MODULE_ENCODER_SCAN_INTERVAL 20
#define _BC_MODULE_ENCODER_DEBOUNCE_TIME 50
#define _BC_MODULE_ENCODER_CLICK_TIMEOUT 500
#define _BC_MODULE_ENCODER_HOLD_TIME 2000

static uint8_t _encoder_get_state();


static void _bc_module_power_exti_handler(bc_exti_line_t line, void *param);
static void _bc_button_event_handler(bc_button_t *self, bc_button_event_t event, void *param);

void bc_module_encoder_init(bc_module_encoder_t *self)
{
    memset(self, 0, sizeof(*self));

    bc_button_init(&self->_button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&self->_button, _bc_button_event_handler, self);

    //p4,p5
    bc_gpio_init(BC_GPIO_P4);
    bc_gpio_init(BC_GPIO_P5);

    bc_gpio_set_mode(BC_GPIO_P4, BC_GPIO_MODE_INPUT);
    bc_gpio_set_mode(BC_GPIO_P5, BC_GPIO_MODE_INPUT);

    self->_encoder_last_state = _encoder_get_state();
    self->value = 0;

    bc_exti_register(BC_EXTI_LINE_P4, BC_EXTI_EDGE_RISING_AND_FALLING, _bc_module_power_exti_handler, self);
    bc_exti_register(BC_EXTI_LINE_P5, BC_EXTI_EDGE_RISING_AND_FALLING, _bc_module_power_exti_handler, self);

    //bc_scheduler_register(_bc_module_encoder_task, self, 10);
}

void bc_module_encoder_set_event_handler(bc_module_encoder_t *self, void (*event_handler)(bc_module_encoder_t *, bc_module_encoder_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_module_encoder_set_scan_interval(bc_module_encoder_t *self, bc_tick_t scan_interval)
{
    //self->_scan_interval = scan_interval;
}

void bc_module_encoder_set_debounce_time(bc_module_encoder_t *self, bc_tick_t debounce_time)
{
    //self->_debounce_time = debounce_time;
}

void bc_module_encoder_set_click_timeout(bc_module_encoder_t *self, bc_tick_t click_timeout)
{
    //self->_click_timeout = click_timeout;
}

void bc_module_encoder_set_hold_time(bc_module_encoder_t *self, bc_tick_t hold_time)
{
    //self->_hold_time = hold_time;
}

static void _bc_module_power_exti_handler(bc_exti_line_t line, void *param)
{
    (void)line;

    bc_module_encoder_t *self = param;

    uint8_t encoder_current_state;

    // Protože mi EXTI nedá informaci kterým směrem se pin změnil, musím načíst přes GPIO aktuální stav
    encoder_current_state = _encoder_get_state();

    if(self->_encoder_last_state != encoder_current_state)
    {

        switch((self->_encoder_last_state << 2) | encoder_current_state)
        {
            // Only one case is used so the event is fired exactly in the middle of
            // two mechanical positions of encoder
            //case 0x01: // 0b00000001:
            case 0x07: // 0b00000111:
            //case 0x0E: // 0b00001110:
            //case 0x08: // 0b00001000:
                self->value--;
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_MODULE_ENCODER_EVENT_DOWN, self->_event_param);
                }
                break;


            //case 0x02: //0b00000010:
            //case 0x0B: //0b00001011:
            case 0x0D: //0b00001101:
            //case 0x04: //0b00000100:
                self->value++;
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_MODULE_ENCODER_EVENT_UP, self->_event_param);
                }
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

    // Mám tady pro každý event buttonu udělat zvlášt if?
    // Protože mi sedí enumy eventů u buttonu i encoder_module tak by šla hodnota teoreticky
    // přímo poslat dále, ale je to dobré řešení se spoléhat, že je nikdo nezmění? (není)
    if(event == BC_BUTTON_EVENT_PRESS)
    {
        if (encoder->_event_handler != NULL)
        {
            encoder->_event_handler(self, BC_MODULE_ENCODER_EVENT_PRESS, encoder->_event_param);
        }
    }
}

static uint8_t _encoder_get_state()
{
    uint8_t newState;

    newState = bc_gpio_get_input(BC_GPIO_P4) ? 0x01 : 0x00;
    newState |= bc_gpio_get_input(BC_GPIO_P5) ? 0x02 : 0x00;

    return newState;
}
