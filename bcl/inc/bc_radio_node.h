#ifndef _BC_RADIO_NODE_H
#define _BC_RADIO_NODE_H

#include <bc_radio.h>
#include <bc_radio_pub.h>

enum
{
    BC_RADIO_NODE_STATE_LED = BC_RADIO_PUB_STATE_LED,
    BC_RADIO_NODE_STATE_RELAY_MODULE_0 = BC_RADIO_PUB_STATE_RELAY_MODULE_0,
    BC_RADIO_NODE_STATE_RELAY_MODULE_1 = BC_RADIO_PUB_STATE_RELAY_MODULE_1,
    BC_RADIO_NODE_STATE_POWER_MODULE_RELAY = BC_RADIO_PUB_STATE_POWER_MODULE_RELAY,
};

bool bc_radio_node_state_set(uint64_t *id, uint8_t state_id, bool *state);

bool bc_radio_node_state_get(uint64_t *id, uint8_t state_id);

void bc_radio_node_decode(uint64_t *id, uint8_t *buffer, size_t length);

#endif // _BC_RADIO_NODE_H
