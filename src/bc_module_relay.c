#include <bc_module_power.h>
//#include "stm32l0xx.h"

//#include "usb_talk.h"
#include <bc_scheduler.h>
#include <bc_module_relay.h>
#include <bc_scheduler.h>

static bc_tick_t _bc_module_relay_task(void *param);

// nepoužité výstupy do nuly
// Init - měl by init přepnout relé do výchozího stavu??
void bc_module_relay_init(bc_module_relay_t *self)
{
    bc_tca9534a_init(&self->_tca9534a, BC_I2C_I2C0, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    bc_tca9534a_set_port_direction(&self->_tca9534a, 0x0F); // inverted: O = output

    self->_timestamp = 0;

    bc_scheduler_register(_bc_module_relay_task, self, 10);
}

static bc_tick_t _bc_module_relay_task(void *param)
{
    bc_module_relay_t *self = param;

    // ms de-energize
    if(self->_timestamp == 1)
    {
        bc_tca9534a_write_port(&(self->_tca9534a), (1 << 6) | (1 << 4)); // off
    }

    if(self->_timestamp > 0)
        self->_timestamp--;

    return 10;
}

// Tady to ještě pořešíme, protože na PCB je NC, NO ale vzpomínám si že jsme se bavili že došlo k nějaké lepší změně.
void bc_module_relay_set(bc_module_relay_t *self, bc_module_relay_state_t state)
{
    if(state)
    {
        bc_tca9534a_write_port(&self->_tca9534a, (1 << 6) | (1 << 7)); // pol A
        self->_relay_state = BC_MODULE_RELAY_STATE_TRUE;
        self->_timestamp = 2;
    } else {
        bc_tca9534a_write_port(&self->_tca9534a, (1 << 4) | (1 << 5)); // pol B
        self->_relay_state = BC_MODULE_RELAY_STATE_FALSE;
        self->_timestamp = 2;
    }
}

void bc_module_relay_toggle(bc_module_relay_t *self)
{
    if(self->_relay_state == BC_MODULE_RELAY_STATE_FALSE)
    {
        bc_module_relay_set(self, BC_MODULE_RELAY_STATE_TRUE);
    }
    else if(self->_relay_state == BC_MODULE_RELAY_STATE_TRUE)
    {
        bc_module_relay_set(self, BC_MODULE_RELAY_STATE_FALSE);
    }
}

//! Make a pulse
void bc_module_relay_pulse(bc_module_relay_t *self, bc_tick_t duration)
{

}
