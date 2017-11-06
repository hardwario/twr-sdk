#include <bc_radio.h>
#include <bc_queue.h>
#include <bc_atsha204.h>
#include <bc_scheduler.h>
#include <bc_spirit1.h>
#include <bc_eeprom.h>
#include <bc_i2c.h>
#include <math.h>

#define _BC_RADIO_SCAN_CACHE_LENGTH	4
#define _BC_RADIO_ID_SIZE           6
#define _BC_RADIO_HEAD_SIZE         (_BC_RADIO_ID_SIZE + 2)
#define _BC_RADIO_MAX_BUFFER_SIZE   (BC_SPIRIT1_MAX_PACKET_SIZE - _BC_RADIO_HEAD_SIZE)
#define _BC_RADIO_MAX_TOPIC_LEN     (_BC_RADIO_MAX_BUFFER_SIZE - 1 - 4 - 1)
#define _BC_RADIO_NULL_BOOL         0xff
#define _BC_RADIO_NULL_INT          INT32_MIN
#define _BC_RADIO_NULL_FLOAT        NAN
#define _BC_RADIO_ACK_TIMEOUT       100
#define _BC_RADIO_SLEEP_RX_TIMEOUT  100


typedef enum
{
    BC_RADIO_HEADER_PAIRING = 0,
    BC_RADIO_HEADER_ACK = 1,

    BC_RADIO_HEADER_NODE_ATTACH = 6,
    BC_RADIO_HEADER_NODE_DETACH = 7,

    BC_RADIO_HEADER_PUB_TOPIC_BOOL = 10,
    BC_RADIO_HEADER_PUB_TOPIC_INT = 11,
    BC_RADIO_HEADER_PUB_TOPIC_FLOAT = 12,
    BC_RADIO_HEADER_PUB_EVENT_COUNT = 13,
    BC_RADIO_HEADER_PUB_STATE = 14,
    BC_RADIO_HEADER_NODE_STATE_SET = 15,
    BC_RADIO_HEADER_NODE_STATE_GET = 16,
    BC_RADIO_HEADER_PUB_BUFFER = 17,

    BC_RADIO_HEADER_PUB_THERMOMETER = 80,
    BC_RADIO_HEADER_PUB_HUMIDITY = 81,
    BC_RADIO_HEADER_PUB_LUX_METER = 82,
    BC_RADIO_HEADER_PUB_BAROMETER = 83,
    BC_RADIO_HEADER_PUB_CO2 = 84,
	BC_RADIO_HEADER_PUB_BATTERY = 85,

} bc_radio_header_t;

typedef enum
{
    BC_RADIO_STATE_SLEEP = 0,
    BC_RADIO_STATE_TX = 1,
    BC_RADIO_STATE_RX = 2,
    BC_RADIO_STATE_TX_ACK = 3,
    BC_RADIO_STATE_RX_TIMEOUT = 4

} bc_radio_state_t;

typedef struct
{
    uint64_t id;
    uint16_t message_id;
    bool message_id_synced;

} bc_radio_peer_t;

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
    uint8_t pub_queue_buffer[128];
    uint8_t rx_queue_buffer[128];

    bc_radio_peer_t peer_devices[BC_RADIO_MAX_DEVICES];
    int peer_devices_lenght;

    uint64_t peer_id;

    bool listening;
    bool scan;
    uint64_t scan_cache[_BC_RADIO_SCAN_CACHE_LENGTH];
    uint8_t scan_length;
    uint8_t scan_head;

    bool automatic_pairing;
    bool save_peer_devices;

} _bc_radio;

static void _bc_radio_task(void *param);
static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param);
static void _bc_radio_load_old_peer_devices(void);
static void _bc_radio_load_peer_devices(void);
static void _bc_radio_save_peer_devices(void);
static void _bc_radio_atsha204_event_handler(bc_atsha204_t *self, bc_atsha204_event_t event, void *event_param);
static bool _bc_radio_peer_device_add(uint64_t id);
static bool _bc_radio_peer_device_remove(uint64_t id);
static uint8_t *_bc_radio_id_to_buffer(uint64_t *id, uint8_t *buffer);
static uint8_t *_bc_radio_bool_to_buffer(bool *value, uint8_t *buffer);
static uint8_t *_bc_radio_int_to_buffer(int *value, uint8_t *buffer);
static uint8_t *_bc_radio_float_to_buffer(float *value, uint8_t *buffer);
static uint8_t *_bc_radio_id_from_buffer(uint8_t *buffer, uint64_t *id);
static uint8_t *_bc_radio_bool_from_buffer(uint8_t *buffer, bool **value);
static uint8_t *_bc_radio_int_from_buffer(uint8_t *buffer, int **value);
static uint8_t *_bc_radio_float_from_buffer(uint8_t *buffer, float **value);

__attribute__((weak)) void bc_radio_on_event_count(uint64_t *id, uint8_t event_id, uint16_t *event_count) { (void) id; (void) event_id; (void) event_count; }
__attribute__((weak)) void bc_radio_on_push_button(uint64_t *id, uint16_t *event_count) { (void) id; (void) event_count; }
__attribute__((weak)) void bc_radio_on_thermometer(uint64_t *id, uint8_t *channel, float *temperature) { (void) id; (void) channel; (void) temperature; }
__attribute__((weak)) void bc_radio_on_humidity(uint64_t *id, uint8_t *channel, float *percentage) { (void) id; (void) channel; (void) percentage; }
__attribute__((weak)) void bc_radio_on_lux_meter(uint64_t *id, uint8_t *channel, float *illuminance) { (void) id; (void) channel; (void) illuminance; }
__attribute__((weak)) void bc_radio_on_barometer(uint64_t *id, uint8_t *channel, float *pressure, float *altitude) { (void) id; (void) channel; (void) pressure; (void) altitude; }
__attribute__((weak)) void bc_radio_on_co2(uint64_t *id, float *concentration) { (void) id; (void) concentration; }
__attribute__((weak)) void bc_radio_on_battery(uint64_t *id, float *voltage) { (void) id; (void) voltage; }
__attribute__((weak)) void bc_radio_on_buffer(uint64_t *id, void *buffer, size_t *length) { (void) id; (void) buffer; (void) length; }
__attribute__((weak)) void bc_radio_on_info(uint64_t *id, char *firmware, char *version) { (void) id; (void) firmware; (void) version; }
__attribute__((weak)) void bc_radio_on_state(uint64_t *id, uint8_t state_id, bool *state) { (void) id; (void) state_id; (void) state; }
__attribute__((weak)) void bc_radio_on_state_set(uint64_t *id, uint8_t state_id, bool *state) { (void) id; (void) state_id; (void) state; }
__attribute__((weak)) void bc_radio_on_state_get(uint64_t *id, uint8_t state_id) { (void) id; (void) state_id; }
__attribute__((weak)) void bc_radio_on_bool(uint64_t *id, char *subtopic, bool *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_on_int(uint64_t *id, char *subtopic, int *value) { (void) id; (void) subtopic; (void) value; }
__attribute__((weak)) void bc_radio_on_float(uint64_t *id, char *subtopic, float *value) { (void) id; (void) subtopic; (void) value; }

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

    if ((_bc_radio.mode == BC_RADIO_MODE_GATEWAY) || (_bc_radio.mode == BC_RADIO_MODE_NODE_LISTENING))
    {
        bc_radio_listen();
    }
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

    uint8_t buffer[1 + _BC_RADIO_ID_SIZE];

    buffer[0] = BC_RADIO_HEADER_NODE_ATTACH;

    _bc_radio_id_to_buffer(&id, buffer + 1);

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

    uint8_t buffer[1 + _BC_RADIO_ID_SIZE];

    buffer[0] = BC_RADIO_HEADER_NODE_DETACH;

    _bc_radio_id_to_buffer(&id, buffer + 1);

    bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer));

    bc_scheduler_plan_now(_bc_radio.task_id);

	return true;
}

bool bc_radio_peer_device_purge_all(void)
{
	for (int i = _bc_radio.peer_devices_lenght -1; i > -1 ; i--)
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
	for (i = 0; (i < _bc_radio.peer_devices_lenght) && (i < length); i++)
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
    for (int i = 0; i < _bc_radio.peer_devices_lenght; i++)
    {
        if (id == _bc_radio.peer_devices[i].id)
        {
            return true;
        }
    }
    return false;
}

bool bc_radio_pub_event_count(uint8_t event_id, uint16_t *event_count)
{
    uint8_t buffer[1 + sizeof(event_id) + sizeof(*event_count)];

    buffer[0] = BC_RADIO_HEADER_PUB_EVENT_COUNT;
    buffer[1] = event_id;

    memcpy(buffer + 2, event_count, sizeof(*event_count));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
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

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_humidity(uint8_t channel, float *percentage)
{
    uint8_t buffer[2 + sizeof(*percentage)];

    buffer[0] = BC_RADIO_HEADER_PUB_HUMIDITY;
    buffer[1] = channel;

    memcpy(&buffer[2], percentage, sizeof(*percentage));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_luminosity(uint8_t channel, float *lux)
{
    uint8_t buffer[2 + sizeof(*lux)];

    buffer[0] = BC_RADIO_HEADER_PUB_LUX_METER;
    buffer[1] = channel;

    memcpy(&buffer[2], lux, sizeof(*lux));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_barometer(uint8_t channel, float *pascal, float *meter)
{
    uint8_t buffer[2 + sizeof(*pascal) + sizeof(*meter)];

    buffer[0] = BC_RADIO_HEADER_PUB_BAROMETER;
    buffer[1] = channel;

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

bool bc_radio_pub_battery(float *voltage)
{
    uint8_t buffer[1 + sizeof(uint8_t) + sizeof(*voltage)];

    buffer[0] = BC_RADIO_HEADER_PUB_BATTERY;

    memcpy(&buffer[1], voltage, sizeof(*voltage));

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_buffer(void *buffer, size_t length)
{
    uint8_t qbuffer[_BC_RADIO_MAX_BUFFER_SIZE];

    if (length > sizeof(qbuffer) - 1)
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

bool bc_radio_pub_state(uint8_t state_id, bool *state)
{
    uint8_t buffer[1 + sizeof(state_id) + sizeof(*state)];

    buffer[0] = BC_RADIO_HEADER_PUB_STATE;
    buffer[1] = state_id;

    _bc_radio_bool_to_buffer(state, buffer + 2);

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_node_state_set(uint64_t *id, uint8_t state_id, bool *state)
{
    uint8_t buffer[1 + _BC_RADIO_ID_SIZE + sizeof(state_id) + sizeof(*state)];

    buffer[0] = BC_RADIO_HEADER_NODE_STATE_SET;

    uint8_t *pbuffer = _bc_radio_id_to_buffer(id, buffer + 1);

    pbuffer[0] = state_id;

    _bc_radio_bool_to_buffer(state, pbuffer + 1);

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_node_state_get(uint64_t *id, uint8_t state_id)
{
    uint8_t buffer[1 + _BC_RADIO_ID_SIZE + sizeof(state_id)];

    buffer[0] = BC_RADIO_HEADER_NODE_STATE_GET;

    uint8_t *pbuffer = _bc_radio_id_to_buffer(id, buffer + 1);

    pbuffer[0] = state_id;

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer)))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_bool(const char *subtopic, bool *value)
{
    uint8_t len = strlen(subtopic);

    if (len > _BC_RADIO_MAX_TOPIC_LEN)
    {
        return false;
    }

    uint8_t buffer[_BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_BOOL;

    _bc_radio_bool_to_buffer(value, buffer + 1);

    strcpy((char *)buffer + 2, subtopic);

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, len + 3))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_int(const char *subtopic, int *value)
{
    uint8_t len = strlen(subtopic);

    if (len > _BC_RADIO_MAX_TOPIC_LEN)
    {
        return false;
    }

    uint8_t buffer[_BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_INT;

    _bc_radio_int_to_buffer(value, buffer + 1);

    strcpy((char *)buffer + 5, subtopic);

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, len + 6))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
}

bool bc_radio_pub_float(const char *subtopic, float *value)
{
    uint8_t len = strlen(subtopic);

    if (len > _BC_RADIO_MAX_TOPIC_LEN)
    {
        return false;
    }

    uint8_t buffer[_BC_RADIO_MAX_BUFFER_SIZE];

    buffer[0] = BC_RADIO_HEADER_PUB_TOPIC_FLOAT;

    _bc_radio_float_to_buffer(value, buffer + 1);

    strcpy((char *)buffer + 5, subtopic);

    if (!bc_queue_put(&_bc_radio.pub_queue, buffer, len + 6))
    {
        return false;
    }

    bc_scheduler_plan_now(_bc_radio.task_id);

    return true;
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

    if (_bc_radio.state == BC_RADIO_STATE_TX_ACK)
    {
        bc_scheduler_plan_current_now();
        return;
    }

    if (_bc_radio.transmit_count != 0)
    {
        bc_spirit1_tx();

        return;
    }

    if (_bc_radio.state == BC_RADIO_STATE_TX)
    {
        _bc_radio.state = BC_RADIO_STATE_SLEEP;
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

        if (len > _BC_RADIO_MAX_BUFFER_SIZE - 4)
        {
            return;
        }

        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        _bc_radio_id_to_buffer(&_bc_radio.my_id, buffer);

        buffer[6] = _bc_radio.message_id;
        buffer[7] = _bc_radio.message_id >> 8;

        buffer[8] = BC_RADIO_HEADER_PAIRING;
        buffer[9] = len_firmware;

        strncpy((char *)buffer + 10, _bc_radio.firmware, _BC_RADIO_MAX_BUFFER_SIZE - 2);
        strncpy((char *)buffer + 10 + len_firmware + 1, _bc_radio.firmware_version, _BC_RADIO_MAX_BUFFER_SIZE - 2 - len_firmware - 1);

        _bc_radio.message_id++;

        bc_spirit1_set_tx_length(10 + len + 2);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 6;

        _bc_radio.state = BC_RADIO_STATE_TX;

        return;
    }

    uint8_t queue_item_buffer[sizeof(_bc_radio.pub_queue_buffer)];
    size_t queue_item_length;
    uint64_t id;
    uint8_t *pbuffer;

    while (bc_queue_get(&_bc_radio.rx_queue, queue_item_buffer, &queue_item_length))
    {
        _bc_radio_id_from_buffer(queue_item_buffer, &id);

        pbuffer = queue_item_buffer + _BC_RADIO_HEAD_SIZE;

        if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_EVENT_COUNT)
        {
            uint16_t event_count;

            memcpy(&event_count, pbuffer + 2, sizeof(event_count));

            if (pbuffer[1] == BC_RADIO_EVENT_PUSH_BUTTON)
            {
                bc_radio_on_push_button(&id, &event_count);
            }

            bc_radio_on_event_count(&id, pbuffer[1], &event_count);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_THERMOMETER)
        {
            float temperature;

            memcpy(&temperature, &queue_item_buffer[8 + 2], sizeof(temperature));

            bc_radio_on_thermometer(&id, &queue_item_buffer[8 + 1], &temperature);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_HUMIDITY)
        {
            float percentage;

            memcpy(&percentage, &queue_item_buffer[8 + 2], sizeof(percentage));

            bc_radio_on_humidity(&id, &queue_item_buffer[8 + 1], &percentage);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_LUX_METER)
        {
            float lux;

            memcpy(&lux, &queue_item_buffer[8 + 2], sizeof(lux));

            bc_radio_on_lux_meter(&id, &queue_item_buffer[8 + 1], &lux);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_BAROMETER)
        {
            float pascal;
            float meter;

            memcpy(&pascal, &queue_item_buffer[8 + 2], sizeof(pascal));
            memcpy(&meter, &queue_item_buffer[8 + 2 + sizeof(pascal)], sizeof(meter));

            bc_radio_on_barometer(&id, &queue_item_buffer[8 + 1], &pascal, &meter);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_CO2)
        {
            float concentration;

            memcpy(&concentration, &queue_item_buffer[8 + 1], sizeof(concentration));

            bc_radio_on_co2(&id, &concentration);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_BATTERY)
        {
            float voltage;

            memcpy(&voltage, &queue_item_buffer[8 + 1], sizeof(voltage));

            bc_radio_on_battery(&id, &voltage);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_BUFFER)
        {
            queue_item_length -= 8 + 1;
            bc_radio_on_buffer(&id, &queue_item_buffer[8 + 1], &queue_item_length);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_STATE)
        {
            bool *state = NULL;

            _bc_radio_bool_from_buffer(pbuffer + 2, &state);

            bc_radio_on_state(&id, pbuffer[1], state);
        }
        else if (pbuffer[0] == BC_RADIO_HEADER_NODE_STATE_SET)
        {
            uint64_t for_id;

            pbuffer = _bc_radio_id_from_buffer(pbuffer + 1, &for_id);

            if (for_id == _bc_radio.my_id)
            {
                bool *state = NULL;

                _bc_radio_bool_from_buffer(pbuffer + 1, &state);

                bc_radio_on_state_set(&for_id, pbuffer[0], state);
            }
        }
        else if (pbuffer[0] == BC_RADIO_HEADER_NODE_STATE_GET)
        {
            uint64_t for_id;

            pbuffer = _bc_radio_id_from_buffer(pbuffer + 1, &for_id);

            if (for_id == _bc_radio.my_id)
            {
                bc_radio_on_state_get(&for_id, pbuffer[0]);
            }
        }
        else if (pbuffer[0] == BC_RADIO_HEADER_PUB_TOPIC_BOOL)
        {
            bool *value = NULL;

            pbuffer = _bc_radio_bool_from_buffer(pbuffer + 1, &value);

            queue_item_buffer[queue_item_length - 1] = 0;

            bc_radio_on_bool(&id, (char *) pbuffer, value);
        }
        else if (pbuffer[0] == BC_RADIO_HEADER_PUB_TOPIC_INT)
        {
            int *value = NULL;

            pbuffer = _bc_radio_int_from_buffer(pbuffer + 1, &value);

            queue_item_buffer[queue_item_length - 1] = 0;

            bc_radio_on_int(&id, (char *) pbuffer, value);
        }
        else if (pbuffer[0] == BC_RADIO_HEADER_PUB_TOPIC_FLOAT)
        {
            float *value = NULL;

            pbuffer = _bc_radio_float_from_buffer(pbuffer + 1, &value);

            queue_item_buffer[queue_item_length - 1] = 0;

            bc_radio_on_float(&id, (char *) pbuffer, value);
        }
    }

    if (bc_queue_get(&_bc_radio.pub_queue, queue_item_buffer, &queue_item_length))
    {
        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        _bc_radio_id_to_buffer(&_bc_radio.my_id, buffer);

        buffer[6] = _bc_radio.message_id;
        buffer[7] = _bc_radio.message_id >> 8;

        _bc_radio.message_id++;

        memcpy(buffer + 8, queue_item_buffer, queue_item_length);

        bc_spirit1_set_tx_length(8 + queue_item_length);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 6;

        _bc_radio.state = BC_RADIO_STATE_TX;
    }

    if ((_bc_radio.state == BC_RADIO_STATE_SLEEP) && (_bc_radio.transmit_count == 0))
    {
        if (_bc_radio.listening)
        {
            bc_spirit1_set_rx_timeout(BC_TICK_INFINITY);
        }
        else
        {
            bc_spirit1_set_rx_timeout(_BC_RADIO_SLEEP_RX_TIMEOUT);
        }

        bc_spirit1_rx();

        _bc_radio.state = BC_RADIO_STATE_RX;
    }

    if (_bc_radio.state == BC_RADIO_STATE_RX_TIMEOUT)
    {
        _bc_radio.state = BC_RADIO_STATE_SLEEP;
    }

    if (_bc_radio.state == BC_RADIO_STATE_SLEEP)
    {
        bc_spirit1_sleep();
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
    uint8_t *rx_buffer = bc_spirit1_get_rx_buffer();
    uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

    memcpy(tx_buffer, rx_buffer, 8);

    tx_buffer[8] = BC_RADIO_HEADER_ACK;

    bc_spirit1_set_tx_length(9);

    _bc_radio.transmit_count = 2;

    bc_scheduler_plan_now(_bc_radio.task_id);
}

static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_SPIRIT1_EVENT_TX_DONE)
    {
        bc_scheduler_plan_now(_bc_radio.task_id);

        if (_bc_radio.transmit_count > 0)
        {
            _bc_radio.transmit_count--;
        }

        if (_bc_radio.state == BC_RADIO_STATE_TX)
        {
            _bc_radio.state = BC_RADIO_STATE_TX_ACK;

            bc_spirit1_set_rx_timeout(_BC_RADIO_ACK_TIMEOUT - 50 + rand() % _BC_RADIO_ACK_TIMEOUT);

            bc_spirit1_rx();

            return;
        }

        if (_bc_radio.listening)
        {
            bc_spirit1_set_rx_timeout(BC_TICK_INFINITY);

            bc_spirit1_rx();
        }
    }
    else if (event == BC_SPIRIT1_EVENT_RX_TIMEOUT)
    {
        if (_bc_radio.state == BC_RADIO_STATE_TX_ACK)
        {
            _bc_radio.state = BC_RADIO_STATE_TX;

            bc_scheduler_plan_now(_bc_radio.task_id);
        }
        else if ((_bc_radio.state == BC_RADIO_STATE_RX) && !_bc_radio.listening)
        {
            _bc_radio.state = BC_RADIO_STATE_RX_TIMEOUT;

            bc_scheduler_plan_now(_bc_radio.task_id);
        }
    }
    else if (event == BC_SPIRIT1_EVENT_RX_DONE)
    {
        size_t length = bc_spirit1_get_rx_length();

        if (length >= 9)
        {
            uint8_t *buffer = bc_spirit1_get_rx_buffer();

            _bc_radio_id_from_buffer(buffer, &_bc_radio.peer_id);

            // ACK check
            if ((_bc_radio.state == BC_RADIO_STATE_TX_ACK) && (buffer[8] == BC_RADIO_HEADER_ACK))
            {
                uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

                if ((_bc_radio.peer_id == _bc_radio.my_id) && (buffer[6] == tx_buffer[6]) && (buffer[7] == tx_buffer[7]))
                {
                    _bc_radio.state = BC_RADIO_STATE_TX;

                    _bc_radio.transmit_count = 0;

                    bc_scheduler_plan_now(_bc_radio.task_id);

                    if (_bc_radio.listening)
                    {
                        bc_spirit1_set_rx_timeout(BC_TICK_INFINITY);
                    }

                    if ((length == 15) && (tx_buffer[8] == BC_RADIO_HEADER_PAIRING))
                    {
                        _bc_radio_id_from_buffer(buffer + 9, &_bc_radio.peer_id);

                        _bc_radio_peer_device_add(_bc_radio.peer_id);

                        if (_bc_radio.event_handler)
                        {
                            _bc_radio.event_handler(BC_RADIO_EVENT_PAIRED, _bc_radio.event_param);
                        }
                    }
                }

                return;
            }

            if (buffer[8] == BC_RADIO_HEADER_PAIRING && length > 10)
            {
                if (_bc_radio.pairing_mode)
                {
                    if (10 + buffer[9] + 1 < length)
                    {
                        bc_radio_peer_device_add(_bc_radio.peer_id);
                    }
                }

                _bc_radio_send_ack();

                if (bc_radio_is_peer_device(_bc_radio.peer_id))
                {
                    uint8_t *tx_buffer = bc_spirit1_get_tx_buffer();

                    _bc_radio_id_to_buffer(&_bc_radio.my_id, tx_buffer + 9);

                    bc_spirit1_set_tx_length(15);

                    if (10 + buffer[9] + 1 < length)
                    {
                        buffer[10 + buffer[9]] = 0;
                        buffer[length - 1] = 0;

                        bc_radio_on_info(&_bc_radio.peer_id, (char *)buffer + 10, (char *)buffer + 10 + buffer[9] + 1);
                    }
                }

                return;
            }

            if ((length == 15) && ((buffer[8] == BC_RADIO_HEADER_NODE_ATTACH) || (buffer[8] == BC_RADIO_HEADER_NODE_DETACH)))
            {
            	uint64_t id;

            	_bc_radio_id_from_buffer(buffer + 9, &id);

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

            int i;
            for (i = 0; i < _bc_radio.peer_devices_lenght; i++)
            {
                if (_bc_radio.peer_id == _bc_radio.peer_devices[i].id)
                {
                    uint16_t message_id;

                    message_id = (uint16_t) buffer[6];
                    message_id |= (uint16_t) buffer[7] << 8;

                    if (_bc_radio.peer_devices[i].message_id != message_id || !_bc_radio.peer_devices[i].message_id_synced)
                    {
                        _bc_radio.peer_devices[i].message_id = message_id;

                        _bc_radio.peer_devices[i].message_id_synced = true;

                        if (length > 9)
                        {
                            bc_queue_put(&_bc_radio.rx_queue, buffer, length);

                            bc_scheduler_plan_now(_bc_radio.task_id);
                        }
                    }

                    _bc_radio_send_ack();

                    return;
                }
            }

            if (i == _bc_radio.peer_devices_lenght)
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

static void _bc_radio_load_old_peer_devices(void)
{
    uint64_t buffer[BC_RADIO_MAX_DEVICES + 1];

    bc_eeprom_read(0x00, buffer, sizeof(buffer));

    uint64_t checksum = buffer[BC_RADIO_MAX_DEVICES];

    for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
    {
        checksum ^= buffer[i];
    }

    if (checksum == 0)
    {
        for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
        {
            if (buffer[i] != 0)
            {
                _bc_radio.peer_devices[_bc_radio.peer_devices_lenght].id = buffer[i];
                _bc_radio.peer_devices[_bc_radio.peer_devices_lenght].message_id_synced = false;
                _bc_radio.peer_devices_lenght++;
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

    _bc_radio.peer_devices_lenght = 0;

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
            _bc_radio.peer_devices[_bc_radio.peer_devices_lenght].id = buffer[0];
            _bc_radio.peer_devices[_bc_radio.peer_devices_lenght].message_id_synced = false;
            _bc_radio.peer_devices_lenght++;
        }
    }

    if (_bc_radio.peer_devices_lenght == 0)
    {
        _bc_radio_load_old_peer_devices();

        _bc_radio.save_peer_devices = true;
    }
}

static void _bc_radio_save_peer_devices(void)
{
    uint32_t address = (uint32_t) bc_eeprom_get_size() - 8;
    uint64_t buffer_write[3];
    uint32_t *pointer_write = (uint32_t *)buffer_write;
    uint64_t buffer_read[3];

    for (int i = 0; i < _bc_radio.peer_devices_lenght; i++)
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

    if (!bc_eeprom_write(bc_eeprom_get_size() - 1, &_bc_radio.peer_devices_lenght, 1))
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
	if (_bc_radio.peer_devices_lenght + 1 == BC_RADIO_MAX_DEVICES)
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

	_bc_radio.peer_devices[_bc_radio.peer_devices_lenght].id = id;
	_bc_radio.peer_devices[_bc_radio.peer_devices_lenght].message_id_synced = false;
	_bc_radio.peer_devices_lenght++;

	_bc_radio_save_peer_devices();

	if (_bc_radio.event_handler != NULL)
	{
		_bc_radio.peer_id = id;
		_bc_radio.event_handler(BC_RADIO_EVENT_ATTACH, _bc_radio.event_param);
	}

	return true;
}

static bool _bc_radio_peer_device_remove(uint64_t id)
{
	for (int i = 0; i < _bc_radio.peer_devices_lenght; i++)
	{
		if (id == _bc_radio.peer_devices[i].id)
		{
		    _bc_radio.peer_devices_lenght--;

		    if (i != _bc_radio.peer_devices_lenght)
		    {
		        memcpy(_bc_radio.peer_devices + i, _bc_radio.peer_devices + _bc_radio.peer_devices_lenght, sizeof(bc_radio_peer_t));
		    }

		    _bc_radio_save_peer_devices();

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

static uint8_t *_bc_radio_id_to_buffer(uint64_t *id, uint8_t *buffer)
{
    buffer[0] = *id;
    buffer[1] = *id >> 8;
    buffer[2] = *id >> 16;
    buffer[3] = *id >> 24;
    buffer[4] = *id >> 32;
    buffer[5] = *id >> 40;

    return buffer + _BC_RADIO_ID_SIZE;
}

static uint8_t *_bc_radio_bool_to_buffer(bool *value, uint8_t *buffer)
{
    *buffer = value == NULL ? _BC_RADIO_NULL_BOOL : *value;

    return buffer + 1;
}

static uint8_t *_bc_radio_int_to_buffer(int *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const int null = _BC_RADIO_NULL_INT;

        memcpy(buffer, &null, sizeof(int));
    }
    else
    {
        memcpy(buffer, value, sizeof(int));
    }

    return buffer + sizeof(int);
}

static uint8_t *_bc_radio_float_to_buffer(float *value, uint8_t *buffer)
{
    if (value == NULL)
    {
        const float null = _BC_RADIO_NULL_FLOAT;

        memcpy(buffer, &null, sizeof(float));
    }
    else
    {
        memcpy(buffer, value, sizeof(float));
    }

    return buffer + sizeof(float);
}

static uint8_t *_bc_radio_id_from_buffer(uint8_t *buffer, uint64_t *id)
{
    *id  = (uint64_t) buffer[0];
    *id |= (uint64_t) buffer[1] << 8;
    *id |= (uint64_t) buffer[2] << 16;
    *id |= (uint64_t) buffer[3] << 24;
    *id |= (uint64_t) buffer[4] << 32;
    *id |= (uint64_t) buffer[5] << 40;

    return buffer + _BC_RADIO_ID_SIZE;
}

static uint8_t *_bc_radio_bool_from_buffer(uint8_t *buffer, bool **value)
{
    if (buffer[0] != _BC_RADIO_NULL_BOOL)
    {
        *value = (bool *) &buffer[0];
    }
    else
    {
        value = NULL;
    }

    return buffer + 1;
}

static uint8_t *_bc_radio_int_from_buffer(uint8_t *buffer, int **value)
{
    static int val;

    memcpy(&val, buffer, sizeof(int));

    if (val == _BC_RADIO_NULL_INT)
    {
        value = NULL;
    }
    else
    {
        *value = &val;
    }

    return buffer + sizeof(int);
}

static uint8_t *_bc_radio_float_from_buffer(uint8_t *buffer, float **value)
{
    static float val;

    memcpy(&val, buffer, sizeof(float));

    if (isnan(val))
    {
        value = NULL;
    }
    else
    {
        *value = &val;
    }

    return buffer + sizeof(float);
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
