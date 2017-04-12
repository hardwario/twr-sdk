#include <bc_radio.h>
#include <bc_queue.h>
#include <bc_device_id.h>
#include <bc_scheduler.h>
#include <bc_spirit1.h>
#include <bc_eeprom.h>

#define BC_RADIO_EEPROM_PEER_DEVICE_ADDRESS 0x00


typedef enum
{
    BC_RADIO_HEADER_ENROLL,
    BC_RADIO_HEADER_PUB_PUSH_BUTTON,
    BC_RADIO_HEADER_PUB_THERMOMETER,
    BC_RADIO_HEADER_PUB_HUMIDITY,
    BC_RADIO_HEADER_PUB_LUX_METER,
    BC_RADIO_HEADER_PUB_BAROMETER,
    BC_RADIO_HEADER_PUB_CO2,
    BC_RADIO_HEADER_PUB_BUFFER

} bc_radio_header_t;

typedef enum
{
    BC_RADIO_STATE_SLEEP = 0,
    BC_RADIO_STATE_TX = 1,
    BC_RADIO_STATE_RX = 2

} bc_radio_state_t;

static struct
{
    bc_radio_state_t state;
    uint32_t device_address;
    uint16_t message_id;
    int transmit_count;
    void (*event_handler)(bc_radio_event_t, void *);
    void *event_param;
    bc_scheduler_task_id_t task_id;
    bool enroll_to_gateway;
    bool enrollment_mode;

    bc_queue_t pub_queue;
    bc_queue_t rx_queue;
    uint8_t pub_queue_buffer[128];
    uint8_t rx_queue_buffer[128];

    bool peer_enrolled;
    uint32_t peer_device_address;
    uint16_t peer_message_id;
    bool peer_message_id_synced;

    bool listening;

} _bc_radio;

static void _bc_radio_task(void *param);
static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param);

__attribute__((weak)) void bc_radio_on_push_button(uint32_t *peer_device_address, uint16_t *event_count) { (void) peer_device_address; (void) event_count; }
__attribute__((weak)) void bc_radio_on_thermometer(uint32_t *peer_device_address, uint8_t *i2c, float *temperature) { (void) peer_device_address; (void) i2c; (void) temperature; }
__attribute__((weak)) void bc_radio_on_humidity(uint32_t *peer_device_address, uint8_t *i2c, float *percentage) { (void) peer_device_address; (void) i2c; (void) percentage; }
__attribute__((weak)) void bc_radio_on_lux_meter(uint32_t *peer_device_address, uint8_t *i2c, float *illuminance) { (void) peer_device_address; (void) i2c; (void) illuminance; }
__attribute__((weak)) void bc_radio_on_barometer(uint32_t *peer_device_address, uint8_t *i2c, float *pressure, float *altitude) { (void) peer_device_address; (void) i2c; (void) pressure; (void) altitude; }
__attribute__((weak)) void bc_radio_on_co2(uint32_t *peer_device_address, float *concentration) { (void) peer_device_address; (void) concentration; }
__attribute__((weak)) void bc_radio_on_buffer(uint32_t *peer_device_address, void *buffer, size_t *length) { (void) peer_device_address; (void) buffer; (void) length; }


void bc_radio_init(void)
{
    memset(&_bc_radio, 0, sizeof(_bc_radio));

    bc_queue_init(&_bc_radio.pub_queue, _bc_radio.pub_queue_buffer, sizeof(_bc_radio.pub_queue_buffer));
    bc_queue_init(&_bc_radio.rx_queue, _bc_radio.rx_queue_buffer, sizeof(_bc_radio.rx_queue_buffer));

    bc_device_id_get(&_bc_radio.device_address, sizeof(_bc_radio.device_address));

    bc_spirit1_init();
    bc_spirit1_set_event_handler(_bc_radio_spirit1_event_handler, NULL);

    bc_eeprom_read(BC_RADIO_EEPROM_PEER_DEVICE_ADDRESS, &_bc_radio.peer_device_address, sizeof(_bc_radio.peer_device_address));

    _bc_radio.task_id = bc_scheduler_register(_bc_radio_task, NULL, BC_TICK_INFINITY);
}

void bc_radio_set_event_handler(void (*event_handler)(bc_radio_event_t, void *), void *event_param)
{
    _bc_radio.event_handler = event_handler;
    _bc_radio.event_param = event_param;
}

void bc_radio_listen(void)
{
    _bc_radio.listening = true;

    bc_scheduler_plan_now(_bc_radio.task_id);
}

void bc_radio_sleep(void)
{
    _bc_radio.listening = false;

    bc_scheduler_plan_now(_bc_radio.task_id);
}

void bc_radio_enroll_to_gateway(void)
{
    _bc_radio.enroll_to_gateway = true;

    bc_scheduler_plan_now(_bc_radio.task_id);
}

void bc_radio_enrollment_start(void)
{
    _bc_radio.enrollment_mode = true;
}

void bc_radio_enrollment_stop(void)
{
    _bc_radio.enrollment_mode = false;
}

bool bc_radio_pub_push_button(uint16_t *event_count)
{
    uint8_t buffer[1 + sizeof(*event_count)];

    buffer[0] = BC_RADIO_HEADER_PUB_PUSH_BUTTON;

    memcpy(&buffer[1], event_count, sizeof(*event_count));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_thermometer(uint8_t i2c, float *temperature)
{
    uint8_t buffer[2 + sizeof(*temperature)];

    buffer[0] = BC_RADIO_HEADER_PUB_THERMOMETER;
    buffer[1] = i2c;

    memcpy(&buffer[2], temperature, sizeof(*temperature));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_humidity(uint8_t i2c, float *percentage)
{
    uint8_t buffer[2 + sizeof(*percentage)];

    buffer[0] = BC_RADIO_HEADER_PUB_HUMIDITY;
    buffer[1] = i2c;

    memcpy(&buffer[2], percentage, sizeof(*percentage));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_luminosity(uint8_t i2c, float *lux)
{
    uint8_t buffer[2 + sizeof(*lux)];

    buffer[0] = BC_RADIO_HEADER_PUB_LUX_METER;
    buffer[1] = i2c;

    memcpy(&buffer[2], lux, sizeof(*lux));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_barometer(uint8_t i2c, float *pascal, float *meter)
{
    uint8_t buffer[2 + sizeof(*pascal) + sizeof(*meter)];

    buffer[0] = BC_RADIO_HEADER_PUB_BAROMETER;
    buffer[1] = i2c;

    memcpy(&buffer[2], pascal, sizeof(*pascal));
    memcpy(&buffer[2 + sizeof(*pascal)], meter, sizeof(*meter));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_co2(float *concentration)
{
    uint8_t buffer[1 + sizeof(*concentration)];

    buffer[0] = BC_RADIO_HEADER_PUB_CO2;

    memcpy(&buffer[1], concentration, sizeof(*concentration));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_buffer(void *buffer, size_t length)
{
    uint8_t qbuffer[1 + BC_SPIRIT1_MAX_PACKET_SIZE - 6];

    if (length > (1 + BC_SPIRIT1_MAX_PACKET_SIZE - 6))
    {
        return false;
    }

    qbuffer[0] = BC_RADIO_HEADER_PUB_BUFFER;

    memcpy(&qbuffer[1], buffer, length);

    if (!bc_queue_put(&_bc_radio.pub_queue, qbuffer, length + 1))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;

}

static void _bc_radio_task(void *param)
{
    (void) param;

    if (_bc_radio.state == BC_RADIO_STATE_TX)
    {
        if (_bc_radio.transmit_count != 0)
        {
            bc_spirit1_tx();

            return;
        }

        _bc_radio.state = BC_RADIO_STATE_SLEEP;
    }

    if (_bc_radio.enroll_to_gateway)
    {
        _bc_radio.enroll_to_gateway = false;

        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        buffer[0] = _bc_radio.device_address;
        buffer[1] = _bc_radio.device_address >> 8;
        buffer[2] = _bc_radio.device_address >> 16;
        buffer[3] = _bc_radio.device_address >> 24;
        buffer[4] = _bc_radio.message_id;
        buffer[5] = _bc_radio.message_id >> 8;
        buffer[6] = BC_RADIO_HEADER_ENROLL;

        _bc_radio.message_id++;

        bc_spirit1_set_tx_length(7);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 10;

        _bc_radio.state = BC_RADIO_STATE_TX;
    }

    uint8_t queue_item_buffer[sizeof(_bc_radio.pub_queue_buffer)];
    size_t queue_item_length;

    while (bc_queue_get(&_bc_radio.rx_queue, queue_item_buffer, &queue_item_length))
    {
        if (queue_item_buffer[0] == BC_RADIO_HEADER_PUB_PUSH_BUTTON)
        {
            uint16_t event_count;

            memcpy(&event_count, &queue_item_buffer[1], sizeof(event_count));

            bc_radio_on_push_button(&_bc_radio.peer_device_address, &event_count);
        }
        else if (queue_item_buffer[0] == BC_RADIO_HEADER_PUB_THERMOMETER)
        {
            float temperature;

            memcpy(&temperature, &queue_item_buffer[2], sizeof(temperature));

            bc_radio_on_thermometer(&_bc_radio.peer_device_address, &queue_item_buffer[1], &temperature);
        }
        else if (queue_item_buffer[0] == BC_RADIO_HEADER_PUB_HUMIDITY)
        {
            float percentage;

            memcpy(&percentage, &queue_item_buffer[2], sizeof(percentage));

            bc_radio_on_humidity(&_bc_radio.peer_device_address, &queue_item_buffer[1], &percentage);
        }
        else if (queue_item_buffer[0] == BC_RADIO_HEADER_PUB_LUX_METER)
        {
            float lux;

            memcpy(&lux, &queue_item_buffer[2], sizeof(lux));

            bc_radio_on_lux_meter(&_bc_radio.peer_device_address, &queue_item_buffer[1], &lux);
        }
        else if (queue_item_buffer[0] == BC_RADIO_HEADER_PUB_BAROMETER)
        {
            float pascal;
            float meter;

            memcpy(&pascal, &queue_item_buffer[2], sizeof(pascal));
            memcpy(&meter, &queue_item_buffer[2 + sizeof(pascal)], sizeof(meter));

            bc_radio_on_barometer(&_bc_radio.peer_device_address, &queue_item_buffer[1], &pascal, &meter);
        }
        else if (queue_item_buffer[0] == BC_RADIO_HEADER_PUB_CO2)
        {
            float concentration;

            memcpy(&concentration, &queue_item_buffer[1], sizeof(concentration));

            bc_radio_on_co2(&_bc_radio.peer_device_address, &concentration);
        }
        else if (queue_item_buffer[0] == BC_RADIO_HEADER_PUB_BUFFER)
        {
            queue_item_length -= 1;
            bc_radio_on_buffer(&_bc_radio.peer_device_address, &queue_item_buffer[1], &queue_item_length);
        }

    }

    if (bc_queue_get(&_bc_radio.pub_queue, queue_item_buffer, &queue_item_length))
    {
        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        buffer[0] = _bc_radio.device_address;
        buffer[1] = _bc_radio.device_address >> 8;
        buffer[2] = _bc_radio.device_address >> 16;
        buffer[3] = _bc_radio.device_address >> 24;
        buffer[4] = _bc_radio.message_id;
        buffer[5] = _bc_radio.message_id >> 8;

        _bc_radio.message_id++;

        memcpy(buffer + 6, queue_item_buffer, queue_item_length);

        bc_spirit1_set_tx_length(6 + queue_item_length);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 10;

        _bc_radio.state = BC_RADIO_STATE_TX;
    }

    if (_bc_radio.listening && _bc_radio.transmit_count == 0)
    {
        bc_spirit1_set_rx_timeout(BC_TICK_INFINITY);
        bc_spirit1_rx();
    }
}

static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_SPIRIT1_EVENT_TX_DONE)
    {
        if (_bc_radio.transmit_count == 0)
        {
            bc_scheduler_plan_now(_bc_radio.task_id);
        }
        else
        {
            _bc_radio.transmit_count--;

            // TODO Use different randomizer
            bc_scheduler_plan_relative(_bc_radio.task_id, rand() % 100);
        }

        if (_bc_radio.listening)
        {
            bc_spirit1_set_rx_timeout(BC_TICK_INFINITY);
            bc_spirit1_rx();
        }
    }
    else if (event == BC_SPIRIT1_EVENT_RX_DONE)
    {
        size_t length = bc_spirit1_get_rx_length();

        if (length >= 6)
        {
            uint8_t *buffer = bc_spirit1_get_rx_buffer();

            uint32_t device_address;

            device_address = (uint32_t) buffer[0];
            device_address |= (uint32_t) buffer[1] << 8;
            device_address |= (uint32_t) buffer[2] << 16;
            device_address |= (uint32_t) buffer[3] << 24;

            if (_bc_radio.enrollment_mode && length == 7 && buffer[6] == BC_RADIO_HEADER_ENROLL)
            {
                _bc_radio.peer_enrolled = true;
                _bc_radio.peer_device_address = device_address;
                _bc_radio.peer_message_id_synced = false;
                _bc_radio.enrollment_mode = false;

                bc_eeprom_write(BC_RADIO_EEPROM_PEER_DEVICE_ADDRESS, &_bc_radio.peer_device_address, sizeof(_bc_radio.peer_device_address));

                if (_bc_radio.event_handler != NULL)
                {
                    _bc_radio.event_handler(BC_RADIO_EVENT_PAIR_SUCCESS, _bc_radio.event_param);
                }
            }

            if (device_address == _bc_radio.peer_device_address)
            {
                uint16_t message_id;

                message_id = (uint16_t) buffer[4];
                message_id |= (uint16_t) buffer[5] << 8;

                if (_bc_radio.peer_message_id != message_id || !_bc_radio.peer_message_id_synced)
                {
                    _bc_radio.peer_message_id = message_id;

                    _bc_radio.peer_message_id_synced = true;

                    if (length > 6)
                    {
                        bc_queue_put(&_bc_radio.rx_queue, buffer + 6, length - 6);

                        bc_scheduler_plan_now(_bc_radio.task_id);
                    }
                }
            }
        }
    }
}
