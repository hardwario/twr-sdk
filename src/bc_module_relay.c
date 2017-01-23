#include <bc_module_power.h>
#include <bc_scheduler.h>
#include <bc_module_relay.h>
#include <bc_scheduler.h>

static bc_tick_t _bc_module_relay_task(void *param);

#define BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT 0x3B
#define BC_MODULE_RELAY_I2C_ADDRESS_ALTERNATE 0x3F

void bc_module_relay_init(bc_module_relay_t *self)
{
    // Init instance
    memset(self, 0, sizeof(*self));
    // Init i2C expander driver
    bc_tca9534a_init(&self->_tca9534a, BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    // Enable outputs
    bc_tca9534a_set_port_direction(&self->_tca9534a, 0x0F); // inverted: O = output
    // Relay is bi-stable, in the begining we don't know the default state
    self->_relay_state = BC_MODULE_RELAY_STATE_UNKNOWN;
}

static void bc_module_relay_scheduler_unregister(bc_module_relay_t *self)
{
    // Check if there is already a task running
    if(self->_task_id)
    {
        // Unregister running task
        bc_scheduler_unregister(self->_task_id);
        // Clear task id
        self->_task_id = 0;
    }
}

static void bc_module_relay_scheduler_register(bc_module_relay_t *self)
{
    // Try unregister if we have a already running task
    bc_module_relay_scheduler_unregister(self);

    // Register relay task
    self->_task_id = bc_scheduler_register(_bc_module_relay_task, self, bc_tick_get() + 20);

}

static bc_tick_t _bc_module_relay_task(void *param)
{
    bc_module_relay_t *self = param;

    switch(self->_state)
    {
        // Final state turning off the coil
        case BC_MODULE_RELAY_TASK_STATE_DEMAGNETIZE:
            // De-energize bistable relay coil - turn off
            bc_tca9534a_write_port(&(self->_tca9534a), (1 << 6) | (1 << 4));
            // Unregister task
            bc_module_relay_scheduler_unregister(self);
            return 0;
            break;

        // Pulse mode - demagnetize the coil and set state to toggle the coil to other direction
        case BC_MODULE_RELAY_TASK_STATE_DEMAGNETIZE_PULSE:
            // De-energize bistable relay coil - turn off
            bc_tca9534a_write_port(&(self->_tca9534a), (1 << 6) | (1 << 4));
            // Set next task state
            self->_state = BC_MODULE_RELAY_TASK_STATE_REVERSE_PULSE;
            // Delay for set period of the pulse
            return self->_pulse_length;
            break;

        // Pulse mode - toggle the relay to another direction
        case BC_MODULE_RELAY_TASK_STATE_REVERSE_PULSE:
            bc_module_relay_toggle(self);
            return 20;
            break;

        default:
            break;
    }

    return 0;
}





void bc_module_relay_set(bc_module_relay_t *self, bc_module_relay_state_t state)
{
    if(state)
    {
        bc_tca9534a_write_port(&self->_tca9534a, (1 << 6) | (1 << 7)); // Polarity A
        self->_relay_state = BC_MODULE_RELAY_STATE_TRUE;
    } else {
        bc_tca9534a_write_port(&self->_tca9534a, (1 << 4) | (1 << 5)); // Polarity B
        self->_relay_state = BC_MODULE_RELAY_STATE_FALSE;
    }

    self->_state = BC_MODULE_RELAY_TASK_STATE_DEMAGNETIZE;
    bc_module_relay_scheduler_register(self);
}



bool bc_module_relay_toggle(bc_module_relay_t *self)
{

    if(self->_relay_state == BC_MODULE_RELAY_STATE_FALSE)
    {
        bc_module_relay_set(self, BC_MODULE_RELAY_STATE_TRUE);
        return true;
    }
    else if(self->_relay_state == BC_MODULE_RELAY_STATE_TRUE)
    {
        bc_module_relay_set(self, BC_MODULE_RELAY_STATE_FALSE);
        return true;
    }

    // In case of BC_MODULE_RELAY_STATE_UNKNOWN
    return false;
}


void bc_module_relay_pulse(bc_module_relay_t *self, bc_tick_t duration)
{
    // Save pulse duration
    self->_pulse_length = duration;

    // Set relay and schedule task
    bc_module_relay_set(self, BC_MODULE_RELAY_STATE_TRUE);
    // Manually change the state of the task to PULSE
    self->_state = BC_MODULE_RELAY_TASK_STATE_DEMAGNETIZE_PULSE;

}
