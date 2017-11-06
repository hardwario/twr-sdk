#include <bc_radio_pub.h>

bool bc_radio_pub_event_count(uint8_t event_id, uint16_t *event_count)
{
    uint8_t buffer[1 + sizeof(event_id) + sizeof(*event_count)];

    buffer[0] = BC_RADIO_HEADER_PUB_EVENT_COUNT;
    buffer[1] = event_id;

    memcpy(buffer + 2, event_count, sizeof(*event_count));

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_push_button(uint16_t *event_count)
{
    return bc_radio_pub_event_count(BC_RADIO_PUB_EVENT_PUSH_BUTTON, event_count);
}

bool bc_radio_pub_thermometer(uint8_t channel, float *temperature)
{
    uint8_t buffer[2 + sizeof(*temperature)];

    buffer[0] = BC_RADIO_HEADER_PUB_THERMOMETER;
    buffer[1] = channel;

    memcpy(&buffer[2], temperature, sizeof(*temperature));

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_humidity(uint8_t channel, float *percentage)
{
    uint8_t buffer[2 + sizeof(*percentage)];

    buffer[0] = BC_RADIO_HEADER_PUB_HUMIDITY;
    buffer[1] = channel;

    memcpy(&buffer[2], percentage, sizeof(*percentage));

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_luminosity(uint8_t channel, float *lux)
{
    uint8_t buffer[2 + sizeof(*lux)];

    buffer[0] = BC_RADIO_HEADER_PUB_LUX_METER;
    buffer[1] = channel;

    memcpy(&buffer[2], lux, sizeof(*lux));

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_barometer(uint8_t channel, float *pascal, float *meter)
{
    uint8_t buffer[2 + sizeof(*pascal) + sizeof(*meter)];

    buffer[0] = BC_RADIO_HEADER_PUB_BAROMETER;
    buffer[1] = channel;

    memcpy(&buffer[2], pascal, sizeof(*pascal));
    memcpy(&buffer[2 + sizeof(*pascal)], meter, sizeof(*meter));

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_co2(float *concentration)
{
    uint8_t buffer[1 + sizeof(*concentration)];

    buffer[0] = BC_RADIO_HEADER_PUB_CO2;

    memcpy(&buffer[1], concentration, sizeof(*concentration));

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_battery(float *voltage)
{
    uint8_t buffer[1 + sizeof(uint8_t) + sizeof(*voltage)];

    buffer[0] = BC_RADIO_HEADER_PUB_BATTERY;

    memcpy(&buffer[1], voltage, sizeof(*voltage));

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_buffer(void *buffer, size_t length)
{
    uint8_t qbuffer[BC_RADIO_MAX_BUFFER_SIZE];

    if (length > sizeof(qbuffer) - 1)
    {
        return false;
    }

    qbuffer[0] = BC_RADIO_HEADER_PUB_BUFFER;

    memcpy(&qbuffer[1], buffer, length);

    return bc_radio_pub_queue_put(qbuffer, length + 1);
}

bool bc_radio_pub_state(uint8_t state_id, bool *state)
{
    uint8_t buffer[1 + sizeof(state_id) + sizeof(*state)];

    buffer[0] = BC_RADIO_HEADER_PUB_STATE;
    buffer[1] = state_id;

    bc_radio_bool_to_buffer(state, buffer + 2);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

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

bool bc_radio_pub_bool(const char *subtopic, bool *value)
{
    uint8_t len = strlen(subtopic);

    if (len > BC_RADIO_MAX_TOPIC_LEN)
    {
        return false;
    }

    uint8_t buffer[BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_BOOL;

    bc_radio_bool_to_buffer(value, buffer + 1);

    strcpy((char *)buffer + 2, subtopic);

    return bc_radio_pub_queue_put(buffer, len + 3);
}

bool bc_radio_pub_int(const char *subtopic, int *value)
{
    uint8_t len = strlen(subtopic);

    if (len > BC_RADIO_MAX_TOPIC_LEN)
    {
        return false;
    }

    uint8_t buffer[BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_INT;

    bc_radio_int_to_buffer(value, buffer + 1);

    strcpy((char *)buffer + 5, subtopic);

    return bc_radio_pub_queue_put(buffer, len + 6);
}

bool bc_radio_pub_float(const char *subtopic, float *value)
{
    uint8_t len = strlen(subtopic);

    if (len > BC_RADIO_MAX_TOPIC_LEN)
    {
        return false;
    }

    uint8_t buffer[BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_FLOAT;

    bc_radio_float_to_buffer(value, buffer + 1);

    strcpy((char *)buffer + 5, subtopic);

    return bc_radio_pub_queue_put(buffer, len + 6);
}
