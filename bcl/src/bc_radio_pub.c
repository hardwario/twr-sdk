#include <bc_radio_pub.h>

#define _BC_RADIO_PUB_BUFFER_SIZE_ACCELERATION (1 + sizeof(float) + sizeof(float) + sizeof(float))

__attribute__((weak)) void bc_radio_pub_on_event_count(uint64_t *id, uint8_t event_id, uint16_t *event_count) { (void) id; (void) event_id; (void) event_count; }
__attribute__((weak)) void bc_radio_pub_on_push_button(uint64_t *id, uint16_t *event_count) { (void) id; (void) event_count; }
__attribute__((weak)) void bc_radio_pub_on_temperature(uint64_t *id, uint8_t channel, float *celsius) { (void) id; (void) channel; (void) celsius; }
__attribute__((weak)) void bc_radio_pub_on_humidity(uint64_t *id, uint8_t channel, float *percentage) { (void) id; (void) channel; (void) percentage; }
__attribute__((weak)) void bc_radio_pub_on_lux_meter(uint64_t *id, uint8_t channel, float *illuminance) { (void) id; (void) channel; (void) illuminance; }
__attribute__((weak)) void bc_radio_pub_on_barometer(uint64_t *id, uint8_t channel, float *pressure, float *altitude) { (void) id; (void) channel; (void) pressure; (void) altitude; }
__attribute__((weak)) void bc_radio_pub_on_co2(uint64_t *id, float *concentration) { (void) id; (void) concentration; }
__attribute__((weak)) void bc_radio_pub_on_battery(uint64_t *id, float *voltage) { (void) id; (void) voltage; }
__attribute__((weak)) void bc_radio_pub_on_acceleration(uint64_t *id, float *x_axis, float *y_axis, float *z_axis) { (void) id; (void) x_axis; (void) y_axis; (void) z_axis; }
__attribute__((weak)) void bc_radio_pub_on_buffer(uint64_t *id, void *buffer, size_t length) { (void) id; (void) buffer; (void) length; }
__attribute__((weak)) void bc_radio_pub_on_state(uint64_t *id, uint8_t state_id, bool *state) { (void) id; (void) state_id; (void) state; }
__attribute__((weak)) void bc_radio_pub_on_bool(uint64_t *id, char *subtopic, bool *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_pub_on_int(uint64_t *id, char *subtopic, int *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_pub_on_uint32(uint64_t *id, char *subtopic, uint32_t *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_pub_on_float(uint64_t *id, char *subtopic, float *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_pub_on_string(uint64_t *id, char *subtopic, char *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_pub_on_value_int(uint64_t *id, uint8_t value_id, int *value) { (void) id; (void) value_id; (void) value; }


bool bc_radio_pub_event_count(uint8_t event_id, uint16_t *event_count)
{
    uint8_t buffer[1 + sizeof(event_id) + sizeof(*event_count)];

    buffer[0] = BC_RADIO_HEADER_PUB_EVENT_COUNT;
    buffer[1] = event_id;

    bc_radio_uint16_to_buffer(event_count, buffer + 2);

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

    bc_radio_float_to_buffer(celsius, buffer + 2);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_humidity(uint8_t channel, float *percentage)
{
    uint8_t buffer[2 + sizeof(*percentage)];

    buffer[0] = BC_RADIO_HEADER_PUB_HUMIDITY;
    buffer[1] = channel;

    bc_radio_float_to_buffer(percentage, buffer + 2);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_luminosity(uint8_t channel, float *lux)
{
    uint8_t buffer[2 + sizeof(*lux)];

    buffer[0] = BC_RADIO_HEADER_PUB_LUX_METER;
    buffer[1] = channel;

    bc_radio_float_to_buffer(lux, buffer + 2);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_barometer(uint8_t channel, float *pascal, float *meter)
{
    uint8_t buffer[2 + sizeof(*pascal) + sizeof(*meter)];

    buffer[0] = BC_RADIO_HEADER_PUB_BAROMETER;
    buffer[1] = channel;

    uint8_t *pointer = bc_radio_float_to_buffer(pascal, buffer + 2);
    bc_radio_float_to_buffer(meter, pointer);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_co2(float *concentration)
{
    uint8_t buffer[1 + sizeof(*concentration)];

    buffer[0] = BC_RADIO_HEADER_PUB_CO2;

    bc_radio_float_to_buffer(concentration, buffer + 1);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_battery(float *voltage)
{
    uint8_t buffer[1 + sizeof(*voltage)];

    buffer[0] = BC_RADIO_HEADER_PUB_BATTERY;

    bc_radio_float_to_buffer(voltage, buffer + 1);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_acceleration(float *x_axis, float *y_axis, float *z_axis)
{
    uint8_t buffer[_BC_RADIO_PUB_BUFFER_SIZE_ACCELERATION];

    buffer[0] = BC_RADIO_HEADER_PUB_ACCELERATION;

    uint8_t *pointer = bc_radio_float_to_buffer(x_axis, buffer + 1);

    pointer = bc_radio_float_to_buffer(y_axis, pointer);

    pointer = bc_radio_float_to_buffer(z_axis, pointer);

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

bool bc_radio_pub_value_int(uint8_t value_id, int *value)
{
    uint8_t buffer[1 + sizeof(uint8_t) + sizeof(int)];

    buffer[0] = BC_RADIO_HEADER_PUB_VALUE_INT;
    buffer[1] = value_id;

    bc_radio_int_to_buffer(value, buffer + 2);

    return bc_radio_pub_queue_put(buffer, sizeof(buffer));
}

bool bc_radio_pub_bool(const char *subtopic, bool *value)
{
    size_t len = strlen(subtopic);

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
    size_t len = strlen(subtopic);

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

bool bc_radio_pub_uint32(const char *subtopic, uint32_t *value)
{
    size_t len = strlen(subtopic);

    if (len > BC_RADIO_MAX_TOPIC_LEN)
    {
        return false;
    }

    uint8_t buffer[BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_UINT32;

    bc_radio_uint32_to_buffer(value, buffer + 1);

    strcpy((char *)buffer + 5, subtopic);

    return bc_radio_pub_queue_put(buffer, len + 6);
}

bool bc_radio_pub_float(const char *subtopic, float *value)
{
    size_t len = strlen(subtopic);

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

bool bc_radio_pub_string(const char *subtopic, const char *value)
{
    size_t len = strlen(subtopic);
    size_t len_value = strlen(value);

    if (len + len_value > (BC_RADIO_MAX_BUFFER_SIZE - 3))
    {
        return false;
    }

    uint8_t buffer[BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_STRING;

    strcpy((char *)buffer + 1, subtopic);
    strcpy((char *)buffer + 1 + len + 1, value);

    return bc_radio_pub_queue_put(buffer, len + len_value + 3);
}

void bc_radio_pub_decode(uint64_t *id, uint8_t *buffer, size_t length)
{

    if (buffer[0] == BC_RADIO_HEADER_PUB_PUSH_BUTTON)
    {
        uint16_t event_count;
        uint16_t *pevent_count;

        bc_radio_uint16_from_buffer(buffer + 1, &event_count, &pevent_count);

        bc_radio_pub_on_push_button(id, &event_count);

        bc_radio_pub_on_event_count(id, BC_RADIO_PUB_EVENT_PUSH_BUTTON, pevent_count);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_EVENT_COUNT)
    {
        uint16_t event_count;
        uint16_t *pevent_count;

        if (length != (1 + sizeof(uint8_t) + sizeof(uint16_t)))
        {
            return;
        }

        bc_radio_uint16_from_buffer(buffer + 2, &event_count, &pevent_count);

        if (buffer[1] == BC_RADIO_PUB_EVENT_PUSH_BUTTON)
        {
            bc_radio_pub_on_push_button(id, pevent_count);
        }

        bc_radio_pub_on_event_count(id, buffer[1], pevent_count);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TEMPERATURE)
    {
        float celsius;
        float *pcelsius;

        if (length != (1 + sizeof(uint8_t) + sizeof(float)))
        {
            return;
        }

        bc_radio_float_from_buffer(buffer + 2, &celsius, &pcelsius);

        bc_radio_pub_on_temperature(id, buffer[1], pcelsius);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_HUMIDITY)
    {
        float percentage;
        float *ppercentage;

        if (length != (1 + sizeof(uint8_t) + sizeof(float)))
        {
            return;
        }

        bc_radio_float_from_buffer(buffer + 2, &percentage, &ppercentage);

        bc_radio_pub_on_humidity(id, buffer[1], ppercentage);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_LUX_METER)
    {
        float lux;
        float *plux;

        if (length != (1 + sizeof(uint8_t) + sizeof(float)))
        {
            return;
        }

        bc_radio_float_from_buffer(buffer + 2, &lux, &plux);

        bc_radio_pub_on_lux_meter(id, buffer[1], plux);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_BAROMETER)
    {
        float pascal;
        float *ppascal;
        float meter;
        float *pmeter;

        if (length != (1 + sizeof(uint8_t) + sizeof(float) + sizeof(float)))
        {
            return;
        }

        uint8_t *pointer = bc_radio_float_from_buffer(buffer + 2, &pascal, &ppascal);

        bc_radio_float_from_buffer(pointer, &meter, &pmeter);

        bc_radio_pub_on_barometer(id, buffer[1], ppascal, pmeter);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_CO2)
    {
        float concentration;
        float *pconcentration;

        if (length != (1 + sizeof(float)))
        {
            return;
        }

        bc_radio_float_from_buffer(buffer + 1, &concentration, &pconcentration);

        bc_radio_pub_on_co2(id, pconcentration);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_BATTERY)
    {
        float voltage;
        float *pvoltage;

        if (length == (1 + sizeof(float)))
        {
            bc_radio_float_from_buffer(buffer + 1, &voltage, &pvoltage);
        }
        else if (length == (1 + 1 + sizeof(float)))
        {
            // Old format
            bc_radio_float_from_buffer(buffer + 2, &voltage, &pvoltage);
        }
        else
        {
            return;
        }

        bc_radio_pub_on_battery(id, pvoltage);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_ACCELERATION)
    {
        if (length != _BC_RADIO_PUB_BUFFER_SIZE_ACCELERATION)
        {
            return;
        }

        float x_axis;
        float *px_axis;
        float y_axis;
        float *py_axis;
        float z_axis;
        float *pz_axis;

        buffer = bc_radio_float_from_buffer(buffer + 1, &x_axis, &px_axis);

        buffer = bc_radio_float_from_buffer(buffer, &y_axis, &py_axis);

        bc_radio_float_from_buffer(buffer, &z_axis, &pz_axis);

        bc_radio_pub_on_acceleration(id, px_axis, py_axis, pz_axis);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_BUFFER)
    {
        bc_radio_pub_on_buffer(id, buffer + 1, length - 1);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_STATE)
    {
        bool state;
        bool *pstate = NULL;

        if (length != (1 + sizeof(uint8_t) + sizeof(bool)))
        {
            return;
        }

        bc_radio_bool_from_buffer(buffer + 2, &state, &pstate);

        bc_radio_pub_on_state(id, buffer[1], pstate);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_BOOL)
    {
        bool value;
        bool *pvalue;

        buffer = bc_radio_bool_from_buffer(buffer + 1, &value, &pvalue);

        buffer[length - 1] = 0;

        bc_radio_pub_on_bool(id, (char *) buffer, pvalue);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_INT)
    {
        int value;
        int *pvalue;

        buffer = bc_radio_int_from_buffer(buffer + 1, &value, &pvalue);

        buffer[length - 1] = 0;

        bc_radio_pub_on_int(id, (char *) buffer, pvalue);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_UINT32)
    {
        uint32_t value;
        uint32_t *pvalue;

        buffer = bc_radio_uint32_from_buffer(buffer + 1, &value, &pvalue);

        buffer[length - 1] = 0;

        bc_radio_pub_on_uint32(id, (char *) buffer, pvalue);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_FLOAT)
    {
        float value;
        float *pvalue;

        buffer = bc_radio_float_from_buffer(buffer + 1, &value, &pvalue);

        buffer[length - 1] = 0;

        bc_radio_pub_on_float(id, (char *) buffer, pvalue);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_TOPIC_STRING)
    {
        buffer[length - 1] = 0;

        size_t len = strlen((char *) buffer + 1);

        bc_radio_pub_on_string(id, (char *) buffer + 1, (char *) buffer + 2 + len);
    }
    else if (buffer[0] == BC_RADIO_HEADER_PUB_VALUE_INT)
    {
        int value;
        int *pvalue;

        if (length != (1 + sizeof(uint8_t) + sizeof(int)))
        {
            return;
        }

        bc_radio_int_from_buffer(buffer + 2, &value, &pvalue);

        bc_radio_pub_on_value_int(id, buffer[1], pvalue);
    }
}
