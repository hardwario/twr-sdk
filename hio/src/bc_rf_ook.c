#include <hio_rf_ook.h>
#include <hio_timer.h>

void _hio_rf_ook_irq_TIM3_handler(void *param);

static struct
{
    volatile uint16_t packet_bit_position;
    uint8_t packet_length;
    uint8_t packet_data[128];
    volatile bool is_busy;
    uint32_t bit_length_us;
    hio_gpio_channel_t gpio;
} _hio_rf_ook;

static int _hio_rf_ook_char_to_int(char input)
{
    if(input >= '0' && input <= '9')
    {
        return input - '0';
    }
    else if(input >= 'A' && input <= 'F')
    {
        return input - 'A' + 10;
    }
    else if(input >= 'a' && input <= 'f')
    {
        return input - 'a' + 10;
    }
    else
    {
        return 0;
    }
}

static uint8_t _hio_rf_ook_string_to_array(char *str, uint8_t *array)
{
    uint8_t array_length = (strlen(str) + 1) / 2;

    for(int i = 0; i < array_length; i++)
    {
        array[i] = _hio_rf_ook_char_to_int(str[i*2]) * 16;

        if(!(i == array_length - 1 && strlen(str) % 2 == 1))
        {
           array[i] += _hio_rf_ook_char_to_int(str[i*2 + 1]);
        }
    }

    return array_length;
}

static void _hio_rf_ook_tim3_configure(uint32_t resolution_us, uint32_t period_cycles)
{
    // Enable TIM3 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Errata workaround
    RCC->APB1ENR;

    // Disable counter if it is running
    TIM3->CR1 &= ~TIM_CR1_CEN;

    // Set prescaler to 5 * 32 (5 microseconds resolution)
    TIM3->PSC = resolution_us * 32 - 1;
    TIM3->ARR = period_cycles - 1;

    TIM3->DIER |= TIM_DIER_UIE;
    // Enable TIM3 interrupts
    NVIC_EnableIRQ(TIM3_IRQn);
}

void hio_rf_ook_init(hio_gpio_channel_t gpio)
{
    memset(&_hio_rf_ook, 0, sizeof(_hio_rf_ook));

    _hio_rf_ook.gpio = gpio;

    hio_gpio_init(_hio_rf_ook.gpio);
    hio_gpio_set_mode(_hio_rf_ook.gpio, HIO_GPIO_MODE_OUTPUT);

    // TIM3 counter stopped when core is halted
    //DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM3_STOP;

    // Set default bitrate
    hio_rf_ook_set_bitrate(4800);
}

void hio_rf_ook_set_bitrate(uint32_t bitrate)
{
    _hio_rf_ook.bit_length_us = 1e6 / bitrate;
}

void hio_rf_ook_set_bitlength(uint32_t bit_length_us)
{
    _hio_rf_ook.bit_length_us = bit_length_us;
}

bool hio_rf_ook_send(uint8_t *packet, uint8_t length)
{
    if ((TIM3->CR1 & TIM_CR1_CEN) != 0)
    {
        // TIM3 is busy
        return false;
    }

    if (length > sizeof(_hio_rf_ook.packet_data))
    {
        // Packet is too long for library's buffer
        return false;
    }

    hio_system_pll_enable();

    _hio_rf_ook_tim3_configure(1, _hio_rf_ook.bit_length_us);

    _hio_rf_ook.packet_length = length;
    _hio_rf_ook.packet_bit_position = 0;

    memcpy(_hio_rf_ook.packet_data, packet, _hio_rf_ook.packet_length);

    hio_timer_set_irq_handler(TIM3, _hio_rf_ook_irq_TIM3_handler, NULL);

    _hio_rf_ook.is_busy = true;

    TIM3->CR1 |= TIM_CR1_CEN;

    return true;
}

bool hio_rf_ook_send_hex_string(char *hex_string)
{
    uint8_t packet_length = _hio_rf_ook_string_to_array(hex_string, _hio_rf_ook.packet_data);

    return hio_rf_ook_send(_hio_rf_ook.packet_data, packet_length);
}

bool hio_rf_ook_is_busy()
{
    return _hio_rf_ook.is_busy;
}

bool hio_rf_ook_is_ready()
{
    if ((TIM3->CR1 & TIM_CR1_CEN) != 0)
    {
        // TIM3 is busy
        return false;
    }

    return _hio_rf_ook.is_busy;
}

void _hio_rf_ook_irq_TIM3_handler(void *param)
{
    (void) param;
    TIM3->SR = ~TIM_DIER_UIE;

    uint8_t byte_index = _hio_rf_ook.packet_bit_position / 8;
    uint8_t bit_index = _hio_rf_ook.packet_bit_position % 8;

    if(_hio_rf_ook.packet_bit_position / 8 == _hio_rf_ook.packet_length)
    {
        // Disable output after last bit is sent
        hio_gpio_set_output(_hio_rf_ook.gpio, 0);

        // end of transmission
        TIM3->CR1 &= ~TIM_CR1_CEN;
        _hio_rf_ook.is_busy = false;

        hio_system_pll_disable();

        return;
    }

    hio_gpio_set_output(_hio_rf_ook.gpio, _hio_rf_ook.packet_data[byte_index] & (1 << (7 - bit_index)));

    _hio_rf_ook.packet_bit_position++;
}

