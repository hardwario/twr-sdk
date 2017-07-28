#include <stm32l0xx.h>
#include <bc_1wire.h>
#include <bc_module_core.h>

static struct
{
    uint8_t last_discrepancy;
    uint8_t last_family_discrepancy;
    bool last_device_flag;
    uint8_t last_rom_no[8];

} _bc_1wire_search;

static bool _bc_1wire_reset(bc_gpio_channel_t channel);
static void _bc_1wire_write_byte(bc_gpio_channel_t channel, uint8_t byte);
static uint8_t _bc_1wire_read_byte(bc_gpio_channel_t channel);
static void _bc_1wire_write_bit(bc_gpio_channel_t channel, uint8_t bit);
static uint8_t _bc_1wire_read_bit(bc_gpio_channel_t channel);
static void _bc_1wire_start(void);
static void _bc_1wire_stop(void);
static void _bc_1wire_delay(uint32_t micro_second);
static void _bc_1wire_search_reset(void);
static void _bc_1wire_search_target_setup(uint8_t family_code);
static bool _bc_1wire_search_next(bc_gpio_channel_t channel, uint64_t *device_number);
static int _bc_1wire_search_devices(bc_gpio_channel_t channel, uint64_t *device_numbers, size_t length);

void bc_1wire_init(bc_gpio_channel_t channel)
{
    bc_gpio_init(channel);
    bc_gpio_set_pull(channel, BC_GPIO_PULL_NONE);
}

bool bc_1wire_reset(bc_gpio_channel_t channel)
{
    bool state;

    _bc_1wire_start();

    state = _bc_1wire_reset(channel);

    _bc_1wire_stop();

    return state;
}

void bc_1wire_select(bc_gpio_channel_t channel, uint64_t *device_number)
{
    _bc_1wire_start();

    if (*device_number == BC_1WIRE_NO_DEVICE_NUMBER) // Skip ROM
    {
        _bc_1wire_write_byte(channel, 0xCC);
    }
    else
    {
        _bc_1wire_write_byte(channel, 0x55);

        for (size_t i = 0; i < sizeof(u_int64_t); i++)
        {
            _bc_1wire_write_byte(channel, ((uint8_t *) device_number)[i]);
        }
    }

    _bc_1wire_stop();
}

void bc_1wire_write(bc_gpio_channel_t channel,const void *buffer, size_t length)
{
    _bc_1wire_start();
    for (size_t i = 0; i < length; i++)
    {
        _bc_1wire_write_byte(channel, ((uint8_t *) buffer)[i]);
    }
    _bc_1wire_stop();
}

void bc_1wire_read(bc_gpio_channel_t channel, void *buffer, size_t length)
{
    _bc_1wire_start();
    for (size_t i = 0; i < length; i++)
    {
        ((uint8_t *) buffer)[i] = _bc_1wire_read_byte(channel);
    }
    _bc_1wire_stop();
}

void bc_1wire_write_8b(bc_gpio_channel_t channel, uint8_t byte)
{
    _bc_1wire_start();
    _bc_1wire_write_byte(channel, byte);
    _bc_1wire_stop();
}

uint8_t bc_1wire_read_8b(bc_gpio_channel_t channel)
{
    _bc_1wire_start();
    uint8_t data = _bc_1wire_read_byte(channel);
    _bc_1wire_stop();
    return data;
}

void bc_1wire_write_bit(bc_gpio_channel_t channel, uint8_t data)
{
    _bc_1wire_start();
    _bc_1wire_write_bit(channel, data & 0x01);
    _bc_1wire_stop();
}

uint8_t bc_1wire_read_bit(bc_gpio_channel_t channel)
{
    _bc_1wire_start();
    uint8_t bit = _bc_1wire_read_bit(channel);
    _bc_1wire_stop();
    return bit;
}

int bc_1wire_search(bc_gpio_channel_t channel, uint64_t *device_numbers, size_t length)
{
    _bc_1wire_search_reset();

    return _bc_1wire_search_devices(channel, device_numbers, length);
}

int bc_1wire_search_target(bc_gpio_channel_t channel, uint8_t family_code, uint64_t *device_numbers, size_t length)
{
    _bc_1wire_search_target_setup(family_code);

    return _bc_1wire_search_devices(channel, device_numbers, length);
}

uint8_t bc_1wire_crc8(const void *buffer, size_t length, uint8_t crc)
{
    uint8_t inbyte;
    uint8_t i;

    while (length--)
    {
        inbyte = *((uint8_t *) buffer++);
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

uint16_t bc_1wire_crc16(const void *buffer, size_t length, uint16_t crc)
{
    static const uint8_t oddparity[16] =
    { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

    uint16_t i;
    for (i = 0; i < length; i++)
    {
        uint16_t cdata = ((uint8_t *) buffer)[i];
        cdata = (cdata ^ crc) & 0xff;
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }
    return crc;
}

static bool _bc_1wire_reset(bc_gpio_channel_t channel)
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
        _bc_1wire_delay(2);
    } while (bc_gpio_get_input(channel) == 0);

    bc_gpio_set_output(channel, 0);
    bc_gpio_set_mode(channel, BC_GPIO_MODE_OUTPUT);

    _bc_1wire_delay(480);

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);
    _bc_1wire_delay(70);

    i = bc_gpio_get_input(channel);

    _bc_1wire_delay(410);

    return i == 0;
}

static void _bc_1wire_write_byte(bc_gpio_channel_t channel, uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        _bc_1wire_write_bit(channel, byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t _bc_1wire_read_byte(bc_gpio_channel_t channel)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        byte |= (_bc_1wire_read_bit(channel) << i);
    }
    return byte;
}

static void _bc_1wire_write_bit(bc_gpio_channel_t channel, uint8_t bit)
{
    bc_gpio_set_output(channel, 0);
    bc_gpio_set_mode(channel, BC_GPIO_MODE_OUTPUT);

    if (bit)
    {
        _bc_1wire_delay(3);

        bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

        _bc_1wire_delay(60);

    }
    else
    {

        _bc_1wire_delay(55);

        bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

        _bc_1wire_delay(8);
    }
}

static uint8_t _bc_1wire_read_bit(bc_gpio_channel_t channel)
{
    uint8_t bit = 0;

    bc_gpio_set_output(channel, 0);
    bc_gpio_set_mode(channel, BC_GPIO_MODE_OUTPUT);

    _bc_1wire_delay(3);

    bc_gpio_set_mode(channel, BC_GPIO_MODE_INPUT);

    _bc_1wire_delay(8);

    bit = bc_gpio_get_input(channel);

    _bc_1wire_delay(50);

    return bit;
}

static void _bc_1wire_start(void)
{
    bc_module_core_pll_enable();

    // Enable clock for TIM6
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    // Enable one-pulse mode
    TIM6->CR1 &= ~TIM_CR1_OPM;

    // Disable counter
    TIM6->CR1 &= ~TIM_CR1_CEN;

    // Set prescaler
    TIM6->PSC = 32;

    // Set auto-reload register
    TIM6->ARR = 0xffffffff;

    // Generate update of registers
    TIM6->EGR = TIM_EGR_UG;

    // Enable counter
    TIM6->CR1 |= TIM_CR1_CEN;
}

static void _bc_1wire_stop(void)
{
    TIM6->CR1 &= ~TIM_CR1_CEN;

    RCC->APB1ENR &= ~RCC_APB1ENR_TIM6EN;

    bc_module_core_pll_disable();
}

static void _bc_1wire_delay(uint32_t micro_second)
{
    uint32_t stop = TIM6->CNT + micro_second - 1;

    while (TIM6->CNT < stop)
    {
        continue;
    }
}

static void _bc_1wire_search_reset(void)
{
    _bc_1wire_search.last_discrepancy = 0;
    _bc_1wire_search.last_device_flag = false;
    _bc_1wire_search.last_family_discrepancy = 0;
}

static void _bc_1wire_search_target_setup(uint8_t family_code)
{
    memset(_bc_1wire_search.last_rom_no, 0, sizeof(_bc_1wire_search.last_rom_no));
    _bc_1wire_search.last_rom_no[0] = family_code;
    _bc_1wire_search.last_discrepancy = 64;
    _bc_1wire_search.last_family_discrepancy = 0;
    _bc_1wire_search.last_device_flag = false;
}

static bool _bc_1wire_search_next(bc_gpio_channel_t channel, uint64_t *device_number)
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

    if (!_bc_1wire_search.last_device_flag)
    {

        _bc_1wire_start();

        if (!_bc_1wire_reset(channel))
        {
            _bc_1wire_search_reset();
            _bc_1wire_stop();
            return false;
        }

        // issue the search command
        _bc_1wire_write_byte(channel, 0xf0);

        // loop to do the search
        do
        {
            id_bit = _bc_1wire_read_bit(channel);
            cmp_id_bit = _bc_1wire_read_bit(channel);

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
                    if (id_bit_number < _bc_1wire_search.last_discrepancy)
                    {
                        search_direction = ((_bc_1wire_search.last_rom_no[rom_byte_number] & rom_byte_mask) > 0);
                    }
                    else
                    {
                        /* If equal to last pick 1, if not then pick 0 */
                        search_direction = (id_bit_number == _bc_1wire_search.last_discrepancy);
                    }

                    /* If 0 was picked then record its position in LastZero */
                    if (search_direction == 0)
                    {
                        last_zero = id_bit_number;

                        /* Check for Last discrepancy in family */
                        if (last_zero < 9)
                        {
                            _bc_1wire_search.last_family_discrepancy = last_zero;
                        }
                    }
                }

                /* Set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask */
                if (search_direction == 1)
                {
                    _bc_1wire_search.last_rom_no[rom_byte_number] |= rom_byte_mask;
                }
                else
                {
                    _bc_1wire_search.last_rom_no[rom_byte_number] &= ~rom_byte_mask;
                }

                /* Serial number search direction write bit */
                _bc_1wire_write_bit(channel, search_direction);

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
        } while (rom_byte_number < 8);

        /* If the search was successful then */
        if (!(id_bit_number < 65))
        {
            /* Search successful so set LastDiscrepancy, LastDeviceFlag, search_result */
            _bc_1wire_search.last_discrepancy = last_zero;

            /* Check for last device */
            if (_bc_1wire_search.last_discrepancy == 0)
            {
                _bc_1wire_search.last_device_flag = true;
            }

            search_result = !_bc_1wire_search.last_rom_no[0] ? false : true;
        }

        _bc_1wire_stop();
    }

    if (search_result && bc_1wire_crc8(_bc_1wire_search.last_rom_no, sizeof(_bc_1wire_search.last_rom_no), 0x00) != 0)
    {
        search_result = false;
    }

    if (search_result)
    {
        memcpy(device_number, _bc_1wire_search.last_rom_no, sizeof(_bc_1wire_search.last_rom_no));
    }
    else
    {
        _bc_1wire_search_reset();
    }

    return search_result;
}

static int _bc_1wire_search_devices(bc_gpio_channel_t channel, uint64_t *device_numbers, size_t length)
{
    int devices = 0;
    int max_devices = length / sizeof(uint64_t);

    while ((devices < max_devices) && _bc_1wire_search_next(channel, device_numbers))
    {
        device_numbers++;
        devices++;
    }

    return devices;
}
