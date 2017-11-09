#include <bc_radio_pub.h>

__attribute__((weak)) void bc_radio_pub_on_event_count(uint64_t *id, uint8_t event_id, uint16_t *event_count) { (void) id; (void) event_id; (void) event_count; }
__attribute__((weak)) void bc_radio_pub_on_push_button(uint64_t *id, uint16_t *event_count) { (void) id; (void) event_count; }
__attribute__((weak)) void bc_radio_pub_on_temperature(uint64_t *id, uint8_t *channel, float *celsius) { (void) id; (void) channel; (void) celsius; }
__attribute__((weak)) void bc_radio_pub_on_humidity(uint64_t *id, uint8_t *channel, float *percentage) { (void) id; (void) channel; (void) percentage; }
__attribute__((weak)) void bc_radio_pub_on_lux_meter(uint64_t *id, uint8_t *channel, float *illuminance) { (void) id; (void) channel; (void) illuminance; }
__attribute__((weak)) void bc_radio_pub_on_barometer(uint64_t *id, uint8_t *channel, float *pressure, float *altitude) { (void) id; (void) channel; (void) pressure; (void) altitude; }
__attribute__((weak)) void bc_radio_pub_on_co2(uint64_t *id, float *concentration) { (void) id; (void) concentration; }
__attribute__((weak)) void bc_radio_pub_on_battery(uint64_t *id, float *voltage) { (void) id; (void) voltage; }
__attribute__((weak)) void bc_radio_pub_on_buffer(uint64_t *id, void *buffer, size_t length) { (void) id; (void) buffer; (void) length; }
__attribute__((weak)) void bc_radio_pub_on_state(uint64_t *id, uint8_t state_id, bool *state) { (void) id; (void) state_id; (void) state; }
__attribute__((weak)) void bc_radio_pub_on_bool(uint64_t *id, char *subtopic, bool *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_pub_on_int(uint64_t *id, char *subtopic, int *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_pub_on_float(uint64_t *id, char *subtopic, float *value) { (void) id; (void) subtopic; (void) value; }

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

bool bc_radio_pub_temperature(uint8_t channel, float *celsius)
{
    uint8_t buffer[2 + sizeof(*celsius)];

    buffer[0] = BC_RADIO_HEADER_PUB_TEMPERATURE;
    buffer[1] = channel;

    memcpy(&buffer[2], celsius, sizeof(*celsius));

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

void bc_radio_pub_decode(uint64_t *id, uint8_t *buffer, size_t length)
{

    if (buffer[0] == BC_RADIO_HEADER_PUB_EVENT_COUNT)
    {
        uint16_t event_count;

        memcpy(&event_count, buffer + 2, sizeof(event_count));

        if (buffer[1] == BC_RADIO_PUB_EVENT_PUSH_BUTTON)
        {
            bc_radio_pub_on_push_button(id, &event_count);
        }

        bc_radio_pub_on_event_count(id, buffer[1], &event_count);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TEMPERATURE)
    {
        float celsius;

        memcpy(&celsius, buffer + 2, sizeof(celsius));

        bc_radio_pub_on_temperature(id, buffer + 1, &celsius);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_HUMIDITY)
    {
        float percentage;

        memcpy(&percentage, buffer + 2, sizeof(percentage));

        bc_radio_pub_on_humidity(id, buffer + 1, &percentage);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_LUX_METER)
    {
        float lux;

        memcpy(&lux, buffer + 2, sizeof(lux));

        bc_radio_pub_on_lux_meter(id, buffer + 1, &lux);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_BAROMETER)
    {
        float pascal;
        float meter;

        memcpy(&pascal, buffer + 2, sizeof(pascal));
        memcpy(&meter, buffer + 2 + sizeof(pascal), sizeof(meter));

        bc_radio_pub_on_barometer(id, buffer + 1, &pascal, &meter);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_CO2)
    {
        float concentration;

        memcpy(&concentration, buffer + 1, sizeof(concentration));

        bc_radio_pub_on_co2(id, &concentration);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_BATTERY)
    {
        float voltage;

        memcpy(&voltage, buffer + 1, sizeof(voltage));

        bc_radio_pub_on_battery(id, &voltage);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_BUFFER)
    {
        bc_radio_pub_on_buffer(id, buffer + 1, length - 1);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_STATE)
    {
        bool *state = NULL;

        bc_radio_bool_from_buffer(buffer + 2, &state);

        bc_radio_pub_on_state(id, buffer[1], state);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_BOOL)
    {
        bool *value = NULL;

        buffer = bc_radio_bool_from_buffer(buffer + 1, &value);

        buffer[length - 1] = 0;

        bc_radio_pub_on_bool(id, (char *) buffer, value);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_INT)
    {
        int *value = NULL;

        buffer = bc_radio_int_from_buffer(buffer + 1, &value);

        buffer[length - 1] = 0;

        bc_radio_pub_on_int(id, (char *) buffer, value);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_FLOAT)
    {
        float *value = NULL;

        buffer = bc_radio_float_from_buffer(buffer + 1, &value);

        buffer[length - 1] = 0;

        bc_radio_pub_on_float(id, (char *) buffer, value);
    }
}
