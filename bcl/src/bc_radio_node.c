#include <bc_radio_node.h>

__attribute__((weak)) void bc_radio_node_on_state_set(uint64_t *id, uint8_t state_id, bool *state) { (void) id; (void) state_id; (void) state; }
__attribute__((weak)) void bc_radio_node_on_state_get(uint64_t *id, uint8_t state_id) { (void) id; (void) state_id; }

bool bc_radio_node_state_set(uint64_t *id, uint8_t state_id, bool *state)
{
    uint8_t buffer[1 + BC_RADIO_ID_SIZE + sizeof(state_id) + sizeof(*state)];

    buffer[0] = BC_RADIO_HEADER_NODE_STATE_SET;

    uint8_t *pbuffer = bc_radio_id_to_buffer(id, buffer + 1);

    pbuffer[0] = state_id;

    bc_radio_bool_to_buffer(state, pbuffer + 1);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_node_state_get(uint64_t *id, uint8_t state_id)
{
    uint8_t buffer[1 + BC_RADIO_ID_SIZE + sizeof(state_id)];

    buffer[0] = BC_RADIO_HEADER_NODE_STATE_GET;

    uint8_t *pbuffer = bc_radio_id_to_buffer(id, buffer + 1);

    pbuffer[0] = state_id;

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

void bc_radio_node_decode(uint64_t *id, uint8_t *buffer, size_t length)
{
    (void) id;

    uint64_t for_id;

    if (length < BC_RADIO_ID_SIZE + 1)
    {
        return;
    }

    buffer = bc_radio_id_from_buffer(buffer + 1, &for_id);

    if (for_id != bc_radio_get_my_id())
    {
        return;
    }

    if (buffer[0] == BC_RADIO_HEADER_NODE_STATE_SET)
    {

        bool *state = NULL;

        bc_radio_bool_from_buffer(buffer + 1, &state);

        bc_radio_node_on_state_set(&for_id, buffer[0], state);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_STATE_GET)
    {
        bc_radio_node_on_state_get(&for_id, buffer[0]);
    }
}
