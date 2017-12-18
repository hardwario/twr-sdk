#include <bc_radio_node.h>

__attribute__((weak)) void bc_radio_node_on_state_set(uint64_t *id, uint8_t state_id, bool *state) { (void) id; (void) state_id; (void) state; }
__attribute__((weak)) void bc_radio_node_on_state_get(uint64_t *id, uint8_t state_id) { (void) id; (void) state_id; }
__attribute__((weak)) void bc_radio_node_on_buffer(uint64_t *id, void *buffer, size_t length) { (void) id; (void) buffer; (void) length; }
__attribute__((weak)) void bc_radio_node_on_led_strip_color_set(uint64_t *id, uint32_t *color) { (void) id; (void) color; }
__attribute__((weak)) void bc_radio_node_on_led_strip_brightness_set(uint64_t *id, uint8_t *brightness) { (void) id; (void) brightness; }
__attribute__((weak)) void bc_radio_node_on_led_strip_compound_set(uint64_t *id, uint8_t *compound, size_t length) { (void) id; (void) compound; (void) length; }
__attribute__((weak)) void bc_radio_node_on_led_strip_effect_set(uint64_t *id, bc_radio_node_led_strip_effect_t type, uint16_t wait, uint32_t *color) { (void) id; (void) type; (void) wait; (void) color; }
__attribute__((weak)) void bc_radio_node_on_led_strip_thermometer_set(uint64_t *id, float *temperature, int8_t *min, int8_t *max, uint8_t *white_dots, float *set_point, uint32_t *set_point_color) { (void) id; (void) temperature; (void) min; (void) max; (void) white_dots; (void) set_point; (void) set_point_color; }


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

bool bc_radio_node_led_strip_thermometer_set(uint64_t *id, float temperature, int8_t min, int8_t max, uint8_t white_dots, float *set_point, uint32_t set_point_color)
{
    uint8_t buffer[1 + BC_RADIO_ID_SIZE + sizeof(float) + sizeof(int8_t) + sizeof(int8_t) + sizeof(uint8_t) + sizeof(float) + sizeof(uint32_t)];

    buffer[0] = BC_RADIO_HEADER_NODE_LED_STRIP_THERMOMETER_SET;

    uint8_t *pbuffer = bc_radio_id_to_buffer(id, buffer + 1);

    pbuffer = bc_radio_float_to_buffer(&temperature, pbuffer);

    *pbuffer++ = min;

    *pbuffer++ = max;

    *pbuffer++ = white_dots;

    if ((set_point == NULL) || isnan(*set_point) || *set_point == BC_RADIO_NULL_FLOAT)
    {
        return bc_radio_pub_queue_put(buffer, sizeof(buffer) - sizeof(float) - sizeof(uint32_t));
    }

    pbuffer = bc_radio_float_to_buffer(set_point, pbuffer);

    pbuffer = bc_radio_data_to_buffer(&set_point_color, sizeof(uint32_t), pbuffer);

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

    uint8_t *pbuffer = buffer + 1 + BC_RADIO_ID_SIZE;
    length = length - 1 - BC_RADIO_ID_SIZE;

    if (buffer[0] == BC_RADIO_HEADER_NODE_STATE_SET)
    {
        bool state;
        bool *pstate;

        bc_radio_bool_from_buffer(pbuffer + 1, &state, &pstate);

        bc_radio_node_on_state_set(&for_id, pbuffer[0], pstate);
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
        bc_radio_node_led_strip_effect_t type = (bc_radio_node_led_strip_effect_t) *pbuffer++;

        uint16_t wait = (uint16_t) *pbuffer++;
        wait |= (uint16_t) *pbuffer++ >> 8;

        uint32_t color;

        bc_radio_data_from_buffer(pbuffer, &color, sizeof(color));

        bc_radio_node_on_led_strip_effect_set(id, type, wait, &color);
    }
    else if (buffer[0] == BC_RADIO_HEADER_NODE_LED_STRIP_THERMOMETER_SET)
    {
        float temperature;
        float *ptemperature;
        float set_point = 0;
        float *pset_point = NULL;
        uint32_t color = 0;

        pbuffer = bc_radio_float_from_buffer(pbuffer, &temperature, &ptemperature);
        int8_t *min = (int8_t *) pbuffer;
        int8_t *max = (int8_t *) pbuffer + 1;
        uint8_t *white_dots = (uint8_t *) pbuffer + 2;

        if (length == sizeof(float) + sizeof(int8_t) + sizeof(int8_t) + sizeof(uint8_t) + sizeof(float) + sizeof(uint32_t))
        {
            pbuffer = bc_radio_float_from_buffer(pbuffer + 3, &set_point, &pset_point);

            bc_radio_data_from_buffer(pbuffer, &color, sizeof(color));
        }

        bc_radio_node_on_led_strip_thermometer_set(id, ptemperature, min, max, white_dots, pset_point, &color);
    }
}
