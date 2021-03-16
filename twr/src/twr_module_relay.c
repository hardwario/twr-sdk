#include <twr_module_relay.h>

static void _twr_module_relay_task(void *param);

#define TWR_MODULE_RELAY_POLARITY_F      ((1 << 6) | (1 << 7))
#define TWR_MODULE_RELAY_POLARITY_T      ((1 << 4) | (1 << 5))
#define TWR_MODULE_RELAY_POLARITY_NONE   ((1 << 6) | (1 << 4))


static bool _twr_module_relay_hardware_init(twr_module_relay_t *self)
{
    // Relay is bi-stable, in the begining we don't know the default state
    self->_relay_state = TWR_MODULE_RELAY_STATE_UNKNOWN;

    // Init i2C expander driver
    if (!twr_tca9534a_init(&self->_tca9534a, TWR_I2C_I2C0, self->_i2c_address))
    {
        return false;
    }
    // De-energize bistable relay coil - turn off
    twr_tca9534a_write_port(&(self->_tca9534a), TWR_MODULE_RELAY_POLARITY_NONE);
    // Enable outputs
    twr_tca9534a_set_port_direction(&self->_tca9534a, 0x00); // inverted: 0 = output

    return true;
}

bool twr_module_relay_init(twr_module_relay_t *self, uint8_t i2c_address)
{
    // Init instance, set state machine initial state
    memset(self, 0, sizeof(*self));
    self->_i2c_address = i2c_address;
    return _twr_module_relay_hardware_init(self);
}

static void twr_module_relay_scheduler_unregister(twr_module_relay_t *self)
{
    // Check if there is already a task running
    if (self->_task_is_active)
    {
        // Unregister running task
        twr_scheduler_unregister(self->_task_id);
        // Clear task id
        self->_task_id = 0;
        self->_task_is_active = false;
    }
}

static void twr_module_relay_scheduler_register(twr_module_relay_t *self)
{
    // Exit if there's already registered task
    if (self->_task_is_active)
    {
        return;
    }

    // Register relay task
    self->_task_id = twr_scheduler_register(_twr_module_relay_task, self, 0);
    self->_task_is_active = true;
}

static void _twr_module_relay_set_output(twr_module_relay_t *self, twr_module_relay_state_t state)
{
    if (state == TWR_MODULE_RELAY_STATE_TRUE)
    {
        if (!twr_tca9534a_write_port(&self->_tca9534a, TWR_MODULE_RELAY_POLARITY_T)) // pol A
        {
            self->_error = true;
        }
    }
    else
    {
        if (!twr_tca9534a_write_port(&self->_tca9534a, TWR_MODULE_RELAY_POLARITY_F)) // pol B
        {
            self->_error = true;
        }
    }
}

static void _twr_module_relay_set_output_disable(twr_module_relay_t *self)
{
    if (!twr_tca9534a_write_port(&(self->_tca9534a), TWR_MODULE_RELAY_POLARITY_NONE))
    {
        self->_error = true;
    }
}

static twr_tick_t twr_module_relay_state_machine(twr_module_relay_t *self, twr_tick_t tick_now)
{
    while (true)
    {
        switch (self->_state)
        {
            case TWR_MODULE_RELAY_TASK_STATE_IDLE:
                // Handle Error
                if (self->_error)
                {
                    // Try to initialize relay module again
                    if (_twr_module_relay_hardware_init(self))
                    {
                        self->_error = false;
                    }
                }

                // Handle commands
                if (self->_command == TWR_MODULE_RELAY_COMMAND_SET)
                {
                    self->_command = TWR_MODULE_RELAY_COMMAND_NONE;
                    self->_state = TWR_MODULE_RELAY_TASK_STATE_SET;
                    continue;
                }

                if (self->_command == TWR_MODULE_RELAY_COMMAND_PULSE)
                {
                    self->_command = TWR_MODULE_RELAY_COMMAND_NONE;
                    self->_state = TWR_MODULE_RELAY_TASK_STATE_PULSE;
                    continue;
                }

                // Unregister task if no command is needed
                twr_module_relay_scheduler_unregister(self);
                return tick_now;

            //
            // Relay set start state
            //
            case TWR_MODULE_RELAY_TASK_STATE_SET:
                // Set relay to the selected polarity
                _twr_module_relay_set_output(self, self->_desired_state);
                self->_relay_state = self->_desired_state;

                self->_state = TWR_MODULE_RELAY_TASK_STATE_SET_DEMAGNETIZE;
                return tick_now + 20;

            case TWR_MODULE_RELAY_TASK_STATE_SET_DEMAGNETIZE:
                // De-energize bistable relay coil - turn off
                _twr_module_relay_set_output_disable(self);

                self->_state = TWR_MODULE_RELAY_TASK_STATE_IDLE;
                // Needs 100ms to let the capacitor on relay board to charge
                return tick_now + 100;

            //
            // Relay pulse start state
            //
            case TWR_MODULE_RELAY_TASK_STATE_PULSE:
                // Create pulse of the set polarity
                _twr_module_relay_set_output(self, self->_desired_state);
                self->_relay_state = self->_desired_state;

                self->_state = TWR_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE;
                return tick_now + 20;

            case TWR_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE:
                // De-energize bistable relay coil - turn off
                _twr_module_relay_set_output_disable(self);

                self->_state = TWR_MODULE_RELAY_TASK_STATE_PULSE_REVERSE;
                return tick_now + self->_pulse_duration;

            case TWR_MODULE_RELAY_TASK_STATE_PULSE_REVERSE:
                // Change actual relay state to the oposite polarity
                self->_relay_state = (self->_relay_state == TWR_MODULE_RELAY_STATE_TRUE) ? TWR_MODULE_RELAY_STATE_FALSE : TWR_MODULE_RELAY_STATE_TRUE;
                _twr_module_relay_set_output(self, self->_relay_state);

                self->_state = TWR_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE_2;
                return tick_now + 20;

            case TWR_MODULE_RELAY_TASK_STATE_PULSE_DEMAGNETIZE_2:
                // De-energize bistable relay coil - turn off
                _twr_module_relay_set_output_disable(self);

                self->_state = TWR_MODULE_RELAY_TASK_STATE_IDLE;
                // Needs 100ms to let the capacitor on relay board to charge
                return tick_now + 100;

            default:
                break;
        }
    }
}

static void _twr_module_relay_task(void *param)
{
    twr_module_relay_t *self = param;

    twr_scheduler_plan_current_absolute(twr_module_relay_state_machine(self, twr_scheduler_get_spin_tick()));
}

void twr_module_relay_set_state(twr_module_relay_t *self, bool state)
{
    // Save set command
    self->_command = TWR_MODULE_RELAY_COMMAND_SET;
    self->_desired_state = (state) ? TWR_MODULE_RELAY_STATE_TRUE : TWR_MODULE_RELAY_STATE_FALSE;

    twr_module_relay_scheduler_register(self);
}

void twr_module_relay_toggle(twr_module_relay_t *self)
{
    if (self->_relay_state == TWR_MODULE_RELAY_STATE_FALSE)
    {
        twr_module_relay_set_state(self, TWR_MODULE_RELAY_STATE_TRUE);
    }
    else if (self->_relay_state == TWR_MODULE_RELAY_STATE_TRUE)
    {
        twr_module_relay_set_state(self, TWR_MODULE_RELAY_STATE_FALSE);
    }
}

void twr_module_relay_pulse(twr_module_relay_t *self, bool state, twr_tick_t duration)
{
    // Save pulse duration
    self->_command = TWR_MODULE_RELAY_COMMAND_PULSE;
    self->_pulse_duration = duration;
    self->_desired_state = (state) ? TWR_MODULE_RELAY_STATE_TRUE : TWR_MODULE_RELAY_STATE_FALSE;

    twr_module_relay_scheduler_register(self);
}

twr_module_relay_state_t twr_module_relay_get_state(twr_module_relay_t *self)
{
    return self->_relay_state;
}
