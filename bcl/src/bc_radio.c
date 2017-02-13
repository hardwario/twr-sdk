#include <bc_radio.h>
#include <bc_device_id.h>
#include <bc_scheduler.h>
#include <bc_spirit1.h>

typedef enum
{
    BC_RADIO_HEADER_ENROLL,
    BC_RADIO_HEADER_PUB_PUSH_BUTTON,
    BC_RADIO_HEADER_PUB_THERMOMETER

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
    uint8_t pub_queue_buffer[64];
    size_t pub_queue_length;

} _bc_radio;

static void _bc_radio_task(void *param);
static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param);

void bc_radio_init(void)
{
    memset(&_bc_radio, 0, sizeof(_bc_radio));

    bc_device_id_get(&_bc_radio.device_address, sizeof(_bc_radio.device_address));

    bc_spirit1_init();
    bc_spirit1_set_event_handler(_bc_radio_spirit1_event_handler, NULL);

    _bc_radio.task_id = bc_scheduler_register(_bc_radio_task, NULL, BC_TICK_INFINITY);
}

void bc_radio_set_event_handler(void (*event_handler)(bc_radio_event_t, void *), void *event_param)
{
    _bc_radio.event_handler = event_handler;
    _bc_radio.event_param = event_param;
}

void bc_radio_enroll_to_gateway(void)
{
    _bc_radio.enroll_to_gateway = true;

    bc_scheduler_plan_now(_bc_radio.task_id);
}

void bc_radio_pub_push_button(uint16_t *event_count)
{
    if ((sizeof(_bc_radio.pub_queue_buffer) - _bc_radio.pub_queue_length) < 1 + sizeof(*event_count))
    {
        return;
    }

    _bc_radio.pub_queue_buffer[_bc_radio.pub_queue_length++] = BC_RADIO_HEADER_PUB_PUSH_BUTTON;

    memcpy(&_bc_radio.pub_queue_buffer[_bc_radio.pub_queue_length], event_count, sizeof(*event_count));

    _bc_radio.pub_queue_length += sizeof(*event_count);

    bc_scheduler_plan_now(_bc_radio.task_id);
}

void bc_radio_pub_thermometer(float *temperature)
{
    if ((sizeof(_bc_radio.pub_queue_buffer) - _bc_radio.pub_queue_length) < 1 + sizeof(*temperature))
    {
        return;
    }

    _bc_radio.pub_queue_buffer[_bc_radio.pub_queue_length++] = BC_RADIO_HEADER_PUB_THERMOMETER;

    memcpy(&_bc_radio.pub_queue_buffer[_bc_radio.pub_queue_length], temperature, sizeof(*temperature));

    _bc_radio.pub_queue_length += sizeof(*temperature);

    bc_scheduler_plan_now(_bc_radio.task_id);
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

        size_t length = 0;

        buffer[length++] = _bc_radio.device_address;
        buffer[length++] = _bc_radio.device_address >> 8;
        buffer[length++] = _bc_radio.device_address >> 16;
        buffer[length++] = _bc_radio.device_address >> 24;
        buffer[length++] = _bc_radio.message_id;
        buffer[length++] = _bc_radio.message_id >> 8;
        buffer[length++] = BC_RADIO_HEADER_ENROLL;

        _bc_radio.message_id++;

        bc_spirit1_set_tx_length(length);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 10;

        _bc_radio.state = BC_RADIO_STATE_TX;
    }

    if (_bc_radio.pub_queue_length != 0)
    {
        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        size_t length = 0;

        buffer[length++] = _bc_radio.device_address;
        buffer[length++] = _bc_radio.device_address >> 8;
        buffer[length++] = _bc_radio.device_address >> 16;
        buffer[length++] = _bc_radio.device_address >> 24;
        buffer[length++] = _bc_radio.message_id;
        buffer[length++] = _bc_radio.message_id >> 8;

        _bc_radio.message_id++;

        if (_bc_radio.pub_queue_buffer[0] == BC_RADIO_HEADER_PUB_PUSH_BUTTON)
        {
            size_t item_length = 1 + sizeof(uint16_t);

            memcpy(buffer + length, _bc_radio.pub_queue_buffer, item_length);

            length += item_length;

            _bc_radio.pub_queue_length -= item_length;

            memmove(_bc_radio.pub_queue_buffer, _bc_radio.pub_queue_buffer + item_length, _bc_radio.pub_queue_length);
        }
        else if (_bc_radio.pub_queue_buffer[0] == BC_RADIO_HEADER_PUB_THERMOMETER)
        {
            size_t item_length = 1 + sizeof(float);

            memcpy(buffer + length, _bc_radio.pub_queue_buffer, item_length);

            length += item_length;

            _bc_radio.pub_queue_length -= item_length;

            memmove(_bc_radio.pub_queue_buffer, _bc_radio.pub_queue_buffer + item_length, _bc_radio.pub_queue_length);
        }

        bc_spirit1_set_tx_length(length);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 10;

        _bc_radio.state = BC_RADIO_STATE_TX;
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
    }
}
