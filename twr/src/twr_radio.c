#include <twr_radio.h>
#include <twr_queue.h>
#include <twr_atsha204.h>
#include <twr_scheduler.h>
#include <twr_eeprom.h>
#include <twr_i2c.h>
#include <twr_radio_pub.h>
#include <twr_radio_node.h>
#include <math.h>

#define _TWR_RADIO_SCAN_CACHE_LENGTH	4
#define _TWR_RADIO_ACK_TIMEOUT       100
#define _TWR_RADIO_SLEEP_RX_TIMEOUT  100
#define _TWR_RADIO_TX_MAX_COUNT      6
#define _TWR_RADIO_ACK_SUB_REQUEST   0x11

typedef enum
{
    TWR_RADIO_STATE_SLEEP = 0,
    TWR_RADIO_STATE_TX = 1,
    TWR_RADIO_STATE_RX = 2,
    TWR_RADIO_STATE_TX_WAIT_ACK = 3,
    TWR_RADIO_STATE_RX_SEND_ACK = 4,
    TWR_RADIO_STATE_TX_SEND_ACK = 5,

} twr_radio_state_t;

static struct
{
    twr_radio_mode_t mode;
    twr_atsha204_t atsha204;
    twr_radio_state_t state;
    uint64_t my_id;
    uint16_t message_id;
    int transmit_count;
    void (*event_handler)(twr_radio_event_t, void *);
    void *event_param;
    twr_scheduler_task_id_t task_id;
    bool pairing_request_to_gateway;
    const char *firmware;
    const char *firmware_version;
    bool pairing_mode;

    twr_queue_t pub_queue;
    twr_queue_t rx_queue;
    uint8_t pub_queue_buffer[TWR_RADIO_PUB_QUEUE_BUFFER_SIZE];
    uint8_t rx_queue_buffer[TWR_RADIO_RX_QUEUE_BUFFER_SIZE];

    uint8_t ack_tx_cache_buffer[15];
    size_t ack_tx_cache_length;
    int ack_transmit_count;
    twr_tick_t rx_timeout;
    bool ack;

    twr_radio_peer_t peer_devices[TWR_RADIO_MAX_DEVICES];
    int peer_devices_length;

    uint64_t peer_id;

    twr_tick_t sleeping_mode_rx_timeout;
    twr_tick_t rx_timeout_sleeping;

    bool scan;
    uint64_t scan_cache[_TWR_RADIO_SCAN_CACHE_LENGTH];
    uint8_t scan_length;
    uint8_t scan_head;

    bool automatic_pairing;
    bool save_peer_devices;

    twr_radio_sub_t *subs;
    int subs_length;
    int sent_subs;

} _twr_radio;

static void _twr_radio_task(void *param);
static void _twr_radio_go_to_state_rx_or_sleep(void);
static void _twr_radio_spirit1_event_handler(twr_spirit1_event_t event, void *event_param);
static void _twr_radio_load_peer_devices(void);
static void _twr_radio_save_peer_devices(void);
static void _twr_radio_atsha204_event_handler(twr_atsha204_t *self, twr_atsha204_event_t event, void *event_param);
static bool _twr_radio_peer_device_add(uint64_t id);
static bool _twr_radio_peer_device_remove(uint64_t id);

__attribute__((weak)) void twr_radio_on_info(uint64_t *id, char *firmware, char *version, twr_radio_mode_t mode) { (void) id; (void) firmware; (void) version; (void) mode;}
__attribute__((weak)) void twr_radio_on_sub(uint64_t *id, uint8_t *order, twr_radio_sub_pt_t *pt, char *topic) { (void) id; (void) order; (void) pt; (void) topic; }

void twr_radio_init(twr_radio_mode_t mode)
{
    memset(&_twr_radio, 0, sizeof(_twr_radio));

    _twr_radio.mode = mode;

    twr_atsha204_init(&_twr_radio.atsha204, TWR_I2C_I2C0, 0x64);
    twr_atsha204_set_event_handler(&_twr_radio.atsha204, _twr_radio_atsha204_event_handler, NULL);
    twr_atsha204_read_serial_number(&_twr_radio.atsha204);

    twr_queue_init(&_twr_radio.pub_queue, _twr_radio.pub_queue_buffer, sizeof(_twr_radio.pub_queue_buffer));
    twr_queue_init(&_twr_radio.rx_queue, _twr_radio.rx_queue_buffer, sizeof(_twr_radio.rx_queue_buffer));

    twr_spirit1_init();
    twr_spirit1_set_event_handler(_twr_radio_spirit1_event_handler, NULL);

    _twr_radio_load_peer_devices();

    _twr_radio.task_id = twr_scheduler_register(_twr_radio_task, NULL, TWR_TICK_INFINITY);

    _twr_radio_go_to_state_rx_or_sleep();
}

void twr_radio_set_event_handler(void (*event_handler)(twr_radio_event_t, void *), void *event_param)
{
    _twr_radio.event_handler = event_handler;
    _twr_radio.event_param = event_param;
}

void twr_radio_listen(twr_tick_t timeout)
{
    _twr_radio.rx_timeout_sleeping = twr_tick_get() + timeout;

    twr_scheduler_plan_now(_twr_radio.task_id);
}

void twr_radio_pairing_request(const char *firmware, const char *version)
{
    _twr_radio.firmware = firmware == NULL ? "" : firmware;

    _twr_radio.firmware_version = version == NULL ? "" : version;

    _twr_radio.pairing_request_to_gateway = true;

    twr_scheduler_plan_now(_twr_radio.task_id);
}

void twr_radio_pairing_mode_start(void)
{
    _twr_radio.pairing_mode = true;
}

void twr_radio_pairing_mode_stop(void)
{
    _twr_radio.pairing_mode = false;
}

bool twr_radio_peer_device_add(uint64_t id)
{

    if (!_twr_radio_peer_device_add(id))
    {
        return false;
    }

    uint8_t buffer[1 + TWR_RADIO_ID_SIZE];

    buffer[0] = TWR_RADIO_HEADER_NODE_ATTACH;

    twr_radio_id_to_buffer(&id, buffer + 1);

    twr_queue_put(&_twr_radio.pub_queue, buffer, sizeof(buffer));

    twr_scheduler_plan_now(_twr_radio.task_id);

    return true;
}

bool twr_radio_peer_device_remove(uint64_t id)
{
    if (!_twr_radio_peer_device_remove(id))
    {
        return false;
    }

    uint8_t buffer[1 + TWR_RADIO_ID_SIZE];

    buffer[0] = TWR_RADIO_HEADER_NODE_DETACH;

    twr_radio_id_to_buffer(&id, buffer + 1);

    twr_queue_put(&_twr_radio.pub_queue, buffer, sizeof(buffer));

    twr_scheduler_plan_now(_twr_radio.task_id);

    return true;
}

bool twr_radio_peer_device_purge_all(void)
{
    for (int i = _twr_radio.peer_devices_length -1; i > -1 ; i--)
    {
        if (_twr_radio.peer_devices[i].id != 0)
        {
            if (!twr_radio_peer_device_remove(_twr_radio.peer_devices[i].id))
            {
                return false;
            }
        }
    }
    return true;
}

void twr_radio_get_peer_id(uint64_t *id, int length)
{
    int i;
    for (i = 0; (i < _twr_radio.peer_devices_length) && (i < length); i++)
    {
        id[i] = _twr_radio.peer_devices[i].id;
    }
    for (;i < length; i++)
    {
        id[i] = 0;
    }
}

void twr_radio_scan_start(void)
{
    memset(_twr_radio.scan_cache, 0x00, sizeof(_twr_radio.scan_cache));
    _twr_radio.scan_length = 0;
    _twr_radio.scan_head = 0;
    _twr_radio.scan = true;
}

void twr_radio_scan_stop(void)
{
    _twr_radio.scan = false;
}

void twr_radio_automatic_pairing_start(void)
{
    _twr_radio.automatic_pairing = true;
}

void twr_radio_automatic_pairing_stop(void)
{
    _twr_radio.automatic_pairing = false;
}

uint64_t twr_radio_get_my_id(void)
{
    return _twr_radio.my_id;
}

uint64_t twr_radio_get_event_id(void)
{
    return _twr_radio.peer_id;
}

bool twr_radio_is_peer_device(uint64_t id)
{
    for (int i = 0; i < _twr_radio.peer_devices_length; i++)
    {
        if (id == _twr_radio.peer_devices[i].id)
        {
            return true;
        }
    }
    return false;
}

bool twr_radio_pub_queue_put(const void *buffer, size_t length)
{
    if (!twr_queue_put(&_twr_radio.pub_queue, buffer, length))
    {
        return false;
    }

    twr_scheduler_plan_now(_twr_radio.task_id);

    return true;
}

void twr_radio_pub_queue_clear()
{
    twr_queue_clear(&_twr_radio.pub_queue);
}

void twr_radio_set_subs(twr_radio_sub_t *subs, int length)
{
    _twr_radio.subs = subs;

    _twr_radio.subs_length = length;

    _twr_radio.sent_subs = 0;
}

bool twr_radio_send_sub_data(uint64_t *id, uint8_t order, void *payload, size_t size)
{
    uint8_t qbuffer[1 + TWR_RADIO_ID_SIZE + TWR_RADIO_NODE_MAX_BUFFER_SIZE];

    if (size > TWR_RADIO_NODE_MAX_BUFFER_SIZE - 1)
    {
        return false;
    }

    qbuffer[0] = TWR_RADIO_HEADER_SUB_DATA;

    uint8_t *pqbuffer = twr_radio_id_to_buffer(id, qbuffer + 1);

    *pqbuffer++ = order;

    if (payload == NULL)
    {
        size = 0;
    }

    if (size > 0)
    {
        memcpy(pqbuffer, payload, size);
    }

    return twr_radio_pub_queue_put(qbuffer, 1 + TWR_RADIO_ID_SIZE + 1 + size);
}

void twr_radio_set_rx_timeout_for_sleeping_node(twr_tick_t timeout)
{
    _twr_radio.sleeping_mode_rx_timeout = timeout;
}

static void _twr_radio_task(void *param)
{
    (void) param;

    if (_twr_radio.my_id == 0)
    {
        twr_atsha204_read_serial_number(&_twr_radio.atsha204);

        twr_scheduler_plan_current_now();

        return;
    }

    if ((_twr_radio.state != TWR_RADIO_STATE_RX) && (_twr_radio.state != TWR_RADIO_STATE_SLEEP))
    {
        twr_scheduler_plan_current_now();

        return;
    }

    if (_twr_radio.save_peer_devices)
    {
        _twr_radio_save_peer_devices();
    }

    if (_twr_radio.pairing_request_to_gateway)
    {
        _twr_radio.pairing_request_to_gateway = false;

        size_t len_firmware = strlen(_twr_radio.firmware);

        size_t len = len_firmware + strlen(_twr_radio.firmware_version);

        if (len > TWR_RADIO_MAX_BUFFER_SIZE - 5)
        {
            return;
        }

        uint8_t *buffer = twr_spirit1_get_tx_buffer();

        twr_radio_id_to_buffer(&_twr_radio.my_id, buffer);

        _twr_radio.message_id++;

        buffer[6] = _twr_radio.message_id;
        buffer[7] = _twr_radio.message_id >> 8;

        buffer[8] = TWR_RADIO_HEADER_PAIRING;
        buffer[9] = len_firmware;

        strncpy((char *)buffer + 10, _twr_radio.firmware, TWR_RADIO_MAX_BUFFER_SIZE - 2);
        strncpy((char *)buffer + 10 + len_firmware + 1, _twr_radio.firmware_version, TWR_RADIO_MAX_BUFFER_SIZE - 2 - len_firmware - 1);

        buffer[10 + len + 1] = _twr_radio.mode;

        twr_spirit1_set_tx_length(10 + len + 2);

        twr_spirit1_tx();

        _twr_radio.transmit_count = _TWR_RADIO_TX_MAX_COUNT;

        _twr_radio.state = TWR_RADIO_STATE_TX;

        return;
    }

    if (_twr_radio.ack && (_twr_radio.sent_subs != _twr_radio.subs_length))
    {
        uint8_t *buffer = twr_spirit1_get_tx_buffer();

        twr_radio_sub_t *sub = &_twr_radio.subs[_twr_radio.sent_subs];

        twr_radio_id_to_buffer(&_twr_radio.my_id, buffer);

        _twr_radio.message_id++;

        buffer[6] = _twr_radio.message_id;
        buffer[7] = _twr_radio.message_id >> 8;

        buffer[8] = TWR_RADIO_HEADER_SUB_REG;

        buffer[9] = _twr_radio.sent_subs;

        buffer[10] = sub->type;

        strncpy((char *)buffer + 11, sub->topic, TWR_RADIO_MAX_BUFFER_SIZE - 3);

        twr_spirit1_set_tx_length(11 + strlen(sub->topic) + 1);

        twr_spirit1_tx();

        _twr_radio.transmit_count = _TWR_RADIO_TX_MAX_COUNT;

        _twr_radio.state = TWR_RADIO_STATE_TX;

        return;
    }

    uint8_t queue_item_buffer[sizeof(_twr_radio.pub_queue_buffer)];
    size_t queue_item_length;
    uint64_t id;

    while (twr_queue_get(&_twr_radio.rx_queue, queue_item_buffer, &queue_item_length))
    {
        twr_radio_id_from_buffer(queue_item_buffer, &id);

        queue_item_length -= TWR_RADIO_HEAD_SIZE;

        twr_radio_pub_decode(&id, queue_item_buffer + TWR_RADIO_HEAD_SIZE, queue_item_length);

        twr_radio_node_decode(&id, queue_item_buffer + TWR_RADIO_HEAD_SIZE, queue_item_length);

        if (queue_item_buffer[TWR_RADIO_HEAD_SIZE] == TWR_RADIO_HEADER_SUB_DATA)
        {
            uint8_t order = queue_item_buffer[TWR_RADIO_HEAD_SIZE + 1 + TWR_RADIO_ID_SIZE];

            if (order >= _twr_radio.subs_length)
            {
                return;
            }

            twr_radio_sub_t *sub = &_twr_radio.subs[order];

            if (sub->callback != NULL)
            {
                uint8_t *payload = NULL;

                if (queue_item_length > 1 + TWR_RADIO_ID_SIZE + 1)
                {
                    payload = queue_item_buffer + TWR_RADIO_HEAD_SIZE + 1 + TWR_RADIO_ID_SIZE + 1;
                }

                sub->callback(&id, sub->topic, payload, sub->param);
            }
        }
        else if (queue_item_buffer[TWR_RADIO_HEAD_SIZE] == TWR_RADIO_HEADER_PUB_INFO)
        {
            queue_item_buffer[queue_item_length + TWR_RADIO_HEAD_SIZE - 1] = 0;

            twr_radio_on_info(&id, (char *) queue_item_buffer + TWR_RADIO_HEAD_SIZE + 1, "", TWR_RADIO_MODE_UNKNOWN);
        }
        else if (queue_item_buffer[TWR_RADIO_HEAD_SIZE] == TWR_RADIO_HEADER_SUB_REG)
        {
            uint8_t *order = queue_item_buffer + TWR_RADIO_HEAD_SIZE + 1;

            twr_radio_sub_pt_t *pt = (twr_radio_sub_pt_t *) queue_item_buffer + TWR_RADIO_HEAD_SIZE + 2;

            char *topic = (char *) queue_item_buffer + TWR_RADIO_HEAD_SIZE + 3;

            twr_radio_on_sub(&id, order, pt, topic);
        }
    }

    if (twr_queue_get(&_twr_radio.pub_queue, queue_item_buffer, &queue_item_length))
    {
        uint8_t *buffer = twr_spirit1_get_tx_buffer();

        twr_radio_id_to_buffer(&_twr_radio.my_id, buffer);

        _twr_radio.message_id++;

        buffer[6] = _twr_radio.message_id;
        buffer[7] = _twr_radio.message_id >> 8;

        memcpy(buffer + 8, queue_item_buffer, queue_item_length);

        twr_spirit1_set_tx_length(8 + queue_item_length);

        twr_spirit1_tx();

        _twr_radio.transmit_count = _TWR_RADIO_TX_MAX_COUNT;

        _twr_radio.state = TWR_RADIO_STATE_TX;
    }
}

static bool _twr_radio_scan_cache_push(void)
{
    for (uint8_t i = 0; i < _twr_radio.scan_length; i++)
    {
        if (_twr_radio.scan_cache[i] == _twr_radio.peer_id)
        {
            return false;
        }
    }

    if (_twr_radio.scan_length < _TWR_RADIO_SCAN_CACHE_LENGTH)
    {
        _twr_radio.scan_length++;
    }

    _twr_radio.scan_cache[_twr_radio.scan_head++] = _twr_radio.peer_id;

    if (_twr_radio.scan_head == _TWR_RADIO_SCAN_CACHE_LENGTH)
    {
        _twr_radio.scan_head = 0;
    }

    return true;
}

static void _twr_radio_send_ack(void)
{
    uint8_t *tx_buffer = twr_spirit1_get_tx_buffer();

    if (_twr_radio.state == TWR_RADIO_STATE_TX_WAIT_ACK)
    {
        _twr_radio.ack_transmit_count = _twr_radio.transmit_count;

        _twr_radio.ack_tx_cache_length = twr_spirit1_get_tx_length();

        memcpy(_twr_radio.ack_tx_cache_buffer, tx_buffer, sizeof(_twr_radio.ack_tx_cache_buffer));

        _twr_radio.state = TWR_RADIO_STATE_TX_SEND_ACK;
    }
    else if (_twr_radio.state == TWR_RADIO_STATE_RX)
    {
        _twr_radio.state = TWR_RADIO_STATE_RX_SEND_ACK;
    }
    else
    {
        return;
    }

    uint8_t *rx_buffer = twr_spirit1_get_rx_buffer();

    memcpy(tx_buffer, rx_buffer, 8);

    tx_buffer[8] = TWR_RADIO_HEADER_ACK;

    twr_spirit1_set_tx_length(9);

    _twr_radio.transmit_count = 2;

    twr_spirit1_tx();
}

static void _twr_radio_go_to_state_rx_or_sleep(void)
{
    if (_twr_radio.state == TWR_RADIO_STATE_TX)
    {
        return;
    }
    
    if (_twr_radio.mode == TWR_RADIO_MODE_NODE_SLEEPING)
    {
        twr_tick_t now = twr_tick_get();

        if (_twr_radio.rx_timeout_sleeping > now)
        {
            _twr_radio.rx_timeout = _twr_radio.rx_timeout_sleeping;

            twr_spirit1_set_rx_timeout(_twr_radio.rx_timeout - now);

            twr_spirit1_rx();

            _twr_radio.state = TWR_RADIO_STATE_RX;
        }
        else
        {
            twr_spirit1_sleep();

            _twr_radio.state = TWR_RADIO_STATE_SLEEP;
        }
    }
    else
    {
        _twr_radio.rx_timeout = TWR_TICK_INFINITY;

        twr_spirit1_set_rx_timeout(TWR_TICK_INFINITY);

        twr_spirit1_rx();

        _twr_radio.state = TWR_RADIO_STATE_RX;
    }

    twr_scheduler_plan_now(_twr_radio.task_id);
}

static void _twr_radio_spirit1_event_handler(twr_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_SPIRIT1_EVENT_TX_DONE)
    {
        if (_twr_radio.transmit_count > 0)
        {
            _twr_radio.transmit_count--;
        }

        if (_twr_radio.state == TWR_RADIO_STATE_TX)
        {
            twr_tick_t timeout = _TWR_RADIO_ACK_TIMEOUT - 50 + rand() % _TWR_RADIO_ACK_TIMEOUT;

            _twr_radio.rx_timeout = twr_tick_get() + timeout;

            twr_spirit1_set_rx_timeout(timeout);

            twr_spirit1_rx();

            _twr_radio.state = TWR_RADIO_STATE_TX_WAIT_ACK;

            _twr_radio.ack = false;

            return;
        }

        if (_twr_radio.state == TWR_RADIO_STATE_RX_SEND_ACK)
        {
            if (_twr_radio.transmit_count > 0)
            {
                twr_spirit1_tx();

                return;
            }
        }

        if (_twr_radio.state == TWR_RADIO_STATE_TX_SEND_ACK)
        {
            if (_twr_radio.transmit_count > 0)
            {
                twr_spirit1_tx();

                return;
            }
            else
            {
                uint8_t *tx_buffer = twr_spirit1_get_tx_buffer();

                memcpy(tx_buffer, _twr_radio.ack_tx_cache_buffer, sizeof(_twr_radio.ack_tx_cache_buffer));

                twr_tick_t timeout = _TWR_RADIO_ACK_TIMEOUT - 50 + rand() % _TWR_RADIO_ACK_TIMEOUT;

                _twr_radio.rx_timeout = twr_tick_get() + timeout;

                _twr_radio.transmit_count = _twr_radio.ack_transmit_count;

                twr_spirit1_set_rx_timeout(timeout);

                twr_spirit1_set_tx_length(_twr_radio.ack_tx_cache_length);

                twr_spirit1_rx();

                _twr_radio.state = TWR_RADIO_STATE_TX_WAIT_ACK;

                return;
            }
        }

        _twr_radio_go_to_state_rx_or_sleep();
    }
    else if (event == TWR_SPIRIT1_EVENT_RX_TIMEOUT)
    {
        if (_twr_radio.state == TWR_RADIO_STATE_TX_WAIT_ACK)
        {
            if (_twr_radio.transmit_count > 0)
            {
                twr_spirit1_tx();

                _twr_radio.state = TWR_RADIO_STATE_TX;

                return;
            }
            else
            {
                if (_twr_radio.event_handler)
                {
                    _twr_radio.event_handler(TWR_RADIO_EVENT_TX_ERROR, _twr_radio.event_param);
                }
            }

        }

        _twr_radio_go_to_state_rx_or_sleep();
    }
    else if (event == TWR_SPIRIT1_EVENT_RX_DONE)
    {
        size_t length = twr_spirit1_get_rx_length();
        uint16_t message_id;

        if ((_twr_radio.rx_timeout != TWR_TICK_INFINITY) && (twr_tick_get() >= _twr_radio.rx_timeout))
        {
            if (_twr_radio.state == TWR_RADIO_STATE_TX_WAIT_ACK)
            {
                if (_twr_radio.transmit_count > 0)
                {
                    twr_spirit1_tx();

                    _twr_radio.state = TWR_RADIO_STATE_TX;

                    return;
                }
            }

            _twr_radio_go_to_state_rx_or_sleep();
        }

        if (length >= 9)
        {
            uint8_t *buffer = twr_spirit1_get_rx_buffer();
            twr_radio_peer_t *peer;

            twr_radio_id_from_buffer(buffer, &_twr_radio.peer_id);

            message_id = (uint16_t) buffer[6];
            message_id |= (uint16_t) buffer[7] << 8;

            // ACK check
            if (buffer[8] == TWR_RADIO_HEADER_ACK)
            {
                if (_twr_radio.state == TWR_RADIO_STATE_TX_WAIT_ACK)
                {
                    uint8_t *tx_buffer = twr_spirit1_get_tx_buffer();

                    if ((_twr_radio.peer_id == _twr_radio.my_id) && (_twr_radio.message_id == message_id) )
                    {
                        _twr_radio.transmit_count = 0;

                        _twr_radio.ack = true;

                        if (tx_buffer[8] == TWR_RADIO_HEADER_PAIRING)
                        {
                            if (length == 15)
                            {
                                twr_radio_id_from_buffer(buffer + 9, &_twr_radio.peer_id);

                                if ((_twr_radio.mode != TWR_RADIO_MODE_GATEWAY) && (_twr_radio.peer_devices[0].id != _twr_radio.peer_id))
                                {
                                    _twr_radio.peer_devices[0].id = _twr_radio.peer_id;
                                    _twr_radio.peer_devices[0].message_id_synced = false;
                                    _twr_radio.peer_devices_length = 1;

                                    _twr_radio.save_peer_devices = true;
                                    twr_scheduler_plan_now(_twr_radio.task_id);

                                    _twr_radio.sent_subs = 0;

                                    if (_twr_radio.event_handler)
                                    {
                                        _twr_radio.event_handler(TWR_RADIO_EVENT_PAIRED, _twr_radio.event_param);
                                    }
                                }
                            }
                        }
                        else if (tx_buffer[8] == TWR_RADIO_HEADER_SUB_REG)
                        {
                            _twr_radio.sent_subs++;
                        }
                        else if ((length == 10) && (buffer[9] == _TWR_RADIO_ACK_SUB_REQUEST))
                        {
                            _twr_radio.sent_subs = 0;
                        }

                        if (_twr_radio.sleeping_mode_rx_timeout != 0)
                        {
                            _twr_radio.rx_timeout_sleeping = twr_tick_get() + _twr_radio.sleeping_mode_rx_timeout;
                        }

                        _twr_radio_go_to_state_rx_or_sleep();

                        if (_twr_radio.event_handler)
                        {
                            _twr_radio.event_handler(TWR_RADIO_EVENT_TX_DONE, _twr_radio.event_param);
                        }
                    }

                }

                return;
            }

            if (buffer[8] == TWR_RADIO_HEADER_PAIRING)
            {
                if (_twr_radio.pairing_mode)
                {
                    if (length >= 9)
                    {
                        twr_radio_peer_device_add(_twr_radio.peer_id);
                    }
                }

                peer = twr_radio_get_peer_device(_twr_radio.peer_id);

                if (peer != NULL)
                {
                    _twr_radio_send_ack();

                    uint8_t *tx_buffer = twr_spirit1_get_tx_buffer();

                    twr_radio_id_to_buffer(&_twr_radio.my_id, tx_buffer + 9);

                    twr_spirit1_set_tx_length(15);

                    if ((length > 10) && (peer->message_id != message_id))
                    {
                        if (10 + (size_t) buffer[9] + 1 < length)
                        {
                            buffer[10 + buffer[9]] = 0;

                            peer->mode = buffer[length - 1];

                            buffer[length - 1] = 0;

                            twr_radio_on_info(&_twr_radio.peer_id, (char *)buffer + 10, (char *)buffer + 10 + buffer[9] + 1, peer->mode);
                        }
                    }

                    peer->message_id = message_id;

                    peer->message_id_synced = true;
                }

                return;
            }

            if ((length == 15) && ((buffer[8] == TWR_RADIO_HEADER_NODE_ATTACH) || (buffer[8] == TWR_RADIO_HEADER_NODE_DETACH)))
            {
                uint64_t id;

                twr_radio_id_from_buffer(buffer + 9, &id);

                if (id == _twr_radio.my_id)
                {
                    if (buffer[8] == TWR_RADIO_HEADER_NODE_ATTACH)
                    {
                        _twr_radio.pairing_request_to_gateway = true;

                        twr_scheduler_plan_now(_twr_radio.task_id);
                    }
                    else if (buffer[8] == TWR_RADIO_HEADER_NODE_DETACH)
                    {
                        _twr_radio_peer_device_remove(_twr_radio.peer_id);

                        if (_twr_radio.event_handler)
                        {
                            _twr_radio.event_handler(TWR_RADIO_EVENT_UNPAIRED, _twr_radio.event_param);
                        }
                    }
                }

                _twr_radio_send_ack();

                return;
            }

            peer = twr_radio_get_peer_device(_twr_radio.peer_id);

            if (peer != NULL)
            {
                if (peer->message_id != message_id)
                {
                    bool send_subs_request = _twr_radio.mode == TWR_RADIO_MODE_GATEWAY && (!peer->message_id_synced || (peer->message_id > message_id));

                    peer->message_id = message_id;

                    peer->message_id_synced = false;

                    if (length > 9)
                    {
                        if ((buffer[8] >= 0x15) && (buffer[8] <= 0x1d) && (length > 14))
                        {
                            uint64_t for_id;

                            twr_radio_id_from_buffer(buffer + 9, &for_id);

                            if (for_id != _twr_radio.my_id)
                            {
                                return;
                            }
                        }

                        twr_queue_put(&_twr_radio.rx_queue, buffer, length);

                        twr_scheduler_plan_now(_twr_radio.task_id);

                        peer->message_id_synced = true;

                        peer->rssi = twr_spirit1_get_rx_rssi();
                    }

                    if (peer->message_id_synced)
                    {
                        _twr_radio_send_ack();

                        if (send_subs_request)
                        {
                            uint8_t *tx_buffer = twr_spirit1_get_tx_buffer();

                            tx_buffer[9] = _TWR_RADIO_ACK_SUB_REQUEST;

                            twr_spirit1_set_tx_length(10);
                        }
                    }

                    return;
                }
            }
            else
            {
                if (_twr_radio.scan && (_twr_radio.event_handler != NULL) && _twr_radio_scan_cache_push())
                {
                    _twr_radio.event_handler(TWR_RADIO_EVENT_SCAN_FIND_DEVICE, _twr_radio.event_param);
                }

                if (_twr_radio.automatic_pairing)
                {
                    twr_radio_peer_device_add(_twr_radio.peer_id);
                }
            }
        }
    }
}

static void _twr_radio_load_peer_devices(void)
{
    uint32_t address = (uint32_t) twr_eeprom_get_size() - 8;
    uint64_t buffer[3];
    uint32_t *pointer = (uint32_t *)buffer;
    uint8_t length = 0;

    twr_eeprom_read(twr_eeprom_get_size() - 1, &length, 1);

    _twr_radio.peer_devices_length = 0;

    for (int i = 0; (i < length) && (i < TWR_RADIO_MAX_DEVICES); i++)
    {
        address -= sizeof(buffer);

        twr_eeprom_read(address, buffer, sizeof(buffer));

        pointer[2] = ~pointer[2];
        pointer[5] = ~pointer[5];

        if ((buffer[0] != buffer[1]) && (buffer[0] != buffer[2]))
        {
            if (buffer[1] == buffer[2])
            {
                buffer[0] = buffer[1];

                _twr_radio.save_peer_devices = true;

                twr_scheduler_plan_now(_twr_radio.task_id);
            }
            else
            {
                continue;
            }
        }

        if (buffer[0] != 0)
        {
            _twr_radio.peer_devices[_twr_radio.peer_devices_length].id = buffer[0];
            _twr_radio.peer_devices[_twr_radio.peer_devices_length].message_id_synced = false;
            _twr_radio.peer_devices_length++;
        }
    }
}

static void _twr_radio_save_peer_devices(void)
{
    uint32_t address = (uint32_t) twr_eeprom_get_size() - 8;
    uint64_t buffer_write[3];
    uint32_t *pointer_write = (uint32_t *)buffer_write;
    uint64_t buffer_read[3];

    _twr_radio.save_peer_devices = false;

    for (int i = 0; i < _twr_radio.peer_devices_length; i++)
    {
        buffer_write[0] = _twr_radio.peer_devices[i].id;
        buffer_write[1] = _twr_radio.peer_devices[i].id;
        buffer_write[2] = _twr_radio.peer_devices[i].id;

        pointer_write[2] = ~pointer_write[2];
        pointer_write[5] = ~pointer_write[5];

        address -= sizeof(buffer_write);

        twr_eeprom_read(address, buffer_read, sizeof(buffer_read));

        if (memcmp(buffer_read, buffer_write, sizeof(buffer_write)) != 0)
        {
            if (!twr_eeprom_write(address, buffer_write, sizeof(buffer_write)))
            {
                _twr_radio.save_peer_devices = true;

                twr_scheduler_plan_now(_twr_radio.task_id);

                return;
            }
        }
    }

    if (!twr_eeprom_write(twr_eeprom_get_size() - 1, &_twr_radio.peer_devices_length, 1))
    {
        _twr_radio.save_peer_devices = true;

        twr_scheduler_plan_now(_twr_radio.task_id);

        return;
    }
}

static void _twr_radio_atsha204_event_handler(twr_atsha204_t *self, twr_atsha204_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_ATSHA204_EVENT_SERIAL_NUMBER)
    {
        if (twr_atsha204_get_serial_number(self, &_twr_radio.my_id, sizeof(_twr_radio.my_id)))
        {
            if (_twr_radio.event_handler != NULL)
            {
                _twr_radio.event_handler(TWR_RADIO_EVENT_INIT_DONE, _twr_radio.event_param);
            }
        }
        else
        {
            if (_twr_radio.event_handler != NULL)
            {
                _twr_radio.event_handler(TWR_RADIO_EVENT_INIT_FAILURE, _twr_radio.event_param);
            }
        }
    }
    else if (event == TWR_ATSHA204_EVENT_ERROR)
    {
        if (_twr_radio.event_handler != NULL)
        {
            _twr_radio.event_handler(TWR_RADIO_EVENT_INIT_FAILURE, _twr_radio.event_param);
        }
    }
}

static bool _twr_radio_peer_device_add(uint64_t id)
{
    if (_twr_radio.peer_devices_length + 1 == TWR_RADIO_MAX_DEVICES)
    {
        if (_twr_radio.event_handler != NULL)
        {
            _twr_radio.peer_id = id;
            _twr_radio.event_handler(TWR_RADIO_EVENT_ATTACH_FAILURE, _twr_radio.event_param);
        }
        return false;
    }

    if (twr_radio_is_peer_device(id))
    {
        return false;
    }

    _twr_radio.peer_devices[_twr_radio.peer_devices_length].id = id;
    _twr_radio.peer_devices[_twr_radio.peer_devices_length].message_id_synced = false;
    _twr_radio.peer_devices_length++;

    _twr_radio.save_peer_devices = true;
    twr_scheduler_plan_now(_twr_radio.task_id);

    if (_twr_radio.event_handler != NULL)
    {
        _twr_radio.peer_id = id;
        _twr_radio.event_handler(TWR_RADIO_EVENT_ATTACH, _twr_radio.event_param);
    }

    return true;
}

static bool _twr_radio_peer_device_remove(uint64_t id)
{
    for (int i = 0; i < _twr_radio.peer_devices_length; i++)
    {
        if (id == _twr_radio.peer_devices[i].id)
        {
            _twr_radio.peer_devices_length--;
            _twr_radio.peer_devices[i].id = 0;

            if (i != _twr_radio.peer_devices_length)
            {
                memcpy(_twr_radio.peer_devices + i, _twr_radio.peer_devices + _twr_radio.peer_devices_length, sizeof(twr_radio_peer_t));
            }

            _twr_radio.save_peer_devices = true;
            twr_scheduler_plan_now(_twr_radio.task_id);

            if (_twr_radio.event_handler != NULL)
            {
                _twr_radio.peer_id = id;
                _twr_radio.event_handler(TWR_RADIO_EVENT_DETACH, _twr_radio.event_param);
            }

            return true;
        }
    }

    return false;
}

twr_radio_peer_t *twr_radio_get_peer_device(uint64_t id)
{
    for (int i = 0; i < _twr_radio.peer_devices_length; i++)
    {
        if (id == _twr_radio.peer_devices[i].id)
        {
            return &_twr_radio.peer_devices[i];
        }
    }

    return NULL;
}

uint8_t *twr_radio_id_to_buffer(uint64_t *id, uint8_t *buffer)
{
    buffer[0] = *id;
    buffer[1] = *id >> 8;
    buffer[2] = *id >> 16;
    buffer[3] = *id >> 24;
    buffer[4] = *id >> 32;
    buffer[5] = *id >> 40;

    return buffer + TWR_RADIO_ID_SIZE;
}

uint8_t *twr_radio_bool_to_buffer(bool *value, uint8_t *buffer)
{
    *buffer = value == NULL ? TWR_RADIO_NULL_BOOL : *value;

    return buffer + 1;
}

uint8_t *twr_radio_int_to_buffer(int *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = TWR_RADIO_NULL_INT;

        memcpy(buffer, &null, sizeof(int));
    }
    else
    {
        memcpy(buffer, value, sizeof(int));
    }

    return buffer + sizeof(int);
}

uint8_t *twr_radio_uint16_to_buffer(uint16_t *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = TWR_RADIO_NULL_UINT16;

        memcpy(buffer, &null, sizeof(uint16_t));
    }
    else
    {
        memcpy(buffer, value, sizeof(uint16_t));
    }

    return buffer + sizeof(uint16_t);
}

uint8_t *twr_radio_uint32_to_buffer(uint32_t *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = TWR_RADIO_NULL_UINT32;

        memcpy(buffer, &null, sizeof(uint32_t));
    }
    else
    {
        memcpy(buffer, value, sizeof(uint32_t));
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *twr_radio_float_to_buffer(float *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const float null = TWR_RADIO_NULL_FLOAT;

        memcpy(buffer, &null, sizeof(float));
    }
    else
    {
        memcpy(buffer, value, sizeof(float));
    }

    return buffer + sizeof(float);
}

uint8_t *twr_radio_data_to_buffer(void *data, size_t length, uint8_t *buffer)
{
    if (data == NULL)
    {
        return buffer + length;
    }

    memcpy(buffer, data, length);

    return buffer + length;
}

uint8_t *twr_radio_id_from_buffer(uint8_t *buffer, uint64_t *id)
{
    *id  = (uint64_t) buffer[0];
    *id |= (uint64_t) buffer[1] << 8;
    *id |= (uint64_t) buffer[2] << 16;
    *id |= (uint64_t) buffer[3] << 24;
    *id |= (uint64_t) buffer[4] << 32;
    *id |= (uint64_t) buffer[5] << 40;

    return buffer + TWR_RADIO_ID_SIZE;
}

uint8_t *twr_radio_bool_from_buffer(uint8_t *buffer, bool *value, bool **pointer)
{
    if (*buffer == TWR_RADIO_NULL_BOOL)
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

uint8_t *twr_radio_int_from_buffer(uint8_t *buffer, int *value, int **pointer)
{
    memcpy(value, buffer, sizeof(int));

    if (*value == TWR_RADIO_NULL_INT)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(int);
}

uint8_t *twr_radio_uint16_from_buffer(uint8_t *buffer, uint16_t *value, uint16_t **pointer)
{
    memcpy(value, buffer, sizeof(uint16_t));

    if (*value == TWR_RADIO_NULL_UINT16)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(uint16_t);
}

uint8_t *twr_radio_uint32_from_buffer(uint8_t *buffer, uint32_t *value, uint32_t **pointer)
{
    memcpy(value, buffer, sizeof(uint32_t));

    if (*value == TWR_RADIO_NULL_UINT32)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *twr_radio_float_from_buffer(uint8_t *buffer, float *value, float **pointer)
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

uint8_t *twr_radio_data_from_buffer(uint8_t *buffer, void *data, size_t length)
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
    twr_led_t *led;
    const char *firmware;
    const char *version;

} _twr_radio_button_event_param_t;

void _twr_radio_button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    _twr_radio_button_event_param_t *param = (_twr_radio_button_event_param_t*) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        if (param->led != NULL)
        {
            twr_led_pulse(param->led, 100);
        }

        static uint16_t event_count = 0;

        twr_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == TWR_BUTTON_EVENT_HOLD)
    {
        twr_radio_pairing_request(param->firmware, param->version);

        if (param->led != NULL)
        {
            twr_led_set_mode(param->led, TWR_LED_MODE_OFF);
            twr_led_pulse(param->led, 1000);
        }
    }
}

void twr_radio_init_pairing_button(const char *firmware, const char *version)
{
    static twr_led_t led;
    static twr_button_t button;
    static _twr_radio_button_event_param_t param;
    param.led = &led;
    param.firmware = firmware;
    param.version = version;

    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, 0);
    // Pass led instance as a callback parameter, so we don't need to add it to the radio structure
    twr_button_set_event_handler(&button, _twr_radio_button_event_handler, &param);

    twr_led_init(&led, TWR_GPIO_LED, false, 0);

}
