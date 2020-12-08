#include <twr_spi.h>
#include <twr_scheduler.h>
#include <twr_dma.h>
#include <twr_system.h>
#include <twr_timer.h>
#include <stm32l0xx.h>

#define _TWR_SPI_EVENT_CLEAR 0

static const uint32_t _twr_spi_speed_table[8] =
{
    [TWR_SPI_SPEED_125_KHZ] = 0x38, // :128 (500 kHz)
    [TWR_SPI_SPEED_250_KHZ] = 0x30, // :64  (250 kHz)
    [TWR_SPI_SPEED_500_KHZ] = 0x28, // :32  (500 kHz)
    [TWR_SPI_SPEED_1_MHZ]   = 0x20, // :16  (1 MHz)
    [TWR_SPI_SPEED_2_MHZ]   = 0x18, // :8   (2 MHz)
    [TWR_SPI_SPEED_4_MHZ]   = 0x10, // :4   (4 MHz)
    [TWR_SPI_SPEED_8_MHZ]   = 0x08, // :2   (8 MHz)
    [TWR_SPI_SPEED_16_MHZ]  = 0x00  // :1   (16 MHz)
};

static const uint32_t _twr_spi_mode_table[4] =
{
    [TWR_SPI_MODE_0] = 0x00, // SPI mode of operation is 0 (CPOL = 0, CPHA = 0)
    [TWR_SPI_MODE_1] = 0x01, // SPI mode of operation is 1 (CPOL = 0, CPHA = 1)
    [TWR_SPI_MODE_2] = 0x02, // SPI mode of operation is 2 (CPOL = 1, CPHA = 0)
    [TWR_SPI_MODE_3] = 0x03  // SPI mode of operation is 3 (CPOL = 1, CPHA = 1)
};

static struct
{
    twr_spi_mode_t mode;
    twr_spi_speed_t speed;
    void (*event_handler)(twr_spi_event_t event, void *_twr_spi_event_param);
    void *event_param;
    bool in_progress;
    bool pending_event_done;
    bool initilized;
    bool manual_cs_control;
    twr_scheduler_task_id_t task_id;
    uint16_t cs_delay;
    uint16_t delay;
    uint16_t cs_quit;

} _twr_spi;

static twr_dma_channel_config_t _twr_spi_dma_config =
{
    .request = TWR_DMA_REQUEST_2,
    .direction = TWR_DMA_DIRECTION_TO_PERIPHERAL,
    .data_size_memory = TWR_DMA_SIZE_1,
    .data_size_peripheral = TWR_DMA_SIZE_1,
    .mode = TWR_DMA_MODE_STANDARD,
    .address_peripheral = (void *)&SPI2->DR,
    .priority = TWR_DMA_PRIORITY_HIGH
};

static uint8_t _twr_spi_transfer_byte(uint8_t value);

static void _twr_spi_dma_event_handler(twr_dma_channel_t channel, twr_dma_event_t event, void *event_param);

static void _twr_spi_task();

void twr_spi_init(twr_spi_speed_t speed, twr_spi_mode_t mode)
{
    // If is already initilized ...
    if(_twr_spi.initilized == true)
    {
        // ... dont do it again
        return;
    }
    _twr_spi.initilized = true;

    // Enable GPIOB clock
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

    // Errata workaround
    RCC->IOPENR;

    // Pull-down on MISO pin
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD14_1;

    // Very high speed on CS, SCLK, MISO and MOSI pins
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED12 | GPIO_OSPEEDER_OSPEED13 | GPIO_OSPEEDER_OSPEED14 | GPIO_OSPEEDER_OSPEED15;

    // General purpose output on CS pin and alternate function on SCLK, MISO and MOSI pins
    GPIOB->MODER &= ~(GPIO_MODER_MODE12_1 | GPIO_MODER_MODE13_0 | GPIO_MODER_MODE14_0 | GPIO_MODER_MODE15_0);

    // Set CS to inactive level;
    GPIOB->BSRR = GPIO_BSRR_BS_12;

    // Enable clock for SPI2
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    // Errata workaround
    RCC->AHBENR;

    // Software slave management, master configuration
    SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR;

    // Set SPI speed
    twr_spi_set_speed(speed);

    // Set SPI mode
    twr_spi_set_mode(mode);

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;

    twr_dma_init();

    twr_timer_init();

    twr_dma_set_event_handler(TWR_DMA_CHANNEL_5, _twr_spi_dma_event_handler, NULL);

    _twr_spi.task_id = twr_scheduler_register(_twr_spi_task, NULL, TWR_TICK_INFINITY);
}

void twr_spi_set_speed(twr_spi_speed_t speed)
{
    uint32_t cr1;

    // Store desired speed
    _twr_spi.speed = speed;

    // Disable SPI
    SPI2->CR1 &= ~SPI_CR1_SPE;

    // Edit the registry image
    cr1 = SPI2->CR1;
    cr1 &= ~(SPI_CR1_BR_Msk | SPI_CR1_SPE);
    cr1 |= _twr_spi_speed_table[speed];

    // Update CR1
    SPI2->CR1 = cr1;

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

void twr_spi_set_timing(uint16_t cs_delay, uint16_t delay, uint16_t cs_quit)
{
    _twr_spi.cs_delay = cs_delay;

    _twr_spi.delay = delay;

    _twr_spi.cs_quit = cs_quit;
}

twr_spi_speed_t twr_spi_get_speed(void)
{
    return _twr_spi.speed;
}

void twr_spi_set_mode(twr_spi_mode_t mode)
{
    uint32_t cr1;

    // Store desired mode
    _twr_spi.mode = mode;

    // Disable SPI
    SPI2->CR1 &= ~SPI_CR1_SPE;

    // Edit the registry image
    cr1 = SPI2->CR1;
    cr1 &= ~(SPI_CR1_CPHA_Msk | SPI_CR1_CPOL_Msk | SPI_CR1_SPE);
    cr1 |= _twr_spi_mode_table[mode];

    // Update CR1
    SPI2->CR1 = cr1;

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

twr_spi_mode_t twr_spi_get_mode(void)
{
    return _twr_spi.mode;
}

void twr_spi_set_manual_cs_control(bool manual_cs_control)
{
    _twr_spi.manual_cs_control = manual_cs_control;
}

bool twr_spi_is_ready(void)
{
	return (!_twr_spi.in_progress) && (_twr_spi.pending_event_done == _TWR_SPI_EVENT_CLEAR);
}

bool twr_spi_transfer(const void *source, void *destination, size_t length)
{
    // If another transfer cannot be executed ...
    if (_twr_spi.in_progress == true)
    {
        // ... dont do it
        return false;
    }

    // Update status
    _twr_spi.in_progress = true;

    // Enable PLL and disable sleep
    twr_system_pll_enable();

    // Set CS to active level
    if (!_twr_spi.manual_cs_control)
    {
        GPIOB->BSRR = GPIO_BSRR_BR_12;
    }

    if (_twr_spi.cs_delay > 0)
    {
        twr_timer_start();
        twr_timer_delay(_twr_spi.cs_delay);
        twr_timer_stop();
    }

    if (source == NULL)
    {
        for (size_t i = 0; i < length; i++)
        {
            // Read byte
            *((uint8_t *) destination + i) = _twr_spi_transfer_byte(0);
        }
    }
    else if (destination == NULL)
    {
        for (size_t i = 0; i < length; i++)
        {
            // Write byte
            _twr_spi_transfer_byte(*((uint8_t *) source + i));
        }
    }
    else
    {
        for (size_t i = 0; i < length; i++)
        {
            // Read and write byte
            *((uint8_t *) destination + i) = _twr_spi_transfer_byte(*((uint8_t *) source + i));
        }
    }

    if (_twr_spi.cs_quit > 0)
    {
        twr_timer_start();
        twr_timer_delay(_twr_spi.cs_quit);
        twr_timer_stop();
    }

    // Set CS to inactive level
    if (!_twr_spi.manual_cs_control)
    {
        GPIOB->BSRR = GPIO_BSRR_BS_12;
    }

    // Disable PLL and enable sleep
    twr_system_pll_disable();

    // Update status
    _twr_spi.in_progress = false;

    return true;
}

bool twr_spi_async_transfer(const void *source, void *destination, size_t length, void (*event_handler)(twr_spi_event_t event, void *event_param), void (*event_param))
{
    // If another transfer cannot be executed now ...
    if((_twr_spi.in_progress == true) || (_twr_spi.pending_event_done != _TWR_SPI_EVENT_CLEAR))
    {
        // ... dont do it
        return false;
    }

    // Update event related variables
    _twr_spi.event_handler = event_handler;
    _twr_spi.event_param = event_param;

    // Enable PLL and disable sleep
    twr_system_pll_enable();

    // If transmit only is requested ...
    if ((source != NULL) && (destination == NULL))
    {
        // ... execute it

        // Set CS to active level
        GPIOB->BSRR = GPIO_BSRR_BR_12;

        // Update status
        _twr_spi.in_progress = true;

        // Disable SPI2
        SPI2->CR1 &= ~SPI_CR1_SPE;

        // Enable TX DMA request
        SPI2->CR2 |= SPI_CR2_TXDMAEN;

        // Enable SPI2
        SPI2->CR1 |= SPI_CR1_SPE;

        // Setup DMA channel
        _twr_spi_dma_config.address_memory = (void *)source;
        _twr_spi_dma_config.length = length;
        twr_dma_channel_config(TWR_DMA_CHANNEL_5, &_twr_spi_dma_config);
        twr_dma_channel_run(TWR_DMA_CHANNEL_5);

        return true;
    }
    // If receive only is requested ...
    else if ((source == NULL) && (destination != NULL))
    {
        // TODO Ready to implement another direction

        // Disable PLL and disable sleep
        twr_system_pll_disable();
    }
    // If transmit and receive is requested ...
    else
    {
        // TODO Ready to implement another direction

        // Disable PLL and disable sleep
        twr_system_pll_disable();
    }

    return false;
}

static uint8_t _twr_spi_transfer_byte(uint8_t value)
{
    // Wait until transmit buffer is empty...
    while ((SPI2->SR & SPI_SR_TXE) == 0)
    {
        continue;
    }

    // Write data register
    SPI2->DR = value;

    // Until receive buffer is empty...
    while ((SPI2->SR & SPI_SR_RXNE) == 0)
    {
        continue;
    }

    // Read data register
    value = SPI2->DR;

    if (_twr_spi.delay > 0)
    {
        twr_timer_start();
        twr_timer_delay(_twr_spi.delay);
        twr_timer_stop();
    }

    return value;
}

static void _twr_spi_dma_event_handler(twr_dma_channel_t channel, twr_dma_event_t event, void *event_param)
{
	(void) channel;
	(void) event_param;

	if (event == TWR_DMA_EVENT_DONE)
	{
	    // Update status
	    _twr_spi.in_progress = false;
	    _twr_spi.pending_event_done = true;

	    GPIOB->BSRR = GPIO_BSRR_BS_12;

	    // Plan task that call event handler
	    twr_scheduler_plan_now(_twr_spi.task_id);
	}
	else if (event == TWR_DMA_EVENT_ERROR)
	{
	    twr_system_reset();
	}
}

static void _twr_spi_task()
{
    // If is event handler valid ...
    if (_twr_spi.event_handler != NULL)
    {
        // ... call event handler
        _twr_spi.event_handler(TWR_SPI_EVENT_DONE, _twr_spi.event_param);

        // Disable PLL and enable sleep
        twr_system_pll_disable();
    }

    _twr_spi.pending_event_done = false;
}
