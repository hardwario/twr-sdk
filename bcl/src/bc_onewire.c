#include <stm32l0xx.h>
#include <bc_irq.h>
#include <bc_onewire.h>
#include <bc_system.h>
#include <bc_timer.h>

typedef struct
{
    bool initialized;

    uint8_t last_discrepancy;
    uint8_t last_family_discrepancy;
    bool last_device_flag;
    uint8_t last_rom_no[8];

    bool transaction;
    bc_gpio_channel_t channel;
    bool auto_ds28e17_sleep_mode;

} _bc_onewire_t;

static _bc_onewire_t _bc_onewire = { .initialized = false };

static bool _bc_onewire_reset(bc_gpio_channel_t channel);
static void _bc_onewire_write_byte(bc_gpio_channel_t channel, uint8_t byte);
static uint8_t _bc_onewire_read_byte(bc_gpio_channel_t channel);
static void _bc_onewire_write_bit(bc_gpio_channel_t channel, uint8_t bit);
static uint8_t _bc_onewire_read_bit(bc_gpio_channel_t channel);
static void _bc_onewire_start(void);
static void _bc_onewire_stop(void);
static void _bc_onewire_search_reset(void);
static void _bc_onewire_search_target_setup(uint8_t family_code);
static int _bc_onewire_search_devices(bc_gpio_channel_t channel, uint64_t *device_list, size_t device_list_size);

void bc_onewire_init(bc_gpio_channel_t channel)
{
    if (!_bc_onewire.initialized)
    {
        memset(&_bc_onewire, 0, sizeof(_bc_onewire));

        _bc_onewire.initialized = true;
    }

    bc_gpio_init(channel);

    bc_gpio_set_pull(channel, BC_GPIO_PULL_NONE);

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);
}

bool bc_onewire_transaction_start(bc_gpio_channel_t channel)
{
	if (_bc_onewire.transaction)
	{
        return false;
	}

    _bc_onewire_start();

    _bc_onewire.transaction = true;

    _bc_onewire.channel = channel;

	return true;
}

bool bc_onewire_transaction_stop(bc_gpio_channel_t channel)
{
	if (!_bc_onewire.transaction)
	{
		return false;
	}

    if (_bc_onewire.auto_ds28e17_sleep_mode)
    {
        if (_bc_onewire_reset(channel))
        {
            _bc_onewire_write_byte(channel, 0xcc);

            _bc_onewire_write_byte(channel, 0x1e);
        }
    }

	_bc_onewire.transaction = false;

	_bc_onewire_stop();

	return true;
}

bool bc_onewire_reset(bc_gpio_channel_t channel)
{
    bool state;

    _bc_onewire_start();

    state = _bc_onewire_reset(channel);

    _bc_onewire_stop();

    return state;
}

void bc_onewire_select(bc_gpio_channel_t channel, uint64_t *device_number)
{
    _bc_onewire_start();

    if (*device_number == BC_ONEWIRE_DEVICE_NUMBER_SKIP_ROM)
    {
        _bc_onewire_write_byte(channel, 0xCC);
    }
    else
    {
        _bc_onewire_write_byte(channel, 0x55);

        for (size_t i = 0; i < sizeof(uint64_t); i++)
        {
            _bc_onewire_write_byte(channel, ((uint8_t *) device_number)[i]);
        }
    }

    _bc_onewire_stop();
}

void bc_onewire_skip_rom(bc_gpio_channel_t channel)
{
    _bc_onewire_start();
    _bc_onewire_write_byte(channel, 0xCC);
    _bc_onewire_stop();
}

void bc_onewire_write(bc_gpio_channel_t channel, const void *buffer, size_t length)
{
    _bc_onewire_start();
    for (size_t i = 0; i < length; i++)
    {
        _bc_onewire_write_byte(channel, ((uint8_t *) buffer)[i]);
    }
    _bc_onewire_stop();
}

void bc_onewire_read(bc_gpio_channel_t channel, void *buffer, size_t length)
{
    _bc_onewire_start();
    for (size_t i = 0; i < length; i++)
    {
        ((uint8_t *) buffer)[i] = _bc_onewire_read_byte(channel);
    }
    _bc_onewire_stop();
}

void bc_onewire_write_8b(bc_gpio_channel_t channel, uint8_t byte)
{
    _bc_onewire_start();
    _bc_onewire_write_byte(channel, byte);
    _bc_onewire_stop();
}

uint8_t bc_onewire_read_8b(bc_gpio_channel_t channel)
{
    _bc_onewire_start();
    uint8_t data = _bc_onewire_read_byte(channel);
    _bc_onewire_stop();
    return data;
}

void bc_onewire_write_bit(bc_gpio_channel_t channel, int bit)
{
    _bc_onewire_start();
    _bc_onewire_write_bit(channel, bit & 0x01);
    _bc_onewire_stop();
}

int bc_onewire_read_bit(bc_gpio_channel_t channel)
{
    _bc_onewire_start();
    int bit = _bc_onewire_read_bit(channel);
    _bc_onewire_stop();
    return bit;
}

int bc_onewire_search_all(bc_gpio_channel_t channel, uint64_t *device_list, size_t device_list_size)
{
    _bc_onewire_search_reset();

    return _bc_onewire_search_devices(channel, device_list, device_list_size);
}

int bc_onewire_search_family(bc_gpio_channel_t channel, uint8_t family_code, uint64_t *device_list,
        size_t device_list_size)
{
    _bc_onewire_search_target_setup(family_code);

    return _bc_onewire_search_devices(channel, device_list, device_list_size);
}

uint8_t bc_onewire_crc8(const void *buffer, size_t length, uint8_t crc)
{
    uint8_t *_buffer = (uint8_t *) buffer;
    uint8_t inbyte;
    uint8_t i;

    while (length--)
    {
        inbyte = *_buffer++;
        for (i = 8; i; i--)
        {
            if ((crc ^ inbyte) & 0x01)
            {
                crc >>= 1;
                crc ^= 0x8C;
            }
            else
            {
                crc >>= 1;
            }
            inbyte >>= 1;
        }
    }

    return crc;
}

uint16_t bc_onewire_crc16(const void *buffer, size_t length, uint16_t crc)
{
    static const uint8_t oddparity[16] =
    { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

    uint16_t i;
    for (i = 0; i < length; i++)
    {
        uint16_t cdata = ((uint8_t *) buffer)[i];
        cdata = (cdata ^ crc) & 0xff;
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4]) crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }
    return crc;
}

static bool _bc_onewire_reset(bc_gpio_channel_t channel)
{
    int i;
    uint8_t retries = 125;

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

    do
    {
        if (retries-- == 0)
        {
            return false;
        }
        bc_timer_delay(2);
    }
    while (bc_gpio_get_input(channel) == 0);

    bc_gpio_set_output(channel, 0);
    bc_gpio_set_mode(channel, BC_GPIO_MODE_OUTPUT);

    bc_timer_delay(480);

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);
    bc_timer_delay(70);

    i = bc_gpio_get_input(channel);

    bc_timer_delay(410);

    return i == 0;
}

static void _bc_onewire_write_byte(bc_gpio_channel_t channel, uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        _bc_onewire_write_bit(channel, byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t _bc_onewire_read_byte(bc_gpio_channel_t channel)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        byte |= (_bc_onewire_read_bit(channel) << i);
    }
    return byte;
}

static void _bc_onewire_write_bit(bc_gpio_channel_t channel, uint8_t bit)
{
    bc_gpio_set_output(channel, 0);
    bc_gpio_set_mode(channel, BC_GPIO_MODE_OUTPUT);

    if (bit)
    {
        bc_irq_disable();

        bc_timer_delay(3);

        bc_irq_enable();

        bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

        bc_timer_delay(60);

    }
    else
    {
        bc_timer_delay(55);

        bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

        bc_timer_delay(8);
    }
}

static uint8_t _bc_onewire_read_bit(bc_gpio_channel_t channel)
{
    uint8_t bit = 0;

    bc_gpio_set_output(channel, 0);

    bc_gpio_set_mode(channel, BC_GPIO_MODE_OUTPUT);

    bc_irq_disable();

    bc_timer_delay(3);

    bc_irq_enable();

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

    bc_irq_disable();

    bc_timer_delay(8);

    bc_irq_enable();

    bit = bc_gpio_get_input(channel);

    bc_timer_delay(50);

    return bit;
}

static void _bc_onewire_start(void)
{
	if (_bc_onewire.transaction)
	{
		return;
	}

    bc_system_pll_enable();

    bc_timer_init();

    bc_timer_start();
}

static void _bc_onewire_stop(void)
{
	if (_bc_onewire.transaction)
	{
		return;
	}

    bc_timer_stop();

    bc_system_pll_disable();
}

static void _bc_onewire_search_reset(void)
{
    _bc_onewire.last_discrepancy = 0;
    _bc_onewire.last_device_flag = false;
    _bc_onewire.last_family_discrepancy = 0;
}

static void _bc_onewire_search_target_setup(uint8_t family_code)
{
    memset(_bc_onewire.last_rom_no, 0, sizeof(_bc_onewire.last_rom_no));
    _bc_onewire.last_rom_no[0] = family_code;
    _bc_onewire.last_discrepancy = 64;
    _bc_onewire.last_family_discrepancy = 0;
    _bc_onewire.last_device_flag = false;
}

void bc_onewire_search_start(uint8_t family_code)
{
    if (family_code != 0)
    {
        _bc_onewire_search_target_setup(family_code);
    }
    else
    {
        _bc_onewire_search_reset();
    }
}

bool bc_onewire_search_next(bc_gpio_channel_t channel, uint64_t *device_number)
{
    bool search_result = false;
    uint8_t id_bit_number;
    uint8_t last_zero, rom_byte_number;
    uint8_t id_bit, cmp_id_bit;
    uint8_t rom_byte_mask, search_direction;

    /* Initialize for search */
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;

    if (!_bc_onewire.last_device_flag)
    {

        _bc_onewire_start();

        if (!_bc_onewire_reset(channel))
        {
            _bc_onewire_search_reset();
            _bc_onewire_stop();
            return false;
        }

        // issue the search command
        _bc_onewire_write_byte(channel, 0xf0);

        // loop to do the search
        do
        {
            id_bit = _bc_onewire_read_bit(channel);
            cmp_id_bit = _bc_onewire_read_bit(channel);

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
            {
                break;
            }
            else
            {
                /* All devices coupled have 0 or 1 */
                if (id_bit != cmp_id_bit)
                {
                    /* Bit write value for search */
                    search_direction = id_bit;
                }
                else
                {
                    /* If this discrepancy is before the Last Discrepancy on a previous next then pick the same as last time */
                    if (id_bit_number < _bc_onewire.last_discrepancy)
                    {
                        search_direction = ((_bc_onewire.last_rom_no[rom_byte_number] & rom_byte_mask) > 0);
                    }
                    else
                    {
                        /* If equal to last pick 1, if not then pick 0 */
                        search_direction = (id_bit_number == _bc_onewire.last_discrepancy);
                    }

                    /* If 0 was picked then record its position in LastZero */
                    if (search_direction == 0)
                    {
                        last_zero = id_bit_number;

                        /* Check for Last discrepancy in family */
                        if (last_zero < 9)
                        {
                            _bc_onewire.last_family_discrepancy = last_zero;
                        }
                    }
                }

                /* Set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask */
                if (search_direction == 1)
                {
                    _bc_onewire.last_rom_no[rom_byte_number] |= rom_byte_mask;
                }
                else
                {
                    _bc_onewire.last_rom_no[rom_byte_number] &= ~rom_byte_mask;
                }

                /* Serial number search direction write bit */
                _bc_onewire_write_bit(channel, search_direction);

                /* Increment the byte counter id_bit_number and shift the mask rom_byte_mask */
                id_bit_number++;
                rom_byte_mask <<= 1;

                /* If the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask */
                if (rom_byte_mask == 0)
                {
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        }
        while (rom_byte_number < 8);

        /* If the search was successful then */
        if (!(id_bit_number < 65))
        {
            /* Search successful so set LastDiscrepancy, LastDeviceFlag, search_result */
            _bc_onewire.last_discrepancy = last_zero;

            /* Check for last device */
            if (_bc_onewire.last_discrepancy == 0)
            {
                _bc_onewire.last_device_flag = true;
            }

            search_result = !_bc_onewire.last_rom_no[0] ? false : true;
        }

        _bc_onewire_stop();
    }

    if (search_result
            && bc_onewire_crc8(_bc_onewire.last_rom_no, sizeof(_bc_onewire.last_rom_no), 0x00) != 0)
    {
        search_result = false;
    }

    if (search_result)
    {
        memcpy(device_number, _bc_onewire.last_rom_no, sizeof(_bc_onewire.last_rom_no));
    }
    else
    {
        _bc_onewire_search_reset();
    }

    return search_result;
}

void bc_onewire_auto_ds28e17_sleep_mode(bool on)
{
    _bc_onewire.auto_ds28e17_sleep_mode = on;
}

static int _bc_onewire_search_devices(bc_gpio_channel_t channel, uint64_t *device_list, size_t device_list_size)
{
    int devices = 0;
    int max_devices = device_list_size / sizeof(uint64_t);

    while ((devices < max_devices) && bc_onewire_search_next(channel, device_list))
    {
        device_list++;
        devices++;
    }

    return devices;
}
