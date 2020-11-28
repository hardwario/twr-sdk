#include <hio_module_relay.h>

static void _hio_module_relay_task(void *param);

#define HIO_MODULE_RELAY_POLARITY_F      ((1 << 6) | (1 << 7))
#define HIO_MODULE_RELAY_POLARITY_T      ((1 << 4) | (1 << 5))
#define HIO_MODULE_RELAY_POLARITY_NONE   ((1 << 6) | (1 << 4))


static bool _hio_module_relay_hardware_init(hio_module_relay_t *self)
{
    // Init i2C expander driver
    if (!hio_tca9534a_init(&self->_tca9534a, HIO_I2C_I2C0, self->_i2c_address))
    {
        return false;
    }
    // De-energize bistable relay coil - turn off
    hio_tca9534a_write_port(&(self->_tca9534a), HIO_MODULE_RELAY_POLARITY_NONE);
    // Enable outputs
    hio_tca9534a_set_port_direction(&self->_tca9534a, 0x00); // inverted: 0 = output
    // Relay is bi-stable, in the begining we don't know the default state
    self->_relay_state = HIO_MODULE_RELAY_STATE_UNKNOWN;

    return true;
}

bool hio_module_relay_init(hio_module_relay_t *self, uint8_t i2c_address)
{
    // Init instance, set state machine initial state
    memset(self, 0, sizeof(*self));
    self->_i2c_address = i2c_address;
    return _hio_module_relay_hardware_init(self);
}


static void hio_module_relay_scheduler_unregister(hio_module_relay_t *self)
{
    // Check if there is already a task running
    if (self->_task_is_active)
    {
        // Unregister running task
        hio_scheduler_unregister(self->_task_id);
        // Clear task id
        self->_task_id = 0;
        self->_task_is_active = false;
    }
}

static void hio_module_relay_scheduler_register(hio_module_relay_t *self)
{
    // Exit if there's already registered task
    if (self->_task_is_active)
    {
        return;
    }

    // Register relay task
    self->_task_id = hio_scheduler_register(_hio_module_relay_task, self, 0);
    self->_task_is_active = true;
}

static void _hio_module_relay_set_output(hio_module_relay_t *self, hio_module_relay_state_t state)
{
    if (state == HIO_MODULE_RELAY_STATE_TRUE)
    {
        if (!hio_tca9534a_write_port(&self->_tca9534a, HIO_MODULE_RELAY_POLARITY_T)) // pol A
        {
            self->_error = true;
        }
    }
    else
    {
        if (!hio_tca9534a_write_port(&self->_tca9534a, HIO_MODULE_RELAY_POLARITY_F)) // pol B
        {
            self->_error = true;
        }
    }
}

static void _hio_module_relay_set_output_disable(hio_module_relay_t *self)
{
    if (!hio_tca9534a_write_port(&(self->_tca9534a), HIO_MODULE_RELAY_POLARITY_NONE))
    {
        self->_error = true;
    }
}

static hio_tick_t hio_module_relay_state_machine(hio_module_relay_t *self, hio_tick_t tick_now)
{
    while (true)
    {
        switch (self->_state)
        {
            case HIO_MODULE_RELAY_TASK_STATE_IDLE:
                // Handle Error
                if (self->_error)
                {
                    // Try to initialize relay module again
                    if (_hio_module_relay_hardware_init(self))
                    {
                        self->_error = false;
                    }
                }

                // Handle commands
                if (self->_command == HIO_MODULE_RELAY_COMMAND_SET)
                {
                    self->_command = HIO_MODULE_RELAY_COMMAND_NONE;
                    self->_state = HIO_MODULE_RELAY_TASK_STATE_SET;
                    continue;
                }

                if (self->_command == HIO_MODULE_RELAY_COMMAND_PULSE)
                {
                    self->_command = HIO_MODULE_RELAY_COMMAND_NONE;
                    self->_state = HIO_MODULE_RELAY_TASK_STATE_PULSE;
                    continue;
                }

                // Unregister task if no command is needed
                hio_module_relay_scheduler_unregister(self);
                return tick_now;

            //
            // Relay set start state
            //
            case HIO_MODULE_RELAY_TASK_STATE_SET:
                // Set relay to the selected polarity
                _hio_module_relay_set_output(self, self->_desired_state);
                self->_relay_state = self->_desired_state;

                self->_state = HIO_MODULE_RELAY_TASK_STATE_SET_DEMAGNETIZE;
                return tick_now + 20;

            case HIO_MODULE_RELAY_TASK_STATE_SET_DEMAGNETIZE:
                // De-energize bistable relay coil - turn off
                _hio_module_relay_set_output_disable(self);

                self->_state = HIO_MODULE_RELAY_TASK_STATE_IDLE;
                // Needs 100ms to let the capacitor on relay board to charge
                return tick_now + 100;

            //
            // Relay pulse start state
            //
            case HIO_MODULE_RELAY_TASK_STATE_PULSE:
                // Create pulse of the set polarity
                _hio_module_relay_set_output(self, self->_desired_state);
                self->_relay_state = self->_desired_state;

                self->_state = HIO_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE;
                return tick_now + 20;

            case HIO_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE:
                // De-energize bistable relay coil - turn off
                _hio_module_relay_set_output_disable(self);

                self->_state = HIO_MODULE_RELAY_TASK_STATE_PULSE_REVERSE;
                return tick_now + self->_pulse_duration;

            case HIO_MODULE_RELAY_TASK_STATE_PULSE_REVERSE:
                // Change actual relay state to the oposite polarity
                self->_relay_state = (self->_relay_state == HIO_MODULE_RELAY_STATE_TRUE) ? HIO_MODULE_RELAY_STATE_FALSE : HIO_MODULE_RELAY_STATE_TRUE;
                _hio_module_relay_set_output(self, self->_relay_state);

                self->_state = HIO_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE_2;
                return tick_now + 20;

            case HIO_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE_2:
                // De-energize bistable relay coil - turn off
                _hio_module_relay_set_output_disable(self);

                self->_state = HIO_MODULE_RELAY_TASK_STATE_IDLE;
                // Needs 100ms to let the capacitor on relay board to charge
                return tick_now + 100;

            default:
                break;
        }
    }
}

static void _hio_module_relay_task(void *param)
{
    hio_module_relay_t *self = param;

    hio_scheduler_plan_current_absolute(hio_module_relay_state_machine(self, hio_scheduler_get_spin_tick()));
}

void hio_module_relay_set_state(hio_module_relay_t *self, bool state)
{
    // Save set command
    self->_command = HIO_MODULE_RELAY_COMMAND_SET;
    self->_desired_state = (state) ? HIO_MODULE_RELAY_STATE_TRUE : HIO_MODULE_RELAY_STATE_FALSE;

    hio_module_relay_scheduler_register(self);
}

void hio_module_relay_toggle(hio_module_relay_t *self)
{
    if (self->_relay_state == HIO_MODULE_RELAY_STATE_FALSE)
    {
        hio_module_relay_set_state(self, HIO_MODULE_RELAY_STATE_TRUE);
    }
    else if (self->_relay_state == HIO_MODULE_RELAY_STATE_TRUE)
    {
        hio_module_relay_set_state(self, HIO_MODULE_RELAY_STATE_FALSE);
    }
}

void hio_module_relay_pulse(hio_module_relay_t *self, bool state, hio_tick_t duration)
{
    // Save pulse duration
    self->_command = HIO_MODULE_RELAY_COMMAND_PULSE;
    self->_pulse_duration = duration;
    self->_desired_state = (state) ? HIO_MODULE_RELAY_STATE_TRUE : HIO_MODULE_RELAY_STATE_FALSE;

    hio_module_relay_scheduler_register(self);
}

hio_module_relay_state_t hio_module_relay_get_state(hio_module_relay_t *self)
{
    return self->_relay_state;
}
