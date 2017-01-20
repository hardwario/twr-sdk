#ifndef _BC_MODULE_RELAY_H
#define _BC_MODULE_RELAY_H

#include <bc_common.h>

//! @addtogroup bc_module_relay bc_module_relay
//! @brief Driver for Relay Module
//! @section example How to use Relay Module
//! @code
//! TODO: example
//! @endcode
//! @{

#include <bc_tca9534a.h>

#define BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT 0x3B
#define BC_MODULE_RELAY_I2C_ADDRESS_ALTERNATE 0x3F

typedef enum
{
    BC_MODULE_RELAY_STATE_FALSE = 0x00,
    BC_MODULE_RELAY_STATE_TRUE = 0x01,
    BC_MODULE_RELAY_STATE_UNKNOWN = 0x02
} bc_module_relay_state_t;

//typedef struct bc_module_relay_t bc_module_relay_t;

typedef struct bc_module_relay_t
{
    bc_tca9534a_t _tca9534a;
    bc_module_relay_state_t _relay_state;
    bc_tick_t _timestamp;
} bc_module_relay_t;


// Init - měl by init přepnout relé do výchozího stavu??
void bc_module_relay_init(bc_module_relay_t *self);

// Tady to ještě pořešíme, protože na PCB je NC, NO ale vzpomínám si že jsme se bavili že došlo k nějaké lepší změně.
void bc_module_relay_set(bc_module_relay_t *self, bc_module_relay_state_t state);

//! Make a pulse
void bc_module_relay_pulse(bc_module_relay_t *self, bc_tick_t duration);

void bc_module_relay_toggle(bc_module_relay_t *self);

//! @}

#endif /* _BC_MODULE_RELAY_H */
