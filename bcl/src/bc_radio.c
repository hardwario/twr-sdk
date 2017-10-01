#include <bc_radio.h>
#include <bc_queue.h>
#include <bc_atsha204.h>
#include <bc_scheduler.h>
#include <bc_spirit1.h>
#include <bc_eeprom.h>
#include <bc_i2c.h>

#define _BC_RADIO_EEPROM_PEER_DEVICE_ADDRESS 0x00

typedef enum
{
    BC_RADIO_HEADER_ENROLL,
    BC_RADIO_HEADER_PUB_PUSH_BUTTON,
    BC_RADIO_HEADER_PUB_THERMOMETER,
    BC_RADIO_HEADER_PUB_HUMIDITY,
    BC_RADIO_HEADER_PUB_LUX_METER,
    BC_RADIO_HEADER_PUB_BAROMETER,
    BC_RADIO_HEADER_PUB_CO2,
    BC_RADIO_HEADER_PUB_BUFFER,
    BC_RADIO_HEADER_ATTACH,
	BC_RADIO_HEADER_DETACH,

} bc_radio_header_t;

typedef enum
{
    BC_RADIO_STATE_SLEEP = 0,
    BC_RADIO_STATE_TX = 1,
    BC_RADIO_STATE_RX = 2

} bc_radio_state_t;

typedef struct
{
    uint64_t address;
    uint16_t message_id;
    bool message_id_synced;

} bc_radio_peer_device_t;

static struct
{
    bc_atsha204_t atsha204;
    bc_radio_state_t state;
    uint64_t device_address;
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

    bc_radio_peer_device_t peer_devices[BC_RADIO_MAX_DEVICES];

    uint64_t peer_device_address;

    bool listening;

} _bc_radio;

static void _bc_radio_task(void *param);
static void _bc_radio_spirit1_event_handler(bc_spirit1_event_t event, void *event_param);
static void _bc_radio_save_peer_devices(void);
static void atsha204_event_handler(bc_atsha204_t *self, bc_atsha204_event_t event, void *event_param);
static bool _bc_radio_peer_device_add(uint64_t device_address);
static bool _bc_radio_peer_device_remove(uint64_t device_address);

__attribute__((weak)) void bc_radio_on_push_button(uint64_t *peer_device_address, uint16_t *event_count) { (void) peer_device_address; (void) event_count; }
__attribute__((weak)) void bc_radio_on_thermometer(uint64_t *peer_device_address, uint8_t *i2c, float *temperature) { (void) peer_device_address; (void) i2c; (void) temperature; }
__attribute__((weak)) void bc_radio_on_humidity(uint64_t *peer_device_address, uint8_t *i2c, float *percentage) { (void) peer_device_address; (void) i2c; (void) percentage; }
__attribute__((weak)) void bc_radio_on_lux_meter(uint64_t *peer_device_address, uint8_t *i2c, float *illuminance) { (void) peer_device_address; (void) i2c; (void) illuminance; }
__attribute__((weak)) void bc_radio_on_barometer(uint64_t *peer_device_address, uint8_t *i2c, float *pressure, float *altitude) { (void) peer_device_address; (void) i2c; (void) pressure; (void) altitude; }
__attribute__((weak)) void bc_radio_on_co2(uint64_t *peer_device_address, float *concentration) { (void) peer_device_address; (void) concentration; }
__attribute__((weak)) void bc_radio_on_buffer(uint64_t *peer_device_address, void *buffer, size_t *length) { (void) peer_device_address; (void) buffer; (void) length; }

void bc_radio_init(void)
{
    memset(&_bc_radio, 0, sizeof(_bc_radio));

    bc_atsha204_init(&_bc_radio.atsha204, BC_I2C_I2C0, 0x64);
    bc_atsha204_set_event_handler(&_bc_radio.atsha204, atsha204_event_handler, NULL);
    bc_atsha204_read_serial_number(&_bc_radio.atsha204);

    bc_queue_init(&_bc_radio.pub_queue, _bc_radio.pub_queue_buffer, sizeof(_bc_radio.pub_queue_buffer));
    bc_queue_init(&_bc_radio.rx_queue, _bc_radio.rx_queue_buffer, sizeof(_bc_radio.rx_queue_buffer));

    bc_spirit1_init();
    bc_spirit1_set_event_handler(_bc_radio_spirit1_event_handler, NULL);

    uint64_t buffer[BC_RADIO_MAX_DEVICES + 1];
    bc_eeprom_read(_BC_RADIO_EEPROM_PEER_DEVICE_ADDRESS, buffer, sizeof(buffer));

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
                _bc_radio.peer_devices[i].address = buffer[i];
                _bc_radio.peer_devices[i].message_id_synced = false;
            }
        }
    }

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

bool bc_radio_peer_device_add(uint64_t device_address)
{

	if (!_bc_radio_peer_device_add(device_address))
	{
		return false;
	}

    uint8_t buffer[1 + sizeof(_bc_radio.device_address)];

    buffer[0] = BC_RADIO_HEADER_ATTACH;

    memcpy(&buffer[1], &device_address, sizeof(device_address));

    bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer));

    bc_scheduler_plan_now(_bc_radio.task_id);

	return true;
}

bool bc_radio_peer_device_remove(uint64_t device_address)
{
	if (!_bc_radio_peer_device_remove(device_address))
	{
		return false;
	}

    uint8_t buffer[1 + sizeof(_bc_radio.device_address)];

    buffer[0] = BC_RADIO_HEADER_DETACH;

    memcpy(&buffer[1], &device_address, sizeof(device_address));

    bc_queue_put(&_bc_radio.pub_queue, buffer, sizeof(buffer));

    bc_scheduler_plan_now(_bc_radio.task_id);

	return true;
}

void bc_radio_get_peer_devices_address(uint64_t *device_address, int length)
{
	for (int i = 0; (i < BC_RADIO_MAX_DEVICES) && (i < length); i++)
	{
		device_address[i] = _bc_radio.peer_devices[i].address;
	}
}

uint64_t bc_radio_get_device_address(void)
{
    return _bc_radio.device_address;
}

uint64_t bc_radio_get_event_device_address(void)
{
    return _bc_radio.peer_device_address;
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
    uint8_t qbuffer[1 + BC_SPIRIT1_MAX_PACKET_SIZE - 8];

    if (length > (1 + BC_SPIRIT1_MAX_PACKET_SIZE - 8))
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

    if (_bc_radio.device_address == 0)
    {
        bc_atsha204_read_serial_number(&_bc_radio.atsha204);
        bc_scheduler_plan_current_now();
        return;
    }

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
        buffer[4] = _bc_radio.device_address >> 32;
        buffer[5] = _bc_radio.device_address >> 40;
        buffer[6] = _bc_radio.message_id;
        buffer[7] = _bc_radio.message_id >> 8;
        buffer[8] = BC_RADIO_HEADER_ENROLL;

        _bc_radio.message_id++;

        bc_spirit1_set_tx_length(9);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 10;

        _bc_radio.state = BC_RADIO_STATE_TX;

        return;
    }

    uint8_t queue_item_buffer[sizeof(_bc_radio.pub_queue_buffer)];
    size_t queue_item_length;
    uint64_t device_address;

    while (bc_queue_get(&_bc_radio.rx_queue, queue_item_buffer, &queue_item_length))
    {

        device_address  = (uint64_t) queue_item_buffer[0];
        device_address |= (uint64_t) queue_item_buffer[1] << 8;
        device_address |= (uint64_t) queue_item_buffer[2] << 16;
        device_address |= (uint64_t) queue_item_buffer[3] << 24;
        device_address |= (uint64_t) queue_item_buffer[4] << 32;
        device_address |= (uint64_t) queue_item_buffer[5] << 40;

        if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_PUSH_BUTTON)
        {
            uint16_t event_count;

            memcpy(&event_count, &queue_item_buffer[8 + 1], sizeof(event_count));

            bc_radio_on_push_button(&device_address, &event_count);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_THERMOMETER)
        {
            float temperature;

            memcpy(&temperature, &queue_item_buffer[8 + 2], sizeof(temperature));

            bc_radio_on_thermometer(&device_address, &queue_item_buffer[8 + 1], &temperature);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_HUMIDITY)
        {
            float percentage;

            memcpy(&percentage, &queue_item_buffer[8 + 2], sizeof(percentage));

            bc_radio_on_humidity(&device_address, &queue_item_buffer[8 + 1], &percentage);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_LUX_METER)
        {
            float lux;

            memcpy(&lux, &queue_item_buffer[8 + 2], sizeof(lux));

            bc_radio_on_lux_meter(&device_address, &queue_item_buffer[8 + 1], &lux);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_BAROMETER)
        {
            float pascal;
            float meter;

            memcpy(&pascal, &queue_item_buffer[8 + 2], sizeof(pascal));
            memcpy(&meter, &queue_item_buffer[8 + 2 + sizeof(pascal)], sizeof(meter));

            bc_radio_on_barometer(&device_address, &queue_item_buffer[8 + 1], &pascal, &meter);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_CO2)
        {
            float concentration;

            memcpy(&concentration, &queue_item_buffer[8 + 1], sizeof(concentration));

            bc_radio_on_co2(&device_address, &concentration);
        }
        else if (queue_item_buffer[8] == BC_RADIO_HEADER_PUB_BUFFER)
        {
            queue_item_length -= 8 + 1;
            bc_radio_on_buffer(&device_address, &queue_item_buffer[8 + 1], &queue_item_length);
        }

    }

    if (bc_queue_get(&_bc_radio.pub_queue, queue_item_buffer, &queue_item_length))
    {
        uint8_t *buffer = bc_spirit1_get_tx_buffer();

        buffer[0] = _bc_radio.device_address;
        buffer[1] = _bc_radio.device_address >> 8;
        buffer[2] = _bc_radio.device_address >> 16;
        buffer[3] = _bc_radio.device_address >> 24;
        buffer[4] = _bc_radio.device_address >> 32;
        buffer[5] = _bc_radio.device_address >> 40;
        buffer[6] = _bc_radio.message_id;
        buffer[7] = _bc_radio.message_id >> 8;

        _bc_radio.message_id++;

        memcpy(buffer + 8, queue_item_buffer, queue_item_length);

        bc_spirit1_set_tx_length(8 + queue_item_length);

        bc_spirit1_tx();

        _bc_radio.transmit_count = 6;

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

        if (length >= 9)
        {
            uint8_t *buffer = bc_spirit1_get_rx_buffer();

            _bc_radio.peer_device_address = (uint64_t) buffer[0];
            _bc_radio.peer_device_address |= (uint64_t) buffer[1] << 8;
            _bc_radio.peer_device_address |= (uint64_t) buffer[2] << 16;
            _bc_radio.peer_device_address |= (uint64_t) buffer[3] << 24;
            _bc_radio.peer_device_address |= (uint64_t) buffer[4] << 32;
            _bc_radio.peer_device_address |= (uint64_t) buffer[5] << 40;

            if (_bc_radio.enrollment_mode && length == 9 && buffer[8] == BC_RADIO_HEADER_ENROLL)
            {
                _bc_radio.enrollment_mode = false;

                if (!bc_radio_peer_device_add(_bc_radio.peer_device_address))
                {
                	bc_radio_peer_device_remove(_bc_radio.peer_device_address);
                }
                return;
            }

            if ((length == 17) && ((buffer[8] == BC_RADIO_HEADER_ATTACH) || (buffer[8] == BC_RADIO_HEADER_DETACH)))
            {
            	uint64_t address;
            	memcpy(&address, buffer + 9, sizeof(address));
            	if (address == _bc_radio.device_address)
            	{
            		if (buffer[8] == BC_RADIO_HEADER_ATTACH)
            		{
            			_bc_radio_peer_device_add(_bc_radio.peer_device_address);
            		}
            		else if (buffer[8] == BC_RADIO_HEADER_DETACH)
            		{
            			_bc_radio_peer_device_remove(_bc_radio.peer_device_address);
            		}
            	}
            	return;
            }

            for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
            {
                if (_bc_radio.peer_device_address == _bc_radio.peer_devices[i].address)
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
                }
            }
        }
    }
}

static void _bc_radio_save_peer_devices(void)
{
    uint64_t buffer[BC_RADIO_MAX_DEVICES + 1];
    buffer[BC_RADIO_MAX_DEVICES] = 0;

    for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
    {
        buffer[BC_RADIO_MAX_DEVICES] ^= _bc_radio.peer_devices[i].address;
        buffer[i] = _bc_radio.peer_devices[i].address;
    }

    bc_eeprom_write(_BC_RADIO_EEPROM_PEER_DEVICE_ADDRESS, buffer, sizeof(buffer));
}

static void atsha204_event_handler(bc_atsha204_t *self, bc_atsha204_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_ATSHA204_EVENT_SERIAL_NUMBER)
    {
        if (bc_atsha204_get_serial_number(self, &_bc_radio.device_address, sizeof(_bc_radio.device_address)))
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

static bool _bc_radio_peer_device_add(uint64_t device_address)
{
	int free_address_i = BC_RADIO_MAX_DEVICES;

	for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
	{
		if (device_address == _bc_radio.peer_devices[i].address)
		{
			return false;
		}

		if ((_bc_radio.peer_devices[i].address == 0) && (i < free_address_i))
		{
			free_address_i = i;
		}
	}

	if (free_address_i == BC_RADIO_MAX_DEVICES)
	{
		if (_bc_radio.event_handler != NULL)
		{
			_bc_radio.peer_device_address = device_address;
			_bc_radio.event_handler(BC_RADIO_EVENT_ATTACH_FAILURE, _bc_radio.event_param);
		}
		return false;
	}

	_bc_radio.peer_devices[free_address_i].address = device_address;
	_bc_radio.peer_devices[free_address_i].message_id_synced = false;

	_bc_radio_save_peer_devices();

	if (_bc_radio.event_handler != NULL)
	{
		_bc_radio.peer_device_address = device_address;
		_bc_radio.event_handler(BC_RADIO_EVENT_ATTACH, _bc_radio.event_param);
	}

	return true;
}

static bool _bc_radio_peer_device_remove(uint64_t device_address)
{
	for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
	{
		if (device_address == _bc_radio.peer_devices[i].address)
		{
			_bc_radio.peer_devices[i].address = 0;
			_bc_radio_save_peer_devices();

			if (_bc_radio.event_handler != NULL)
			{
				_bc_radio.peer_device_address = device_address;
				_bc_radio.event_handler(BC_RADIO_EVENT_DETACH, _bc_radio.event_param);
			}

			return true;
		}
	}

	return false;
}


void _bc_radio_button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;
    bc_led_t *led = (bc_led_t*)event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        if(led)
        {
            bc_led_pulse(led, 100);
        }

        static uint16_t event_count = 0;

        bc_radio_pub_push_button(&event_count);

        event_count++;
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_enroll_to_gateway();

        if(led)
        {
            bc_led_set_mode(led, BC_LED_MODE_OFF);
            bc_led_pulse(led, 1000);
        }
    }
}

void bc_radio_button_pair(bc_button_t *button, bc_led_t *led)
{
    // Init button and callback
    bc_button_init(button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    // Pass led instance as a callback parameter, so we don't need to add it to the radio structure
    bc_button_set_event_handler(button, _bc_radio_button_event_handler, led);

    // Parameter led can be NULL
    if(led)
    {
        bc_led_init(led, BC_GPIO_LED, false, false);
    }
}
