#include <hio_radio.h>
#include <hio_queue.h>
#include <hio_atsha204.h>
#include <hio_scheduler.h>
#include <hio_eeprom.h>
#include <hio_i2c.h>
#include <hio_radio_pub.h>
#include <hio_radio_node.h>
#include <math.h>

#define _HIO_RADIO_SCAN_CACHE_LENGTH	4
#define _HIO_RADIO_ACK_TIMEOUT       100
#define _HIO_RADIO_SLEEP_RX_TIMEOUT  100
#define _HIO_RADIO_TX_MAX_COUNT      6
#define _HIO_RADIO_ACK_SUB_REQUEST   0x11

typedef enum
{
    HIO_RADIO_STATE_SLEEP = 0,
    HIO_RADIO_STATE_TX = 1,
    HIO_RADIO_STATE_RX = 2,
    HIO_RADIO_STATE_TX_WAIT_ACK = 3,
    HIO_RADIO_STATE_RX_SEND_ACK = 4,
    HIO_RADIO_STATE_TX_SEND_ACK = 5,

} hio_radio_state_t;

static struct
{
    hio_radio_mode_t mode;
    hio_atsha204_t atsha204;
    hio_radio_state_t state;
    uint64_t my_id;
    uint16_t message_id;
    int transmit_count;
    void (*event_handler)(hio_radio_event_t, void *);
    void *event_param;
    hio_scheduler_task_id_t task_id;
    bool pairing_request_to_gateway;
    const char *firmware;
    const char *firmware_version;
    bool pairing_mode;

    hio_queue_t pub_queue;
    hio_queue_t rx_queue;
    uint8_t pub_queue_buffer[HIO_RADIO_PUB_QUEUE_BUFFER_SIZE];
    uint8_t rx_queue_buffer[HIO_RADIO_RX_QUEUE_BUFFER_SIZE];

    uint8_t ack_tx_cache_buffer[15];
    size_t ack_tx_cache_length;
    int ack_transmit_count;
    hio_tick_t rx_timeout;
    bool ack;

    hio_radio_peer_t peer_devices[HIO_RADIO_MAX_DEVICES];
    int peer_devices_length;

    uint64_t peer_id;

    hio_tick_t sleeping_mode_rx_timeout;
    hio_tick_t rx_timeout_sleeping;

    bool scan;
    uint64_t scan_cache[_HIO_RADIO_SCAN_CACHE_LENGTH];
    uint8_t scan_length;
    uint8_t scan_head;

    bool automatic_pairing;
    bool save_peer_devices;

    hio_radio_sub_t *subs;
    int subs_length;
    int sent_subs;

} _hio_radio;

static void _hio_radio_task(void *param);
static void _hio_radio_go_to_state_rx_or_sleep(void);
static void _hio_radio_spirit1_event_handler(hio_spirit1_event_t event, void *event_param);
static void _hio_radio_load_peer_devices(void);
static void _hio_radio_save_peer_devices(void);
static void _hio_radio_atsha204_event_handler(hio_atsha204_t *self, hio_atsha204_event_t event, void *event_param);
static bool _hio_radio_peer_device_add(uint64_t id);
static bool _hio_radio_peer_device_remove(uint64_t id);

__attribute__((weak)) void hio_radio_on_info(uint64_t *id, char *firmware, char *version, hio_radio_mode_t mode) { (void) id; (void) firmware; (void) version; (void) mode;}
__attribute__((weak)) void hio_radio_on_sub(uint64_t *id, uint8_t *order, hio_radio_sub_pt_t *pt, char *topic) { (void) id; (void) order; (void) pt; (void) topic; }

void hio_radio_init(hio_radio_mode_t mode)
{
    memset(&_hio_radio, 0, sizeof(_hio_radio));

    _hio_radio.mode = mode;

    hio_atsha204_init(&_hio_radio.atsha204, HIO_I2C_I2C0, 0x64);
    hio_atsha204_set_event_handler(&_hio_radio.atsha204, _hio_radio_atsha204_event_handler, NULL);
    hio_atsha204_read_serial_number(&_hio_radio.atsha204);

    hio_queue_init(&_hio_radio.pub_queue, _hio_radio.pub_queue_buffer, sizeof(_hio_radio.pub_queue_buffer));
    hio_queue_init(&_hio_radio.rx_queue, _hio_radio.rx_queue_buffer, sizeof(_hio_radio.rx_queue_buffer));

    hio_spirit1_init();
    hio_spirit1_set_event_handler(_hio_radio_spirit1_event_handler, NULL);

    _hio_radio_load_peer_devices();

    _hio_radio.task_id = hio_scheduler_register(_hio_radio_task, NULL, HIO_TICK_INFINITY);

    _hio_radio_go_to_state_rx_or_sleep();
}

void hio_radio_set_event_handler(void (*event_handler)(hio_radio_event_t, void *), void *event_param)
{
    _hio_radio.event_handler = event_handler;
    _hio_radio.event_param = event_param;
}

void hio_radio_listen(hio_tick_t timeout)
{
    _hio_radio.rx_timeout_sleeping = hio_tick_get() + timeout;

    hio_scheduler_plan_now(_hio_radio.task_id);
}

void hio_radio_pairing_request(const char *firmware, const char *version)
{
    if ((_hio_radio.firmware != NULL) || (_hio_radio.firmware_version != NULL))
    {
        return;
    }

    _hio_radio.firmware = firmware;

    _hio_radio.firmware_version = version;

    _hio_radio.pairing_request_to_gateway = true;

    hio_scheduler_plan_now(_hio_radio.task_id);
}

void hio_radio_pairing_mode_start(void)
{
    _hio_radio.pairing_mode = true;
}

void hio_radio_pairing_mode_stop(void)
{
    _hio_radio.pairing_mode = false;
}

bool hio_radio_peer_device_add(uint64_t id)
{

    if (!_hio_radio_peer_device_add(id))
    {
        return false;
    }

    uint8_t buffer[1 + HIO_RADIO_ID_SIZE];

    buffer[0] = HIO_RADIO_HEADER_NODE_ATTACH;

    hio_radio_id_to_buffer(&id, buffer + 1);

    hio_queue_put(&_hio_radio.pub_queue, buffer, sizeof(buffer));

    hio_scheduler_plan_now(_hio_radio.task_id);

    return true;
}

bool hio_radio_peer_device_remove(uint64_t id)
{
    if (!_hio_radio_peer_device_remove(id))
    {
        return false;
    }

    uint8_t buffer[1 + HIO_RADIO_ID_SIZE];

    buffer[0] = HIO_RADIO_HEADER_NODE_DETACH;

    hio_radio_id_to_buffer(&id, buffer + 1);

    hio_queue_put(&_hio_radio.pub_queue, buffer, sizeof(buffer));

    hio_scheduler_plan_now(_hio_radio.task_id);

    return true;
}

bool hio_radio_peer_device_purge_all(void)
{
    for (int i = _hio_radio.peer_devices_length -1; i > -1 ; i--)
    {
        if (_hio_radio.peer_devices[i].id != 0)
        {
            if (!hio_radio_peer_device_remove(_hio_radio.peer_devices[i].id))
            {
                return false;
            }
        }
    }
    return true;
}

void hio_radio_get_peer_id(uint64_t *id, int length)
{
    int i;
    for (i = 0; (i < _hio_radio.peer_devices_length) && (i < length); i++)
    {
        id[i] = _hio_radio.peer_devices[i].id;
    }
    for (;i < length; i++)
    {
        id[i] = 0;
    }
}

void hio_radio_scan_start(void)
{
    memset(_hio_radio.scan_cache, 0x00, sizeof(_hio_radio.scan_cache));
    _hio_radio.scan_length = 0;
    _hio_radio.scan_head = 0;
    _hio_radio.scan = true;
}

void hio_radio_scan_stop(void)
{
    _hio_radio.scan = false;
}

void hio_radio_automatic_pairing_start(void)
{
    _hio_radio.automatic_pairing = true;
}

void hio_radio_automatic_pairing_stop(void)
{
    _hio_radio.automatic_pairing = false;
}

uint64_t hio_radio_get_my_id(void)
{
    return _hio_radio.my_id;
}

uint64_t hio_radio_get_event_id(void)
{
    return _hio_radio.peer_id;
}

bool hio_radio_is_peer_device(uint64_t id)
{
    for (int i = 0; i < _hio_radio.peer_devices_length; i++)
    {
        if (id == _hio_radio.peer_devices[i].id)
        {
            return true;
        }
    }
    return false;
}

bool hio_radio_pub_queue_put(const void *buffer, size_t length)
{
    if (!hio_queue_put(&_hio_radio.pub_queue, buffer, length))
    {
        return false;
    }

    hio_scheduler_plan_now(_hio_radio.task_id);

    return true;
}

void hio_radio_set_subs(hio_radio_sub_t *subs, int length)
{
    _hio_radio.subs = subs;

    _hio_radio.subs_length = length;

    _hio_radio.sent_subs = 0;
}

bool hio_radio_send_sub_data(uint64_t *id, uint8_t order, void *payload, size_t size)
{
    uint8_t qbuffer[1 + HIO_RADIO_ID_SIZE + HIO_RADIO_NODE_MAX_BUFFER_SIZE];

    if (size > HIO_RADIO_NODE_MAX_BUFFER_SIZE - 1)
    {
        return false;
    }

    qbuffer[0] = HIO_RADIO_HEADER_SUB_DATA;

    uint8_t *pqbuffer = hio_radio_id_to_buffer(id, qbuffer + 1);

    *pqbuffer++ = order;

    if (payload == NULL)
    {
        size = 0;
    }

    if (size > 0)
    {
        memcpy(pqbuffer, payload, size);
    }

    return hio_radio_pub_queue_put(qbuffer, 1 + HIO_RADIO_ID_SIZE + 1 + size);
}

void hio_radio_set_rx_timeout_for_sleeping_node(hio_tick_t timeout)
{
    _hio_radio.sleeping_mode_rx_timeout = timeout;
}

static void _hio_radio_task(void *param)
{
    (void) param;

    if (_hio_radio.my_id == 0)
    {
        hio_atsha204_read_serial_number(&_hio_radio.atsha204);

        hio_scheduler_plan_current_now();

        return;
    }

    if ((_hio_radio.state != HIO_RADIO_STATE_RX) && (_hio_radio.state != HIO_RADIO_STATE_SLEEP))
    {
        hio_scheduler_plan_current_now();

        return;
    }

    if (_hio_radio.save_peer_devices)
    {
        _hio_radio_save_peer_devices();
    }

    if (_hio_radio.pairing_request_to_gateway)
    {
        _hio_radio.pairing_request_to_gateway = false;

        size_t len_firmware = strlen(_hio_radio.firmware);

        size_t len = len_firmware + strlen(_hio_radio.firmware_version);

        if (len > HIO_RADIO_MAX_BUFFER_SIZE - 5)
        {
            return;
        }

        uint8_t *buffer = hio_spirit1_get_tx_buffer();

        hio_radio_id_to_buffer(&_hio_radio.my_id, buffer);

        _hio_radio.message_id++;

        buffer[6] = _hio_radio.message_id;
        buffer[7] = _hio_radio.message_id >> 8;

        buffer[8] = HIO_RADIO_HEADER_PAIRING;
        buffer[9] = len_firmware;

        strncpy((char *)buffer + 10, _hio_radio.firmware, HIO_RADIO_MAX_BUFFER_SIZE - 2);
        strncpy((char *)buffer + 10 + len_firmware + 1, _hio_radio.firmware_version, HIO_RADIO_MAX_BUFFER_SIZE - 2 - len_firmware - 1);

        buffer[10 + len + 1] = _hio_radio.mode;

        hio_spirit1_set_tx_length(10 + len + 2);

        hio_spirit1_tx();

        _hio_radio.transmit_count = _HIO_RADIO_TX_MAX_COUNT;

        _hio_radio.state = HIO_RADIO_STATE_TX;

        return;
    }

    if (_hio_radio.ack && (_hio_radio.sent_subs != _hio_radio.subs_length))
    {
        uint8_t *buffer = hio_spirit1_get_tx_buffer();

        hio_radio_sub_t *sub = &_hio_radio.subs[_hio_radio.sent_subs];

        hio_radio_id_to_buffer(&_hio_radio.my_id, buffer);

        _hio_radio.message_id++;

        buffer[6] = _hio_radio.message_id;
        buffer[7] = _hio_radio.message_id >> 8;

        buffer[8] = HIO_RADIO_HEADER_SUB_REG;

        buffer[9] = _hio_radio.sent_subs;

        buffer[10] = sub->type;

        strncpy((char *)buffer + 11, sub->topic, HIO_RADIO_MAX_BUFFER_SIZE - 3);

        hio_spirit1_set_tx_length(11 + strlen(sub->topic) + 1);

        hio_spirit1_tx();

        _hio_radio.transmit_count = _HIO_RADIO_TX_MAX_COUNT;

        _hio_radio.state = HIO_RADIO_STATE_TX;

        return;
    }

    uint8_t queue_item_buffer[sizeof(_hio_radio.pub_queue_buffer)];
    size_t queue_item_length;
    uint64_t id;

    while (hio_queue_get(&_hio_radio.rx_queue, queue_item_buffer, &queue_item_length))
    {
        hio_radio_id_from_buffer(queue_item_buffer, &id);

        queue_item_length -= HIO_RADIO_HEAD_SIZE;

        hio_radio_pub_decode(&id, queue_item_buffer + HIO_RADIO_HEAD_SIZE, queue_item_length);

        hio_radio_node_decode(&id, queue_item_buffer + HIO_RADIO_HEAD_SIZE, queue_item_length);

        if (queue_item_buffer[HIO_RADIO_HEAD_SIZE] == HIO_RADIO_HEADER_SUB_DATA)
        {
            uint8_t order = queue_item_buffer[HIO_RADIO_HEAD_SIZE + 1 + HIO_RADIO_ID_SIZE];

            if (order >= _hio_radio.subs_length)
            {
                return;
            }

            hio_radio_sub_t *sub = &_hio_radio.subs[order];

            if (sub->callback != NULL)
            {
                uint8_t *payload = NULL;

                if (queue_item_length > 1 + HIO_RADIO_ID_SIZE + 1)
                {
                    payload = queue_item_buffer + HIO_RADIO_HEAD_SIZE + 1 + HIO_RADIO_ID_SIZE + 1;
                }

                sub->callback(&id, sub->topic, payload, sub->param);
            }
        }
        else if (queue_item_buffer[HIO_RADIO_HEAD_SIZE] == HIO_RADIO_HEADER_PUB_INFO)
        {
            queue_item_buffer[queue_item_length + HIO_RADIO_HEAD_SIZE - 1] = 0;

            hio_radio_on_info(&id, (char *) queue_item_buffer + HIO_RADIO_HEAD_SIZE + 1, "", HIO_RADIO_MODE_UNKNOWN);
        }
        else if (queue_item_buffer[HIO_RADIO_HEAD_SIZE] == HIO_RADIO_HEADER_SUB_REG)
        {
            uint8_t *order = queue_item_buffer + HIO_RADIO_HEAD_SIZE + 1;

            hio_radio_sub_pt_t *pt = (hio_radio_sub_pt_t *) queue_item_buffer + HIO_RADIO_HEAD_SIZE + 2;

            char *topic = (char *) queue_item_buffer + HIO_RADIO_HEAD_SIZE + 3;

            hio_radio_on_sub(&id, order, pt, topic);
        }
    }

    if (hio_queue_get(&_hio_radio.pub_queue, queue_item_buffer, &queue_item_length))
    {
        uint8_t *buffer = hio_spirit1_get_tx_buffer();

        hio_radio_id_to_buffer(&_hio_radio.my_id, buffer);

        _hio_radio.message_id++;

        buffer[6] = _hio_radio.message_id;
        buffer[7] = _hio_radio.message_id >> 8;

        memcpy(buffer + 8, queue_item_buffer, queue_item_length);

        hio_spirit1_set_tx_length(8 + queue_item_length);

        hio_spirit1_tx();

        _hio_radio.transmit_count = _HIO_RADIO_TX_MAX_COUNT;

        _hio_radio.state = HIO_RADIO_STATE_TX;
    }
}

static bool _hio_radio_scan_cache_push(void)
{
    for (uint8_t i = 0; i < _hio_radio.scan_length; i++)
    {
        if (_hio_radio.scan_cache[i] == _hio_radio.peer_id)
        {
            return false;
        }
    }

    if (_hio_radio.scan_length < _HIO_RADIO_SCAN_CACHE_LENGTH)
    {
        _hio_radio.scan_length++;
    }

    _hio_radio.scan_cache[_hio_radio.scan_head++] = _hio_radio.peer_id;

    if (_hio_radio.scan_head == _HIO_RADIO_SCAN_CACHE_LENGTH)
    {
        _hio_radio.scan_head = 0;
    }

    return true;
}

static void _hio_radio_send_ack(void)
{
    uint8_t *tx_buffer = hio_spirit1_get_tx_buffer();

    if (_hio_radio.state == HIO_RADIO_STATE_TX_WAIT_ACK)
    {
        _hio_radio.ack_transmit_count = _hio_radio.transmit_count;

        _hio_radio.ack_tx_cache_length = hio_spirit1_get_tx_length();

        memcpy(_hio_radio.ack_tx_cache_buffer, tx_buffer, sizeof(_hio_radio.ack_tx_cache_buffer));

        _hio_radio.state = HIO_RADIO_STATE_TX_SEND_ACK;
    }
    else if (_hio_radio.state == HIO_RADIO_STATE_RX)
    {
        _hio_radio.state = HIO_RADIO_STATE_RX_SEND_ACK;
    }
    else
    {
        return;
    }

    uint8_t *rx_buffer = hio_spirit1_get_rx_buffer();

    memcpy(tx_buffer, rx_buffer, 8);

    tx_buffer[8] = HIO_RADIO_HEADER_ACK;

    hio_spirit1_set_tx_length(9);

    _hio_radio.transmit_count = 2;

    hio_spirit1_tx();
}

static void _hio_radio_go_to_state_rx_or_sleep(void)
{
    if (_hio_radio.mode == HIO_RADIO_MODE_NODE_SLEEPING)
    {
        hio_tick_t now = hio_tick_get();

        if (_hio_radio.rx_timeout_sleeping > now)
        {
            _hio_radio.rx_timeout = _hio_radio.rx_timeout_sleeping;

            hio_spirit1_set_rx_timeout(_hio_radio.rx_timeout - now);

            hio_spirit1_rx();

            _hio_radio.state = HIO_RADIO_STATE_RX;
        }
        else
        {
            hio_spirit1_sleep();

            _hio_radio.state = HIO_RADIO_STATE_SLEEP;
        }
    }
    else
    {
        _hio_radio.rx_timeout = HIO_TICK_INFINITY;

        hio_spirit1_set_rx_timeout(HIO_TICK_INFINITY);

        hio_spirit1_rx();

        _hio_radio.state = HIO_RADIO_STATE_RX;
    }

    hio_scheduler_plan_now(_hio_radio.task_id);
}

static void _hio_radio_spirit1_event_handler(hio_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_SPIRIT1_EVENT_TX_DONE)
    {
        if (_hio_radio.transmit_count > 0)
        {
            _hio_radio.transmit_count--;
        }

        if (_hio_radio.state == HIO_RADIO_STATE_TX)
        {
            hio_tick_t timeout = _HIO_RADIO_ACK_TIMEOUT - 50 + rand() % _HIO_RADIO_ACK_TIMEOUT;

            _hio_radio.rx_timeout = hio_tick_get() + timeout;

            hio_spirit1_set_rx_timeout(timeout);

            hio_spirit1_rx();

            _hio_radio.state = HIO_RADIO_STATE_TX_WAIT_ACK;

            _hio_radio.ack = false;

            return;
        }

        if (_hio_radio.state == HIO_RADIO_STATE_RX_SEND_ACK)
        {
            if (_hio_radio.transmit_count > 0)
            {
                hio_spirit1_tx();

                return;
            }
        }

        if (_hio_radio.state == HIO_RADIO_STATE_TX_SEND_ACK)
        {
            if (_hio_radio.transmit_count > 0)
            {
                hio_spirit1_tx();

                return;
            }
            else
            {
                uint8_t *tx_buffer = hio_spirit1_get_tx_buffer();

                memcpy(tx_buffer, _hio_radio.ack_tx_cache_buffer, sizeof(_hio_radio.ack_tx_cache_buffer));

                hio_tick_t timeout = _HIO_RADIO_ACK_TIMEOUT - 50 + rand() % _HIO_RADIO_ACK_TIMEOUT;

                _hio_radio.rx_timeout = hio_tick_get() + timeout;

                _hio_radio.transmit_count = _hio_radio.ack_transmit_count;

                hio_spirit1_set_rx_timeout(timeout);

                hio_spirit1_set_tx_length(_hio_radio.ack_tx_cache_length);

                hio_spirit1_rx();

                _hio_radio.state = HIO_RADIO_STATE_TX_WAIT_ACK;

                return;
            }
        }

        _hio_radio_go_to_state_rx_or_sleep();
    }
    else if (event == HIO_SPIRIT1_EVENT_RX_TIMEOUT)
    {
        if (_hio_radio.state == HIO_RADIO_STATE_TX_WAIT_ACK)
        {
            if (_hio_radio.transmit_count > 0)
            {
                hio_spirit1_tx();

                _hio_radio.state = HIO_RADIO_STATE_TX;

                return;
            }
            else
            {
                if (_hio_radio.event_handler)
                {
                    _hio_radio.event_handler(HIO_RADIO_EVENT_TX_ERROR, _hio_radio.event_param);
                }
            }

        }

        _hio_radio_go_to_state_rx_or_sleep();
    }
    else if (event == HIO_SPIRIT1_EVENT_RX_DONE)
    {
        size_t length = hio_spirit1_get_rx_length();
        uint16_t message_id;

        if ((_hio_radio.rx_timeout != HIO_TICK_INFINITY) && (hio_tick_get() >= _hio_radio.rx_timeout))
        {
            if (_hio_radio.state == HIO_RADIO_STATE_TX_WAIT_ACK)
            {
                if (_hio_radio.transmit_count > 0)
                {
                    hio_spirit1_tx();

                    _hio_radio.state = HIO_RADIO_STATE_TX;

                    return;
                }
            }

            _hio_radio_go_to_state_rx_or_sleep();
        }

        if (length >= 9)
        {
            uint8_t *buffer = hio_spirit1_get_rx_buffer();
            hio_radio_peer_t *peer;

            hio_radio_id_from_buffer(buffer, &_hio_radio.peer_id);

            message_id = (uint16_t) buffer[6];
            message_id |= (uint16_t) buffer[7] << 8;

            // ACK check
            if (buffer[8] == HIO_RADIO_HEADER_ACK)
            {
                if (_hio_radio.state == HIO_RADIO_STATE_TX_WAIT_ACK)
                {
                    uint8_t *tx_buffer = hio_spirit1_get_tx_buffer();

                    if ((_hio_radio.peer_id == _hio_radio.my_id) && (_hio_radio.message_id == message_id) )
                    {
                        _hio_radio.transmit_count = 0;

                        _hio_radio.ack = true;

                        if (tx_buffer[8] == HIO_RADIO_HEADER_PAIRING)
                        {
                            if (length == 15)
                            {
                                hio_radio_id_from_buffer(buffer + 9, &_hio_radio.peer_id);

                                if ((_hio_radio.mode != HIO_RADIO_MODE_GATEWAY) && (_hio_radio.peer_devices[0].id != _hio_radio.peer_id))
                                {
                                    _hio_radio.peer_devices[0].id = _hio_radio.peer_id;
                                    _hio_radio.peer_devices[0].message_id_synced = false;
                                    _hio_radio.peer_devices_length = 1;

                                    _hio_radio.save_peer_devices = true;
                                    hio_scheduler_plan_now(_hio_radio.task_id);

                                    _hio_radio.sent_subs = 0;

                                    if (_hio_radio.event_handler)
                                    {
                                        _hio_radio.event_handler(HIO_RADIO_EVENT_PAIRED, _hio_radio.event_param);
                                    }
                                }
                            }
                        }
                        else if (tx_buffer[8] == HIO_RADIO_HEADER_SUB_REG)
                        {
                            _hio_radio.sent_subs++;
                        }
                        else if ((length == 10) && (buffer[9] == _HIO_RADIO_ACK_SUB_REQUEST))
                        {
                            _hio_radio.sent_subs = 0;
                        }

                        if (_hio_radio.sleeping_mode_rx_timeout != 0)
                        {
                            _hio_radio.rx_timeout_sleeping = hio_tick_get() + _hio_radio.sleeping_mode_rx_timeout;
                        }

                        _hio_radio_go_to_state_rx_or_sleep();

                        if (_hio_radio.event_handler)
                        {
                            _hio_radio.event_handler(HIO_RADIO_EVENT_TX_DONE, _hio_radio.event_param);
                        }
                    }

                }

                return;
            }

            if (buffer[8] == HIO_RADIO_HEADER_PAIRING)
            {
                if (_hio_radio.pairing_mode)
                {
                    if (length >= 9)
                    {
                        hio_radio_peer_device_add(_hio_radio.peer_id);
                    }
                }

                peer = hio_radio_get_peer_device(_hio_radio.peer_id);

                if (peer != NULL)
                {
                    _hio_radio_send_ack();

                    uint8_t *tx_buffer = hio_spirit1_get_tx_buffer();

                    hio_radio_id_to_buffer(&_hio_radio.my_id, tx_buffer + 9);

                    hio_spirit1_set_tx_length(15);

                    if ((length > 10) && (peer->message_id != message_id))
                    {
                        if (10 + (size_t) buffer[9] + 1 < length)
                        {
                            buffer[10 + buffer[9]] = 0;

                            peer->mode = buffer[length - 1];

                            buffer[length - 1] = 0;

                            hio_radio_on_info(&_hio_radio.peer_id, (char *)buffer + 10, (char *)buffer + 10 + buffer[9] + 1, peer->mode);
                        }
                    }

                    peer->message_id = message_id;

                    peer->message_id_synced = true;
                }

                return;
            }

            if ((length == 15) && ((buffer[8] == HIO_RADIO_HEADER_NODE_ATTACH) || (buffer[8] == HIO_RADIO_HEADER_NODE_DETACH)))
            {
                uint64_t id;

                hio_radio_id_from_buffer(buffer + 9, &id);

                if (id == _hio_radio.my_id)
                {
                    if (buffer[8] == HIO_RADIO_HEADER_NODE_ATTACH)
                    {
                        _hio_radio.pairing_request_to_gateway = true;

                        hio_scheduler_plan_now(_hio_radio.task_id);
                    }
                    else if (buffer[8] == HIO_RADIO_HEADER_NODE_DETACH)
                    {
                        _hio_radio_peer_device_remove(_hio_radio.peer_id);

                        if (_hio_radio.event_handler)
                        {
                            _hio_radio.event_handler(HIO_RADIO_EVENT_UNPAIRED, _hio_radio.event_param);
                        }
                    }
                }

                _hio_radio_send_ack();

                return;
            }

            peer = hio_radio_get_peer_device(_hio_radio.peer_id);

            if (peer != NULL)
            {
                if (peer->message_id != message_id)
                {
                    bool send_subs_request = _hio_radio.mode == HIO_RADIO_MODE_GATEWAY && (!peer->message_id_synced || (peer->message_id > message_id));

                    peer->message_id = message_id;

                    peer->message_id_synced = false;

                    if (length > 9)
                    {
                        if ((buffer[8] >= 0x15) && (buffer[8] <= 0x1d) && (length > 14))
                        {
                            uint64_t for_id;

                            hio_radio_id_from_buffer(buffer + 9, &for_id);

                            if (for_id != _hio_radio.my_id)
                            {
                                return;
                            }
                        }

                        hio_queue_put(&_hio_radio.rx_queue, buffer, length);

                        hio_scheduler_plan_now(_hio_radio.task_id);

                        peer->message_id_synced = true;

                        peer->rssi = hio_spirit1_get_rx_rssi();
                    }

                    if (peer->message_id_synced)
                    {
                        _hio_radio_send_ack();

                        if (send_subs_request)
                        {
                            uint8_t *tx_buffer = hio_spirit1_get_tx_buffer();

                            tx_buffer[9] = _HIO_RADIO_ACK_SUB_REQUEST;

                            hio_spirit1_set_tx_length(10);
                        }
                    }

                    return;
                }
            }
            else
            {
                if (_hio_radio.scan && (_hio_radio.event_handler != NULL) && _hio_radio_scan_cache_push())
                {
                    _hio_radio.event_handler(HIO_RADIO_EVENT_SCAN_FIND_DEVICE, _hio_radio.event_param);
                }

                if (_hio_radio.automatic_pairing)
                {
                    hio_radio_peer_device_add(_hio_radio.peer_id);
                }
            }
        }
    }
}

static void _hio_radio_load_peer_devices(void)
{
    uint32_t address = (uint32_t) hio_eeprom_get_size() - 8;
    uint64_t buffer[3];
    uint32_t *pointer = (uint32_t *)buffer;
    uint8_t length = 0;

    hio_eeprom_read(hio_eeprom_get_size() - 1, &length, 1);

    _hio_radio.peer_devices_length = 0;

    for (int i = 0; (i < length) && (i < HIO_RADIO_MAX_DEVICES); i++)
    {
        address -= sizeof(buffer);

        hio_eeprom_read(address, buffer, sizeof(buffer));

        pointer[2] = ~pointer[2];
        pointer[5] = ~pointer[5];

        if ((buffer[0] != buffer[1]) && (buffer[0] != buffer[2]))
        {
            if (buffer[1] == buffer[2])
            {
                buffer[0] = buffer[1];

                _hio_radio.save_peer_devices = true;

                hio_scheduler_plan_now(_hio_radio.task_id);
            }
            else
            {
                continue;
            }
        }

        if (buffer[0] != 0)
        {
            _hio_radio.peer_devices[_hio_radio.peer_devices_length].id = buffer[0];
            _hio_radio.peer_devices[_hio_radio.peer_devices_length].message_id_synced = false;
            _hio_radio.peer_devices_length++;
        }
    }
}

static void _hio_radio_save_peer_devices(void)
{
    uint32_t address = (uint32_t) hio_eeprom_get_size() - 8;
    uint64_t buffer_write[3];
    uint32_t *pointer_write = (uint32_t *)buffer_write;
    uint64_t buffer_read[3];

    _hio_radio.save_peer_devices = false;

    for (int i = 0; i < _hio_radio.peer_devices_length; i++)
    {
        buffer_write[0] = _hio_radio.peer_devices[i].id;
        buffer_write[1] = _hio_radio.peer_devices[i].id;
        buffer_write[2] = _hio_radio.peer_devices[i].id;

        pointer_write[2] = ~pointer_write[2];
        pointer_write[5] = ~pointer_write[5];

        address -= sizeof(buffer_write);

        hio_eeprom_read(address, buffer_read, sizeof(buffer_read));

        if (memcmp(buffer_read, buffer_write, sizeof(buffer_write)) != 0)
        {
            if (!hio_eeprom_write(address, buffer_write, sizeof(buffer_write)))
            {
                _hio_radio.save_peer_devices = true;

                hio_scheduler_plan_now(_hio_radio.task_id);

                return;
            }
        }
    }

    if (!hio_eeprom_write(hio_eeprom_get_size() - 1, &_hio_radio.peer_devices_length, 1))
    {
        _hio_radio.save_peer_devices = true;

        hio_scheduler_plan_now(_hio_radio.task_id);

        return;
    }
}

static void _hio_radio_atsha204_event_handler(hio_atsha204_t *self, hio_atsha204_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_ATSHA204_EVENT_SERIAL_NUMBER)
    {
        if (hio_atsha204_get_serial_number(self, &_hio_radio.my_id, sizeof(_hio_radio.my_id)))
        {
            if (_hio_radio.event_handler != NULL)
            {
                _hio_radio.event_handler(HIO_RADIO_EVENT_INIT_DONE, _hio_radio.event_param);
            }
        }
        else
        {
            if (_hio_radio.event_handler != NULL)
            {
                _hio_radio.event_handler(HIO_RADIO_EVENT_INIT_FAILURE, _hio_radio.event_param);
            }
        }
    }
    else if (event == HIO_ATSHA204_EVENT_ERROR)
    {
        if (_hio_radio.event_handler != NULL)
        {
            _hio_radio.event_handler(HIO_RADIO_EVENT_INIT_FAILURE, _hio_radio.event_param);
        }
    }
}

static bool _hio_radio_peer_device_add(uint64_t id)
{
    if (_hio_radio.peer_devices_length + 1 == HIO_RADIO_MAX_DEVICES)
    {
        if (_hio_radio.event_handler != NULL)
        {
            _hio_radio.peer_id = id;
            _hio_radio.event_handler(HIO_RADIO_EVENT_ATTACH_FAILURE, _hio_radio.event_param);
        }
        return false;
    }

    if (hio_radio_is_peer_device(id))
    {
        return false;
    }

    _hio_radio.peer_devices[_hio_radio.peer_devices_length].id = id;
    _hio_radio.peer_devices[_hio_radio.peer_devices_length].message_id_synced = false;
    _hio_radio.peer_devices_length++;

    _hio_radio.save_peer_devices = true;
    hio_scheduler_plan_now(_hio_radio.task_id);

    if (_hio_radio.event_handler != NULL)
    {
        _hio_radio.peer_id = id;
        _hio_radio.event_handler(HIO_RADIO_EVENT_ATTACH, _hio_radio.event_param);
    }

    return true;
}

static bool _hio_radio_peer_device_remove(uint64_t id)
{
    for (int i = 0; i < _hio_radio.peer_devices_length; i++)
    {
        if (id == _hio_radio.peer_devices[i].id)
        {
            _hio_radio.peer_devices_length--;

            if (i != _hio_radio.peer_devices_length)
            {
                memcpy(_hio_radio.peer_devices + i, _hio_radio.peer_devices + _hio_radio.peer_devices_length, sizeof(hio_radio_peer_t));
            }

            _hio_radio.save_peer_devices = true;
            hio_scheduler_plan_now(_hio_radio.task_id);

            if (_hio_radio.event_handler != NULL)
            {
                _hio_radio.peer_id = id;
                _hio_radio.event_handler(HIO_RADIO_EVENT_DETACH, _hio_radio.event_param);
            }

            return true;
        }
    }

    return false;
}

hio_radio_peer_t *hio_radio_get_peer_device(uint64_t id)
{
    for (int i = 0; i < _hio_radio.peer_devices_length; i++)
    {
        if (id == _hio_radio.peer_devices[i].id)
        {
            return &_hio_radio.peer_devices[i];
        }
    }

    return NULL;
}

uint8_t *hio_radio_id_to_buffer(uint64_t *id, uint8_t *buffer)
{
    buffer[0] = *id;
    buffer[1] = *id >> 8;
    buffer[2] = *id >> 16;
    buffer[3] = *id >> 24;
    buffer[4] = *id >> 32;
    buffer[5] = *id >> 40;

    return buffer + HIO_RADIO_ID_SIZE;
}

uint8_t *hio_radio_bool_to_buffer(bool *value, uint8_t *buffer)
{
    *buffer = value == NULL ? HIO_RADIO_NULL_BOOL : *value;

    return buffer + 1;
}

uint8_t *hio_radio_int_to_buffer(int *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = HIO_RADIO_NULL_INT;

        memcpy(buffer, &null, sizeof(int));
    }
    else
    {
        memcpy(buffer, value, sizeof(int));
    }

    return buffer + sizeof(int);
}

uint8_t *hio_radio_uint16_to_buffer(uint16_t *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = HIO_RADIO_NULL_UINT16;

        memcpy(buffer, &null, sizeof(uint16_t));
    }
    else
    {
        memcpy(buffer, value, sizeof(uint16_t));
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *hio_radio_uint32_to_buffer(uint32_t *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = HIO_RADIO_NULL_UINT32;

        memcpy(buffer, &null, sizeof(uint32_t));
    }
    else
    {
        memcpy(buffer, value, sizeof(uint32_t));
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *hio_radio_float_to_buffer(float *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const float null = HIO_RADIO_NULL_FLOAT;

        memcpy(buffer, &null, sizeof(float));
    }
    else
    {
        memcpy(buffer, value, sizeof(float));
    }

    return buffer + sizeof(float);
}

uint8_t *hio_radio_data_to_buffer(void *data, size_t length, uint8_t *buffer)
{
    if (data == NULL)
    {
        return buffer + length;
    }

    memcpy(buffer, data, length);

    return buffer + length;
}

uint8_t *hio_radio_id_from_buffer(uint8_t *buffer, uint64_t *id)
{
    *id  = (uint64_t) buffer[0];
    *id |= (uint64_t) buffer[1] << 8;
    *id |= (uint64_t) buffer[2] << 16;
    *id |= (uint64_t) buffer[3] << 24;
    *id |= (uint64_t) buffer[4] << 32;
    *id |= (uint64_t) buffer[5] << 40;

    return buffer + HIO_RADIO_ID_SIZE;
}

uint8_t *hio_radio_bool_from_buffer(uint8_t *buffer, bool *value, bool **pointer)
{
    if (*buffer == HIO_RADIO_NULL_BOOL)
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

uint8_t *hio_radio_int_from_buffer(uint8_t *buffer, int *value, int **pointer)
{
    memcpy(value, buffer, sizeof(int));

    if (*value == HIO_RADIO_NULL_INT)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(int);
}

uint8_t *hio_radio_uint16_from_buffer(uint8_t *buffer, uint16_t *value, uint16_t **pointer)
{
    memcpy(value, buffer, sizeof(uint16_t));

    if (*value == HIO_RADIO_NULL_UINT16)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(uint16_t);
}

uint8_t *hio_radio_uint32_from_buffer(uint8_t *buffer, uint32_t *value, uint32_t **pointer)
{
    memcpy(value, buffer, sizeof(uint32_t));

    if (*value == HIO_RADIO_NULL_UINT32)
    {
        *pointer = NULL;
    }
    else
    {
        *pointer = value;
    }

    return buffer + sizeof(uint32_t);
}

uint8_t *hio_radio_float_from_buffer(uint8_t *buffer, float *value, float **pointer)
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

uint8_t *hio_radio_data_from_buffer(uint8_t *buffer, void *data, size_t length)
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
    hio_led_t *led;
    const char *firmware;
    const char *version;

} _hio_radio_button_event_param_t;

void _hio_radio_button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    _hio_radio_button_event_param_t *param = (_hio_radio_button_event_param_t*) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        if (param->led != NULL)
        {
            hio_led_pulse(param->led, 100);
        }

        static uint16_t event_count = 0;

        hio_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == HIO_BUTTON_EVENT_HOLD)
    {
        hio_radio_pairing_request(param->firmware, param->version);

        if (param->led != NULL)
        {
            hio_led_set_mode(param->led, HIO_LED_MODE_OFF);
            hio_led_pulse(param->led, 1000);
        }
    }
}

void hio_radio_init_pairing_button(const char *firmware, const char *version)
{
    static hio_led_t led;
    static hio_button_t button;
    static _hio_radio_button_event_param_t param;
    param.led = &led;
    param.firmware = firmware;
    param.version = version;

    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, 0);
    // Pass led instance as a callback parameter, so we don't need to add it to the radio structure
    hio_button_set_event_handler(&button, _hio_radio_button_event_handler, &param);

    hio_led_init(&led, HIO_GPIO_LED, false, 0);

}
