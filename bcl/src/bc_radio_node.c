#include <bc_radio_node.h>

__attribute__((weak)) void bc_radio_node_on_state_set(uint64_t *id, uint8_t state_id, bool *state) { (void) id; (void) state_id; (void) state; }
__attribute__((weak)) void bc_radio_node_on_state_get(uint64_t *id, uint8_t state_id) { (void) id; (void) state_id; }
__attribute__((weak)) void bc_radio_node_on_buffer(uint64_t *id, void *buffer, size_t length) { (void) id; (void) buffer; (void) length; }
__attribute__((weak)) void bc_radio_node_on_led_strip_color_set(uint64_t *id, uint32_t *color) { (void) id; (void) color; }
__attribute__((weak)) void bc_radio_node_on_led_strip_brightness_set(uint64_t *id, uint8_t *brightness) { (void) id; (void) brightness; }
__attribute__((weak)) void bc_radio_node_on_led_strip_compound_set(uint64_t *id, uint8_t *compound, size_t length) { (void) id; (void) compound; (void) length; }
__attribute__((weak)) void bc_radio_node_on_led_strip_effect_set(uint64_t *id, bc_radio_node_led_strip_effect_t type, uint16_t wait, uint32_t color) { (void) id; (void) type; (void) wait; (void) color; }

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

bool bc_radio_node_buffer(uint64_t *id, void *buffer, size_t length)
{
    uint8_t qbuffer[1 + BC_RADIO_ID_SIZE + BC_RADIO_NODE_MAX_BUFFER_SIZE];

    if (length > BC_RADIO_NODE_MAX_BUFFER_SIZE)
    {
        return false;
    }

    qbuffer[0] = BC_RADIO_HEADER_NODE_BUFFER;
    bc_radio_id_to_buffer(id, qbuffer + 1);

    memcpy(qbuffer + 1 + BC_RADIO_ID_SIZE, buffer, length);

    return bc_radio_pub_queue_put(qbuffer, length + 1 + BC_RADIO_ID_SIZE);
}

bool bc_radio_node_led_strip_color_set(uint64_t *id, uint32_t color)
{
    uint8_t buffer[1 + BC_RADIO_ID_SIZE + sizeof(color)];

    buffer[0] = BC_RADIO_HEADER_NODE_LED_STRIP_COLOR_SET;

    uint8_t *pbuffer = bc_radio_id_to_buffer(id, buffer + 1);

    bc_radio_data_to_buffer(&color, sizeof(color), pbuffer);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_node_led_strip_brightness_set(uint64_t *id, uint8_t brightness)
{
    uint8_t buffer[1 + BC_RADIO_ID_SIZE + sizeof(brightness)];

    buffer[0] = BC_RADIO_HEADER_NODE_LED_STRIP_BRIGHTNESS_SET;

    uint8_t *pbuffer = bc_radio_id_to_buffer(id, buffer + 1);

    pbuffer[0] = brightness;

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_node_led_strip_compound_set(uint64_t *id, uint8_t *compound, size_t length)
{
    if ((length > BC_RADIO_NODE_MAX_BUFFER_SIZE) || (length % 5 != 0))
    {
        return false;
    }

    uint8_t buffer[1 + BC_RADIO_ID_SIZE + BC_RADIO_NODE_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_NODE_LED_STRIP_COMPOUND_SET;

    uint8_t *pbuffer = bc_radio_id_to_buffer(id, buffer + 1);

    bc_radio_data_to_buffer(compound, length, pbuffer);

    return bc_radio_pub_queue_put(buffer, 1 + BC_RADIO_ID_SIZE + length);
}

bool bc_radio_node_led_strip_effect_set(uint64_t *id, bc_radio_node_led_strip_effect_t type, uint16_t wait, uint32_t color)
{
    uint8_t buffer[1 + BC_RADIO_ID_SIZE + 1 + sizeof(uint16_t) + sizeof(uint32_t)];

    buffer[0] = BC_RADIO_HEADER_NODE_LED_STRIP_EFFECT_SET;

    uint8_t *pbuffer = bc_radio_id_to_buffer(id, buffer + 1);

    *pbuffer++ = type;

    *pbuffer++ = (uint16_t) wait;
    *pbuffer++ = (uint16_t) wait >> 8;

    bc_radio_data_to_buffer(&color, sizeof(color), pbuffer);

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

    uint8_t *pbuffer = bc_radio_id_from_buffer(buffer + 1, &for_id);
    length = length - BC_RADIO_ID_SIZE - 1;

    if (for_id != bc_radio_get_my_id())
    {
        return;
    }

    if (buffer[0] == BC_RADIO_HEADER_NODE_STATE_SET)
    {
        bool *state = NULL;

        bc_radio_bool_from_buffer(pbuffer + 1, &state);

        bc_radio_node_on_state_set(&for_id, pbuffer[0], state);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_STATE_GET)
    {
        bc_radio_node_on_state_get(&for_id, pbuffer[0]);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_BUFFER)
    {
        bc_radio_node_on_buffer(id, pbuffer, length);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_LED_STRIP_COLOR_SET)
    {
        uint32_t color;

        bc_radio_data_from_buffer(pbuffer, &color, sizeof(color));

        bc_radio_node_on_led_strip_color_set(id, &color);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_LED_STRIP_BRIGHTNESS_SET)
    {
        bc_radio_node_on_led_strip_brightness_set(id, pbuffer);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_LED_STRIP_COMPOUND_SET)
    {
        bc_radio_node_on_led_strip_compound_set(id, pbuffer, length);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_LED_STRIP_EFFECT_SET)
    {
        bc_radio_node_led_strip_effect_t type;
        uint16_t wait = 0;
        uint32_t color;

        type = (bc_radio_node_led_strip_effect_t) *pbuffer++;

        wait = (uint16_t) *pbuffer++;
        wait |= (uint16_t) *pbuffer++ >> 8;

        bc_radio_data_from_buffer(pbuffer, &color, sizeof(color));

        bc_radio_node_on_led_strip_effect_set(id, type, wait, color);
    }
}
