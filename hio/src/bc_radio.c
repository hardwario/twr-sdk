#include <bc_radio.h>
#include <bc_queue.h>
#include <bc_atsha204.h>
#include <bc_scheduler.h>
#include <bc_eeprom.h>
#include <bc_i2c.h>
#include <bc_radio_pub.h>
#include <bc_radio_node.h>
#include <math.h>

#define _BC_RADIO_SCAN_CACHE_LENGTH	4
#define _BC_RADIO_ACK_TIMEOUT       100
#define _BC_RADIO_SLEEP_RX_TIMEOUT  100
#define _BC_RADIO_TX_MAX_COUNT      6
#define _BC_RADIO_ACK_SUB_REQUEST   0x11

typedef enum
{
    BC_RADIO_STATE_SLEEP = 0,
    BC_RADIO_STATE_TX = 1,
    BC_RADIO_STATE_RX = 2,
    BC_RADIO_STATE_TX_WAIT_ACK = 3,
    BC_RADIO_STATE_RX_SEND_ACK = 4,
    BC_RADIO_STATE_TX_SEND_ACK = 5,

} bc_radio_state_t;

static struct
{
    bc_radio_mode_t mode;
    bc_atsha204_t atsha204;
    bc_radio_state_t state;
    uint64_t my_id;
    uint16_t message_id;
    int transmit_count;
    void (*event_handler)(bc_radio_event_t, void *);
    void *event_param;
    bc_scheduler_task_id_t task_id;
    bool pairing_request_to_gateway;
    const char *firmware;
    const char *firmware_version;
    bool pairing_mode;

    bc_queue_t pub_queue;
    bc_queue_t rx_queue;
    uint8_t pub_queue_buffer[BC_RADIO_PUB_QUEUE_BUFFER_SIZE];
    uint8_t rx_queue_buffer[BC_RADIO_RX_QUEUE_BUFFER_SIZE];

    uint8_t ack_tx_cache_buffer[15];
    size_t ack_tx_cache_length;
    int ack_transmit_count;
    bc_tick_t rx_timeout;
    bool ack;

    bc_radio_peer_t peer_devices[BC_RADIO_MAX_DEVICES];
    int peer_devices_length;

    uint64_t peer_id;

    bc_tick_t sleeping_mode_rx_timeout;
    bc_tick_t rx_timeout_sleeping;

    bool scan;
    uint64_t scan_cache[_BC_RADIO_SCAN_CACHE_LENGTH];
    uint8_t scan_length;
    uint8_t scan_head;

    bool automatic_pairing;
    bool save_peer_devices;

    bc_radio_sub_t *subs;
    int subs_length;
    int sent_subs;

} _bc_radio;

static void _bc_radio_task(void *param);
static void _bc_radio_go_to_state_rx_or_sleep(void);
static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param);
static void _bc_radio_load_peer_devices(void);
static void _bc_radio_save_peer_devices(void);
static void _bc_radio_atsha204_event_handler(bc_atsha204_t *self, bc_atsha204_event_t event, void *event_param);
static bool _bc_radio_peer_device_add(uint64_t id);
static bool _bc_radio_peer_device_remove(uint64_t id);

__attribute__((weak)) void bc_radio_on_info(uint64_t *id, char *firmware, char *version, bc_radio_mode_t mode) { (void) id; (void) firmware; (void) version; (void) mode;}
__attribute__((weak)) void bc_radio_on_sub(uint64_t *id, uint8_t *order, bc_radio_sub_pt_t *pt, char *topic) { (void) id; (void) order; (void) pt; (void) topic; }

void bc_radio_init(bc_radio_mode_t mode)
{
    memset(&_bc_radio, 0, sizeof(_bc_radio));

    _bc_radio.mode = mode;

    bc_atsha204_init(&_bc_radio.atsha204, BC_I2C_I2C0, 0x64);
    bc_atsha204_set_event_handler(&_bc_radio.atsha204, _bc_radio_atsha204_event_handler, NULL);
    bc_atsha204_read_serial_number(&_bc_radio.atsha204);

    bc_queue_init(&_bc_radio.pub_queue, _bc_radio.pub_queue_buffer, sizeof(_bc_radio.pub_queue_buffer));
    bc_queue_init(&_bc_radio.rx_queue, _bc_radio.rx_queue_buffer, sizeof(_bc_radio.rx_queue_buffer));

    bc_spirit1_init();
    bc_spirit1_set_event_handler(_bc_radio_spirit1_event_handler, NULL);

    _bc_radio_load_peer_devices();

    _bc_radio.task_id = bc_scheduler_register(_bc_radio_task, NULL, BC_TICK_INFINITY);

    _bc_radio_go_to_state_rx_or_sleep();
}

void bc_radio_set_event_handler(void (*event_handler)(bc_radio_event_t, void *), void *event_param)
{
    _bc_radio.event_handler = event_handler;
    _bc_radio.event_param = event_param;
}

void bc_radio_listen(bc_tick_t timeout)
{
    _bc_radio.rx_timeout_sleeping = bc_tick_get() + timeout;

    bc_scheduler_plan_now(_bc_radio.task_id);
}

void bc_radio_pairing_request(const char *firmware, const char *version)
{
    if ((_bc_radio.firmware != NULL) || (_bc_radio.firmware_version != NULL))
    {
        return;
    }

    _bc_radio.firmware = firmware;

    _bc_radio.firmware_version = version;

    _bc_radio.pairing_request_to_gateway = true;

    bc_scheduler_plan_now(_bc_radio.task_id);
}

void bc_radio_pairing_mode_start(void)
{
    _bc_radio.pairing_mode = true;
}

void bc_radio_pairing_mode_stop(void)
{
    _bc_radio.pairing_mode = false;
}

bool bc_radio_peer_device_add(uint64_t id)
{

    if (!_bc_radio_peer_device_add(id))
    {
        return false;
    }

    uint8_t buffer[1 + BC_RADIO_ID_SIZE];

    buffer[0] = BC_RADIO_HEADER_NODE_ATTACH;

    bc_radio_id_to_buffer(&id, buffer + 1);

    bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer));

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_peer_device_remove(uint64_t id)
{
    if (!_bc_radio_peer_device_remove(id))
    {
        return false;
    }

    uint8_t buffer[1 + BC_RADIO_ID_SIZE];

    buffer[0] = BC_RADIO_HEADER_NODE_DETACH;

    bc_radio_id_to_buffer(&id, buffer + 1);

    bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer));

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_peer_device_purge_all(void)
{
    for (int i = _bc_radio.peer_devices_length -1; i > -1 ; i--)
    {
        if (_bc_radio.peer_devices[i].id != 0)
        {
            if (!bc_radio_peer_device_remove(_bc_radio.peer_devices[i].id))
            {
                return false;
            }
        }
    }
    return true;
}

void bc_radio_get_peer_id(uint64_t *id, int length)
{
    int i;
    for (i = 0; (i < _bc_radio.peer_devices_length) && (i < length); i++)
    {
        id[i] = _bc_radio.peer_devices[i].id;
    }
    for (;i < length; i++)
    {
        id[i] = 0;
    }
}

void bc_radio_scan_start(void)
{
    memset(_bc_radio.scan_cache, 0x00, sizeof(_bc_radio.scan_cache));
    _bc_radio.scan_length = 0;
    _bc_radio.scan_head = 0;
    _bc_radio.scan = true;
}

void bc_radio_scan_stop(void)
{
    _bc_radio.scan = false;
}

void bc_radio_automatic_pairing_start(void)
{
    _bc_radio.automatic_pairing = true;
}

void bc_radio_automatic_pairing_stop(void)
{
    _bc_radio.automatic_pairing = false;
}

uint64_t bc_radio_get_my_id(void)
{
    return _bc_radio.my_id;
}

uint64_t bc_radio_get_event_id(void)
{
    return _bc_radio.peer_id;
}

bool bc_radio_is_peer_device(uint64_t id)
{
    for (int i = 0; i < _bc_radio.peer_devices_length; i++)
    {
        if (id == _bc_radio.peer_devices[i].id)
        {
            return true;
        }
    }
    return false;
}

bool bc_radio_pub_queue_put(const void *buffer, size_t length)
{
    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, length))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

void bc_radio_set_subs(bc_radio_sub_t *subs, int length)
{
    _bc_radio.subs = subs;

    _bc_radio.subs_length = length;

    _bc_radio.sent_subs = 0;
}

bool bc_radio_send_sub_data(uint64_t *id, uint8_t order, void *payload, size_t size)
{
    uint8_t qbuffer[1 + BC_RADIO_ID_SIZE + BC_RADIO_NODE_MAX_BUFFER_SIZE];

    if (size > BC_RADIO_NODE_MAX_BUFFER_SIZE - 1)
    {
        return false;
    }

    qbuffer[0] = BC_RADIO_HEADER_SUB_DATA;

    uint8_t *pqbuffer = bc_radio_id_to_buffer(id, qbuffer + 1);

    *pqbuffer++ = order;

    if (payload == NULL)
    {
        size = 0;
    }

    if (size > 0)
    {
        memcpy(pqbuffer, payload, size);
    }

    return bc_radio_pub_queue_put(qbuffer, 1 + BC_RADIO_ID_SIZE + 1 + size);
}

void bc_radio_set_rx_timeout_for_sleeping_node(bc_tick_t timeout)
{
    _bc_radio.sleeping_mode_rx_timeout = timeout;
}

static void _bc_radio_task(void *param)
{
    (void) param;

    if (_bc_radio.my_id == 0)
    {
        bc_atsha204_read_serial_number(&_bc_radio.atsha204);

        bc_scheduler_plan_current_now();

        return;
    }

    if ((_bc_radio.state != BC_RADIO_STATE_RX) && (_bc_radio.state != BC_RADIO_STATE_SLEEP))
    {
        bc_scheduler_plan_current_now();

        return;
    }

    if (_bc_radio.save_peer_devices)
    {
        _bc_radio_save_peer_devices();
    }

    if (_bc_radio.pairing_request_to_gateway)
    {
        _bc_radio.pairing_request_to_gateway = false;

        size_t len_firmware = strlen(_bc_radio.firmware);

        size_t len = len_firmware + strlen(_bc_radio.firmware_version);

        if (len > BC_RADIO_MAX_BUFFER_SIZE - 5)
        {
            return;
        }

        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        bc_radio_id_to_buffer(&_bc_radio.my_id, buffer);

        _bc_radio.message_id++;

        buffer[6] = _bc_radio.message_id;
        buffer[7] = _bc_radio.message_id >> 8;

        buffer[8] = BC_RADIO_HEADER_PAIRING;
        buffer[9] = len_firmware;

        strncpy((char *)buffer + 10, _bc_radio.firmware, BC_RADIO_MAX_BUFFER_SIZE - 2);
        strncpy((char *)buffer + 10 + len_firmware + 1, _bc_radio.firmware_version, BC_RADIO_MAX_BUFFER_SIZE - 2 - len_firmware - 1);

        buffer[10 + len + 1] = _bc_radio.mode;

        bc_spirit1_set_tx_length(10 + len + 2);

        bc_spirit1_tx();

        _bc_radio.transmit_count = _BC_RADIO_TX_MAX_COUNT;

        _bc_radio.state = BC_RADIO_STATE_TX;

        return;
    }

    if (_bc_radio.ack && (_bc_radio.sent_subs != _bc_radio.subs_length))
    {
        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        bc_radio_sub_t *sub = &_bc_radio.subs[_bc_radio.sent_subs];

        bc_radio_id_to_buffer(&_bc_radio.my_id, buffer);

        _bc_radio.message_id++;

        buffer[6] = _bc_radio.message_id;
        buffer[7] = _bc_radio.message_id >> 8;

        buffer[8] = BC_RADIO_HEADER_SUB_REG;

        buffer[9] = _bc_radio.sent_subs;

        buffer[10] = sub->type;

        strncpy((char *)buffer + 11, sub->topic, BC_RADIO_MAX_BUFFER_SIZE - 3);

        bc_spirit1_set_tx_length(11 + strlen(sub->topic) + 1);

        bc_spirit1_tx();

        _bc_radio.transmit_count = _BC_RADIO_TX_MAX_COUNT;

        _bc_radio.state = BC_RADIO_STATE_TX;

        return;
    }

    uint8_t queue_item_buffer[sizeof(_bc_radio.pub_queue_buffer)];
    size_t queue_item_length;
    uint64_t id;

    while (bc_queue_get(&_bc_radio.rx_queue, queue_item_buffer, &queue_item_length))
    {
        bc_radio_id_from_buffer(queue_item_buffer, &id);

        queue_item_length -= BC_RADIO_HEAD_SIZE;

        bc_radio_pub_decode(&id, queue_item_buffer + BC_RADIO_HEAD_SIZE, queue_item_length);

        bc_radio_node_decode(&id, queue_item_buffer + BC_RADIO_HEAD_SIZE, queue_item_length);

        if (queue_item_buffer[BC_RADIO_HEAD_SIZE] == BC_RADIO_HEADER_SUB_DATA)
        {
            uint8_t order = queue_item_buffer[BC_RADIO_HEAD_SIZE + 1 + BC_RADIO_ID_SIZE];

            if (order >= _bc_radio.subs_length)
            {
                return;
            }

            bc_radio_sub_t *sub = &_bc_radio.subs[order];

            if (sub->callback != NULL)
            {
                uint8_t *payload = NULL;

                if (queue_item_length > 1 + BC_RADIO_ID_SIZE + 1)
                {
                    payload = queue_item_buffer + BC_RADIO_HEAD_SIZE + 1 + BC_RADIO_ID_SIZE + 1;
                }

                sub->callback(&id, sub->topic, payload, sub->param);
            }
        }
        else if (queue_item_buffer[BC_RADIO_HEAD_SIZE] == BC_RADIO_HEADER_PUB_INFO)
        {
            queue_item_buffer[queue_item_length + BC_RADIO_HEAD_SIZE - 1] = 0;

            bc_radio_on_info(&id, (char *) queue_item_buffer + BC_RADIO_HEAD_SIZE + 1, "", BC_RADIO_MODE_UNKNOWN);
        }
        else if (queue_item_buffer[BC_RADIO_HEAD_SIZE] == BC_RADIO_HEADER_SUB_REG)
        {
            uint8_t *order = queue_item_buffer + BC_RADIO_HEAD_SIZE + 1;

            bc_radio_sub_pt_t *pt = (bc_radio_sub_pt_t *) queue_item_buffer + BC_RADIO_HEAD_SIZE + 2;

            char *topic = (char *) queue_item_buffer + BC_RADIO_HEAD_SIZE + 3;

            bc_radio_on_sub(&id, order, pt, topic);
        }
    }

    if (bc_queue_get(&_bc_radio.pub_queue, queue_item_buffer, &queue_item_length))
    {
        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        bc_radio_id_to_buffer(&_bc_radio.my_id, buffer);

        _bc_radio.message_id++;

        buffer[6] = _bc_radio.message_id;
        buffer[7] = _bc_radio.message_id >> 8;

        memcpy(buffer + 8, queue_item_buffer, queue_item_length);

        bc_spirit1_set_tx_length(8 + queue_item_length);

        bc_spirit1_tx();

        _bc_radio.transmit_count = _BC_RADIO_TX_MAX_COUNT;

        _bc_radio.state = BC_RADIO_STATE_TX;
    }
}

static bool _bc_radio_scan_cache_push(void)
{
    for (uint8_t i = 0; i < _bc_radio.scan_length; i++)
    {
        if (_bc_radio.scan_cache[i] == _bc_radio.peer_id)
        {
            return false;
        }
    }

    if (_bc_radio.scan_length < _BC_RADIO_SCAN_CACHE_LENGTH)
    {
        _bc_radio.scan_length++;
    }

    _bc_radio.scan_cache[_bc_radio.scan_head++] = _bc_radio.peer_id;

    if (_bc_radio.scan_head == _BC_RADIO_SCAN_CACHE_LENGTH)
    {
        _bc_radio.scan_head = 0;
    }

    return true;
}

static void _bc_radio_send_ack(void)
{
    uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

    if (_bc_radio.state == BC_RADIO_STATE_TX_WAIT_ACK)
    {
        _bc_radio.ack_transmit_count = _bc_radio.transmit_count;

        _bc_radio.ack_tx_cache_length = bc_spirit1_get_tx_length();

        memcpy(_bc_radio.ack_tx_cache_buffer, tx_buffer, sizeof(_bc_radio.ack_tx_cache_buffer));

        _bc_radio.state = BC_RADIO_STATE_TX_SEND_ACK;
    }
    else if (_bc_radio.state == BC_RADIO_STATE_RX)
    {
        _bc_radio.state = BC_RADIO_STATE_RX_SEND_ACK;
    }
    else
    {
        return;
    }

    uint8_t *rx_buffer = bc_spirit1_get_rx_buffer();

    memcpy(tx_buffer, rx_buffer, 8);

    tx_buffer[8] = BC_RADIO_HEADER_ACK;

    bc_spirit1_set_tx_length(9);

    _bc_radio.transmit_count = 2;

    bc_spirit1_tx();
}

static void _bc_radio_go_to_state_rx_or_sleep(void)
{
    if (_bc_radio.mode == BC_RADIO_MODE_NODE_SLEEPING)
    {
        bc_tick_t now = bc_tick_get();

        if (_bc_radio.rx_timeout_sleeping > now)
        {
            _bc_radio.rx_timeout = _bc_radio.rx_timeout_sleeping;

            bc_spirit1_set_rx_timeout(_bc_radio.rx_timeout - now);

            bc_spirit1_rx();

            _bc_radio.state = BC_RADIO_STATE_RX;
        }
        else
        {
            bc_spirit1_sleep();

            _bc_radio.state = BC_RADIO_STATE_SLEEP;
        }
    }
    else
    {
        _bc_radio.rx_timeout = BC_TICK_INFINITY;

        bc_spirit1_set_rx_timeout(BC_TICK_INFINITY);

        bc_spirit1_rx();

        _bc_radio.state = BC_RADIO_STATE_RX;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);
}

static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_SPIRIT1_EVENT_TX_DONE)
    {
        if (_bc_radio.transmit_count > 0)
        {
            _bc_radio.transmit_count--;
        }

        if (_bc_radio.state == BC_RADIO_STATE_TX)
        {
            bc_tick_t timeout = _BC_RADIO_ACK_TIMEOUT - 50 + rand() % _BC_RADIO_ACK_TIMEOUT;

            _bc_radio.rx_timeout = bc_tick_get() + timeout;

            bc_spirit1_set_rx_timeout(timeout);

            bc_spirit1_rx();

            _bc_radio.state = BC_RADIO_STATE_TX_WAIT_ACK;

            _bc_radio.ack = false;

            return;
        }

        if (_bc_radio.state == BC_RADIO_STATE_RX_SEND_ACK)
        {
            if (_bc_radio.transmit_count > 0)
            {
                bc_spirit1_tx();

                return;
            }
        }

        if (_bc_radio.state == BC_RADIO_STATE_TX_SEND_ACK)
        {
            if (_bc_radio.transmit_count > 0)
            {
                bc_spirit1_tx();

                return;
            }
            else
            {
                uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

                memcpy(tx_buffer, _bc_radio.ack_tx_cache_buffer, sizeof(_bc_radio.ack_tx_cache_buffer));

                bc_tick_t timeout = _BC_RADIO_ACK_TIMEOUT - 50 + rand() % _BC_RADIO_ACK_TIMEOUT;

                _bc_radio.rx_timeout = bc_tick_get() + timeout;

                _bc_radio.transmit_count = _bc_radio.ack_transmit_count;

                bc_spirit1_set_rx_timeout(timeout);

                bc_spirit1_set_tx_length(_bc_radio.ack_tx_cache_length);

                bc_spirit1_rx();

                _bc_radio.state = BC_RADIO_STATE_TX_WAIT_ACK;

                return;
            }
        }

        _bc_radio_go_to_state_rx_or_sleep();
    }
    else if (event == BC_SPIRIT1_EVENT_RX_TIMEOUT)
    {
        if (_bc_radio.state == BC_RADIO_STATE_TX_WAIT_ACK)
        {
            if (_bc_radio.transmit_count > 0)
            {
                bc_spirit1_tx();

                _bc_radio.state = BC_RADIO_STATE_TX;

                return;
            }
            else
            {
                if (_bc_radio.event_handler)
                {
                    _bc_radio.event_handler(BC_RADIO_EVENT_TX_ERROR, _bc_radio.event_param);
                }
            }

        }

        _bc_radio_go_to_state_rx_or_sleep();
    }
    else if (event == BC_SPIRIT1_EVENT_RX_DONE)
    {
        size_t length = bc_spirit1_get_rx_length();
        uint16_t message_id;

        if ((_bc_radio.rx_timeout != BC_TICK_INFINITY) && (bc_tick_get() >= _bc_radio.rx_timeout))
        {
            if (_bc_radio.state == BC_RADIO_STATE_TX_WAIT_ACK)
            {
                if (_bc_radio.transmit_count > 0)
                {
                    bc_spirit1_tx();

                    _bc_radio.state = BC_RADIO_STATE_TX;

                    return;
                }
            }

            _bc_radio_go_to_state_rx_or_sleep();
        }

        if (length >= 9)
        {
            uint8_t *buffer = bc_spirit1_get_rx_buffer();
            bc_radio_peer_t *peer;

            bc_radio_id_from_buffer(buffer, &_bc_radio.peer_id);

            message_id = (uint16_t) buffer[6];
            message_id |= (uint16_t) buffer[7] << 8;

            // ACK check
            if (buffer[8] == BC_RADIO_HEADER_ACK)
            {
                if (_bc_radio.state == BC_RADIO_STATE_TX_WAIT_ACK)
                {
                    uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

                    if ((_bc_radio.peer_id == _bc_radio.my_id) && (_bc_radio.message_id == message_id) )
                    {
                        _bc_radio.transmit_count = 0;

                        _bc_radio.ack = true;

                        if (tx_buffer[8] == BC_RADIO_HEADER_PAIRING)
                        {
                            if (length == 15)
                            {
                                bc_radio_id_from_buffer(buffer + 9, &_bc_radio.peer_id);

                                if ((_bc_radio.mode != BC_RADIO_MODE_GATEWAY) && (_bc_radio.peer_devices[0].id != _bc_radio.peer_id))
                                {
                                    _bc_radio.peer_devices[0].id = _bc_radio.peer_id;
                                    _bc_radio.peer_devices[0].message_id_synced = false;
                                    _bc_radio.peer_devices_length = 1;

                                    _bc_radio.save_peer_devices = true;
                                    bc_scheduler_plan_now(_bc_radio.task_id);

                                    _bc_radio.sent_subs = 0;

                                    if (_bc_radio.event_handler)
                                    {
                                        _bc_radio.event_handler(BC_RADIO_EVENT_PAIRED, _bc_radio.event_param);
                                    }
                                }
                            }
                        }
                        else if (tx_buffer[8] == BC_RADIO_HEADER_SUB_REG)
                        {
                            _bc_radio.sent_subs++;
                        }
                        else if ((length == 10) && (buffer[9] == _BC_RADIO_ACK_SUB_REQUEST))
                        {
                            _bc_radio.sent_subs = 0;
                        }

                        if (_bc_radio.sleeping_mode_rx_timeout != 0)
                        {
                            _bc_radio.rx_timeout_sleeping = bc_tick_get() + _bc_radio.sleeping_mode_rx_timeout;
                        }

                        _bc_radio_go_to_state_rx_or_sleep();

                        if (_bc_radio.event_handler)
                        {
                            _bc_radio.event_handler(BC_RADIO_EVENT_TX_DONE, _bc_radio.event_param);
                        }
                    }

                }

                return;
            }

            if (buffer[8] == BC_RADIO_HEADER_PAIRING)
            {
                if (_bc_radio.pairing_mode)
                {
                    if (length >= 9)
                    {
                        bc_radio_peer_device_add(_bc_radio.peer_id);
                    }
                }

                peer = bc_radio_get_peer_device(_bc_radio.peer_id);

                if (peer != NULL)
                {
                    _bc_radio_send_ack();

                    uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

                    bc_radio_id_to_buffer(&_bc_radio.my_id, tx_buffer + 9);

                    bc_spirit1_set_tx_length(15);

                    if ((length > 10) && (peer->message_id != message_id))
                    {
                        if (10 + (size_t) buffer[9] + 1 < length)
                        {
                            buffer[10 + buffer[9]] = 0;

                            peer->mode = buffer[length - 1];

                            buffer[length - 1] = 0;

                            bc_radio_on_info(&_bc_radio.peer_id, (char *)buffer + 10, (char *)buffer + 10 + buffer[9] + 1, peer->mode);
                        }
                    }

                    peer->message_id = message_id;

                    peer->message_id_synced = true;
                }

                return;
            }

            if ((length == 15) && ((buffer[8] == BC_RADIO_HEADER_NODE_ATTACH) || (buffer[8] == BC_RADIO_HEADER_NODE_DETACH)))
            {
                uint64_t id;

                bc_radio_id_from_buffer(buffer + 9, &id);

                if (id == _bc_radio.my_id)
                {
                    if (buffer[8] == BC_RADIO_HEADER_NODE_ATTACH)
                    {
                        _bc_radio.pairing_request_to_gateway = true;

                        bc_scheduler_plan_now(_bc_radio.task_id);
                    }
                    else if (buffer[8] == BC_RADIO_HEADER_NODE_DETACH)
                    {
                        _bc_radio_peer_device_remove(_bc_radio.peer_id);

                        if (_bc_radio.event_handler)
                        {
                            _bc_radio.event_handler(BC_RADIO_EVENT_UNPAIRED, _bc_radio.event_param);
                        }
                    }
                }

                _bc_radio_send_ack();

                return;
            }

            peer = bc_radio_get_peer_device(_bc_radio.peer_id);

            if (peer != NULL)
            {
                if (peer->message_id != message_id)
                {
                    bool send_subs_request = _bc_radio.mode == BC_RADIO_MODE_GATEWAY && (!peer->message_id_synced || (peer->message_id > message_id));

                    peer->message_id = message_id;

                    peer->message_id_synced = false;

                    if (length > 9)
                    {
                        if ((buffer[8] >= 0x15) && (buffer[8] <= 0x1d) && (length > 14))
                        {
                            uint64_t for_id;

                            bc_radio_id_from_buffer(buffer + 9, &for_id);

                            if (for_id != _bc_radio.my_id)
                            {
                                return;
                            }
                        }

                        bc_queue_put(&_bc_radio.rx_queue, buffer, length);

                        bc_scheduler_plan_now(_bc_radio.task_id);

                        peer->message_id_synced = true;

                        peer->rssi = bc_spirit1_get_rx_rssi();
                    }

                    if (peer->message_id_synced)
                    {
                        _bc_radio_send_ack();

                        if (send_subs_request)
                        {
                            uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

                            tx_buffer[9] = _BC_RADIO_ACK_SUB_REQUEST;

                            bc_spirit1_set_tx_length(10);
                        }
                    }

                    return;
                }
            }
            else
            {
                if (_bc_radio.scan && (_bc_radio.event_handler != NULL) && _bc_radio_scan_cache_push())
                {
                    _bc_radio.event_handler(BC_RADIO_EVENT_SCAN_FIND_DEVICE, _bc_radio.event_param);
                }

                if (_bc_radio.automatic_pairing)
                {
                    bc_radio_peer_device_add(_bc_radio.peer_id);
                }
            }
        }
    }
}

static void _bc_radio_load_peer_devices(void)
{
    uint32_t address = (uint32_t) bc_eeprom_get_size() - 8;
    uint64_t buffer[3];
    uint32_t *pointer = (uint32_t *)buffer;
    uint8_t length = 0;

    bc_eeprom_read(bc_eeprom_get_size() - 1, &length, 1);

    _bc_radio.peer_devices_length = 0;

    for (int i = 0; (i < length) && (i < BC_RADIO_MAX_DEVICES); i++)
    {
        address -= sizeof(buffer);

        bc_eeprom_read(address, buffer, sizeof(buffer));

        pointer[2] = ~pointer[2];
        pointer[5] = ~pointer[5];

        if ((buffer[0] != buffer[1]) && (buffer[0] != buffer[2]))
        {
            if (buffer[1] == buffer[2])
            {
                buffer[0] = buffer[1];

                _bc_radio.save_peer_devices = true;

                bc_scheduler_plan_now(_bc_radio.task_id);
            }
            else
            {
                continue;
            }
        }

        if (buffer[0] != 0)
        {
            _bc_radio.peer_devices[_bc_radio.peer_devices_length].id = buffer[0];
            _bc_radio.peer_devices[_bc_radio.peer_devices_length].message_id_synced = false;
            _bc_radio.peer_devices_length++;
        }
    }
}

static void _bc_radio_save_peer_devices(void)
{
    uint32_t address = (uint32_t) bc_eeprom_get_size() - 8;
    uint64_t buffer_write[3];
    uint32_t *pointer_write = (uint32_t *)buffer_write;
    uint64_t buffer_read[3];

    _bc_radio.save_peer_devices = false;

    for (int i = 0; i < _bc_radio.peer_devices_length; i++)
    {
        buffer_write[0] = _bc_radio.peer_devices[i].id;
        buffer_write[1] = _bc_radio.peer_devices[i].id;
        buffer_write[2] = _bc_radio.peer_devices[i].id;

        pointer_write[2] = ~pointer_write[2];
        pointer_write[5] = ~pointer_write[5];

        address -= sizeof(buffer_write);

        bc_eeprom_read(address, buffer_read, sizeof(buffer_read));

        if (memcmp(buffer_read, buffer_write, sizeof(buffer_write)) != 0)
        {
            if (!bc_eeprom_write(address, buffer_write, sizeof(buffer_write)))
            {
                _bc_radio.save_peer_devices = true;

                bc_scheduler_plan_now(_bc_radio.task_id);

                return;
            }
        }
    }

    if (!bc_eeprom_write(bc_eeprom_get_size() - 1, &_bc_radio.peer_devices_length, 1))
    {
        _bc_radio.save_peer_devices = true;

        bc_scheduler_plan_now(_bc_radio.task_id);

        return;
    }
}

static void _bc_radio_atsha204_event_handler(bc_atsha204_t *self, bc_atsha204_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_ATSHA204_EVENT_SERIAL_NUMBER)
    {
        if (bc_atsha204_get_serial_number(self, &_bc_radio.my_id, sizeof(_bc_radio.my_id)))
        {
            if (_bc_radio.event_handler != NULL)
            {
                _bc_radio.event_handler(BC_RADIO_EVENT_INIT_DONE, _bc_radio.event_param);
            }
        }
        else
        {
            if (_bc_radio.event_handler != NULL)
            {
                _bc_radio.event_handler(BC_RADIO_EVENT_INIT_FAILURE, _bc_radio.event_param);
            }
        }
    }
    else if (event == BC_ATSHA204_EVENT_ERROR)
    {
        if (_bc_radio.event_handler != NULL)
        {
            _bc_radio.event_handler(BC_RADIO_EVENT_INIT_FAILURE, _bc_radio.event_param);
        }
    }
}

static bool _bc_radio_peer_device_add(uint64_t id)
{
    if (_bc_radio.peer_devices_length + 1 == BC_RADIO_MAX_DEVICES)
    {
        if (_bc_radio.event_handler != NULL)
        {
            _bc_radio.peer_id = id;
            _bc_radio.event_handler(BC_RADIO_EVENT_ATTACH_FAILURE, _bc_radio.event_param);
        }
        return false;
    }

    if (bc_radio_is_peer_device(id))
    {
        return false;
    }

    _bc_radio.peer_devices[_bc_radio.peer_devices_length].id = id;
    _bc_radio.peer_devices[_bc_radio.peer_devices_length].message_id_synced = false;
    _bc_radio.peer_devices_length++;

    _bc_radio.save_peer_devices = true;
    bc_scheduler_plan_now(_bc_radio.task_id);

    if (_bc_radio.event_handler != NULL)
    {
        _bc_radio.peer_id = id;
        _bc_radio.event_handler(BC_RADIO_EVENT_ATTACH, _bc_radio.event_param);
    }

    return true;
}

static bool _bc_radio_peer_device_remove(uint64_t id)
{
    for (int i = 0; i < _bc_radio.peer_devices_length; i++)
    {
        if (id == _bc_radio.peer_devices[i].id)
        {
            _bc_radio.peer_devices_length--;

            if (i != _bc_radio.peer_devices_length)
            {
                memcpy(_bc_radio.peer_devices + i, _bc_radio.peer_devices + _bc_radio.peer_devices_length, sizeof(bc_radio_peer_t));
            }

            _bc_radio.save_peer_devices = true;
            bc_scheduler_plan_now(_bc_radio.task_id);

            if (_bc_radio.event_handler != NULL)
            {
                _bc_radio.peer_id = id;
                _bc_radio.event_handler(BC_RADIO_EVENT_DETACH, _bc_radio.event_param);
            }

            return true;
        }
    }

    return false;
}

bc_radio_peer_t *bc_radio_get_peer_device(uint64_t id)
{
    for (int i = 0; i < _bc_radio.peer_devices_length; i++)
    {
        if (id == _bc_radio.peer_devices[i].id)
        {
            return &_bc_radio.peer_devices[i];
        }
    }

    return NULL;
}

uint8_t *bc_radio_id_to_buffer(uint64_t *id, uint8_t *buffer)
{
    buffer[0] = *id;
    buffer[1] = *id >> 8;
    buffer[2] = *id >> 16;
    buffer[3] = *id >> 24;
    buffer[4] = *id >> 32;
    buffer[5] = *id >> 40;

    return buffer + BC_RADIO_ID_SIZE;
}

uint8_t *bc_radio_bool_to_buffer(bool *value, uint8_t *buffer)
{
    *buffer = value == NULL ? BC_RADIO_NULL_BOOL : *value;

    return buffer + 1;
}

uint8_t *bc_radio_int_to_buffer(int *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = BC_RADIO_NULL_INT;

        memcpy(buffer, &null, sizeof(int));
    }
    else
    {
        memcpy(buffer, value, sizeof(int));
    }

    return buffer + sizeof(int);
}

uint8_t *bc_radio_uint16_to_buffer(uint16_t *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = BC_RADIO_NULL_UINT16;

        memcpy(buffer, &null, sizeof(uint16_t));
    }
    else
    {
        memcpy(buffer, value, sizeof(uint16_t));
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *bc_radio_uint32_to_buffer(uint32_t *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = BC_RADIO_NULL_UINT32;

        memcpy(buffer, &null, sizeof(uint32_t));
    }
    else
    {
        memcpy(buffer, value, sizeof(uint32_t));
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *bc_radio_float_to_buffer(float *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const float null = BC_RADIO_NULL_FLOAT;

        memcpy(buffer, &null, sizeof(float));
    }
    else
    {
        memcpy(buffer, value, sizeof(float));
    }

    return buffer + sizeof(float);
}

uint8_t *bc_radio_data_to_buffer(void *data, size_t length, uint8_t *buffer)
{
    if (data == NULL)
    {
        return buffer + length;
    }

    memcpy(buffer, data, length);

    return buffer + length;
}

uint8_t *bc_radio_id_from_buffer(uint8_t *buffer, uint64_t *id)
{
    *id  = (uint64_t) buffer[0];
    *id |= (uint64_t) buffer[1] << 8;
    *id |= (uint64_t) buffer[2] << 16;
    *id |= (uint64_t) buffer[3] << 24;
    *id |= (uint64_t) buffer[4] << 32;
    *id |= (uint64_t) buffer[5] << 40;

    return buffer + BC_RADIO_ID_SIZE;
}

uint8_t *bc_radio_bool_from_buffer(uint8_t *buffer, bool *value, bool **pointer)
{
    if (*buffer == BC_RADIO_NULL_BOOL)
    {
        *pointer = NULL;
    }
    else
    {
        *value = *(bool *) buffer;
        *pointer = value;
    }

    return  buffer + 1;
}

uint8_t *bc_radio_int_from_buffer(uint8_t *buffer, int *value, int **pointer)
{
    memcpy(value, buffer, sizeof(int));

    if (*value == BC_RADIO_NULL_INT)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(int);
}

uint8_t *bc_radio_uint16_from_buffer(uint8_t *buffer, uint16_t *value, uint16_t **pointer)
{
    memcpy(value, buffer, sizeof(uint16_t));

    if (*value == BC_RADIO_NULL_UINT16)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(uint16_t);
}

uint8_t *bc_radio_uint32_from_buffer(uint8_t *buffer, uint32_t *value, uint32_t **pointer)
{
    memcpy(value, buffer, sizeof(uint32_t));

    if (*value == BC_RADIO_NULL_UINT32)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *bc_radio_float_from_buffer(uint8_t *buffer, float *value, float **pointer)
{
    memcpy(value, buffer, sizeof(float));

    if (isnan(*value))
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(float);
}

uint8_t *bc_radio_data_from_buffer(uint8_t *buffer, void *data, size_t length)
{
    if (data == NULL)
    {
        return buffer + length;
    }

    memcpy(data, buffer, length);

    return buffer + length;
}

typedef struct
{
    bc_led_t *led;
    const char *firmware;
    const char *version;

} _bc_radio_button_event_param_t;

void _bc_radio_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    _bc_radio_button_event_param_t *param = (_bc_radio_button_event_param_t*) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        if (param->led != NULL)
        {
            bc_led_pulse(param->led, 100);
        }

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_pairing_request(param->firmware, param->version);

        if (param->led != NULL)
        {
            bc_led_set_mode(param->led, BC_LED_MODE_OFF);
            bc_led_pulse(param->led, 1000);
        }
    }
}

void bc_radio_init_pairing_button(const char *firmware, const char *version)
{
    static bc_led_t led;
    static bc_button_t button;
    static _bc_radio_button_event_param_t param;
    param.led = &led;
    param.firmware = firmware;
    param.version = version;

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, 0);
    // Pass led instance as a callback parameter, so we don't need to add it to the radio structure
    bc_button_set_event_handler(&button, _bc_radio_button_event_handler, &param);

    bc_led_init(&led, BC_GPIO_LED, false, 0);

}
