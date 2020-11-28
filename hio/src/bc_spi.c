#include <bc_spi.h>
#include <bc_scheduler.h>
#include <bc_dma.h>
#include <bc_system.h>
#include <bc_timer.h>
#include <stm32l0xx.h>

#define _BC_SPI_EVENT_CLEAR 0

static const uint32_t _bc_spi_speed_table[8] =
{
    [BC_SPI_SPEED_125_KHZ] = 0x38, // :128 (500 kHz)
    [BC_SPI_SPEED_250_KHZ] = 0x30, // :64  (250 kHz)
    [BC_SPI_SPEED_500_KHZ] = 0x28, // :32  (500 kHz)
    [BC_SPI_SPEED_1_MHZ]   = 0x20, // :16  (1 MHz)
    [BC_SPI_SPEED_2_MHZ]   = 0x18, // :8   (2 MHz)
    [BC_SPI_SPEED_4_MHZ]   = 0x10, // :4   (4 MHz)
    [BC_SPI_SPEED_8_MHZ]   = 0x08, // :2   (8 MHz)
    [BC_SPI_SPEED_16_MHZ]  = 0x00  // :1   (16 MHz)
};

static const uint32_t _bc_spi_mode_table[4] =
{
    [BC_SPI_MODE_0] = 0x00, // SPI mode of operation is 0 (CPOL = 0, CPHA = 0)
    [BC_SPI_MODE_1] = 0x01, // SPI mode of operation is 1 (CPOL = 0, CPHA = 1)
    [BC_SPI_MODE_2] = 0x02, // SPI mode of operation is 2 (CPOL = 1, CPHA = 0)
    [BC_SPI_MODE_3] = 0x03  // SPI mode of operation is 3 (CPOL = 1, CPHA = 1)
};

static struct
{
    bc_spi_mode_t mode;
    bc_spi_speed_t speed;
    void (*event_handler)(bc_spi_event_t event, void *_bc_spi_event_param);
    void *event_param;
    bool in_progress;
    bool pending_event_done;
    bool initilized;
    bool manual_cs_control;
    bc_scheduler_task_id_t task_id;
    uint16_t cs_delay;
    uint16_t delay;
    uint16_t cs_quit;

} _bc_spi;

static bc_dma_channel_config_t _bc_spi_dma_config =
{
    .request = BC_DMA_REQUEST_2,
    .direction = BC_DMA_DIRECTION_TO_PERIPHERAL,
    .data_size_memory = BC_DMA_SIZE_1,
    .data_size_peripheral = BC_DMA_SIZE_1,
    .mode = BC_DMA_MODE_STANDARD,
    .address_peripheral = (void *)&SPI2->DR,
    .priority = BC_DMA_PRIORITY_HIGH
};

static uint8_t _bc_spi_transfer_byte(uint8_t value);

static void _bc_spi_dma_event_handler(bc_dma_channel_t channel, bc_dma_event_t event, void *event_param);

static void _bc_spi_task();

void bc_spi_init(bc_spi_speed_t speed, bc_spi_mode_t mode)
{
    // If is already initilized ...
    if(_bc_spi.initilized == true)
    {
        // ... dont do it again
        return;
    }
    _bc_spi.initilized = true;

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
    bc_spi_set_speed(speed);

    // Set SPI mode
    bc_spi_set_mode(mode);

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;

    bc_dma_init();

    bc_timer_init();

    bc_dma_set_event_handler(BC_DMA_CHANNEL_5, _bc_spi_dma_event_handler, NULL);

    _bc_spi.task_id = bc_scheduler_register(_bc_spi_task, NULL, BC_TICK_INFINITY);
}

void bc_spi_set_speed(bc_spi_speed_t speed)
{
    uint32_t cr1;

    // Store desired speed
    _bc_spi.speed = speed;

    // Disable SPI
    SPI2->CR1 &= ~SPI_CR1_SPE;

    // Edit the registry image
    cr1 = SPI2->CR1;
    cr1 &= ~(SPI_CR1_BR_Msk | SPI_CR1_SPE);
    cr1 |= _bc_spi_speed_table[speed];

    // Update CR1
    SPI2->CR1 = cr1;

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

void bc_spi_set_timing(uint16_t cs_delay, uint16_t delay, uint16_t cs_quit)
{
    _bc_spi.cs_delay = cs_delay;

    _bc_spi.delay = delay;

    _bc_spi.cs_quit = cs_quit;
}

bc_spi_speed_t bc_spi_get_speed(void)
{
    return _bc_spi.speed;
}

void bc_spi_set_mode(bc_spi_mode_t mode)
{
    uint32_t cr1;

    // Store desired mode
    _bc_spi.mode = mode;

    // Disable SPI
    SPI2->CR1 &= ~SPI_CR1_SPE;

    // Edit the registry image
    cr1 = SPI2->CR1;
    cr1 &= ~(SPI_CR1_CPHA_Msk | SPI_CR1_CPOL_Msk | SPI_CR1_SPE);
    cr1 |= _bc_spi_mode_table[mode];

    // Update CR1
    SPI2->CR1 = cr1;

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

bc_spi_mode_t bc_spi_get_mode(void)
{
    return _bc_spi.mode;
}

void bc_spi_set_manual_cs_control(bool manual_cs_control)
{
    _bc_spi.manual_cs_control = manual_cs_control;
}

bool bc_spi_is_ready(void)
{
	return (!_bc_spi.in_progress) && (_bc_spi.pending_event_done == _BC_SPI_EVENT_CLEAR);
}

bool bc_spi_transfer(const void *source, void *destination, size_t length)
{
    // If another transfer cannot be executed ...
    if (_bc_spi.in_progress == true)
    {
        // ... dont do it
        return false;
    }

    // Update status
    _bc_spi.in_progress = true;

    // Enable PLL and disable sleep
    bc_system_pll_enable();

    // Set CS to active level
    if (!_bc_spi.manual_cs_control)
    {
        GPIOB->BSRR = GPIO_BSRR_BR_12;
    }

    if (_bc_spi.cs_delay > 0)
    {
        bc_timer_start();
        bc_timer_delay(_bc_spi.cs_delay);
        bc_timer_stop();
    }

    if (source == NULL)
    {
        for (size_t i = 0; i < length; i++)
        {
            // Read byte
            *((uint8_t *) destination + i) = _bc_spi_transfer_byte(0);
        }
    }
    else if (destination == NULL)
    {
        for (size_t i = 0; i < length; i++)
        {
            // Write byte
            _bc_spi_transfer_byte(*((uint8_t *) source + i));
        }
    }
    else
    {
        for (size_t i = 0; i < length; i++)
        {
            // Read and write byte
            *((uint8_t *) destination + i) = _bc_spi_transfer_byte(*((uint8_t *) source + i));
        }
    }

    if (_bc_spi.cs_quit > 0)
    {
        bc_timer_start();
        bc_timer_delay(_bc_spi.cs_quit);
        bc_timer_stop();
    }

    // Set CS to inactive level
    if (!_bc_spi.manual_cs_control)
    {
        GPIOB->BSRR = GPIO_BSRR_BS_12;
    }

    // Disable PLL and enable sleep
    bc_system_pll_disable();

    // Update status
    _bc_spi.in_progress = false;

    return true;
}

bool bc_spi_async_transfer(const void *source, void *destination, size_t length, void (*event_handler)(bc_spi_event_t event, void *event_param), void (*event_param))
{
    // If another transfer cannot be executed now ...
    if((_bc_spi.in_progress == true) || (_bc_spi.pending_event_done != _BC_SPI_EVENT_CLEAR))
    {
        // ... dont do it
        return false;
    }

    // Update event related variables
    _bc_spi.event_handler = event_handler;
    _bc_spi.event_param = event_param;

    // Enable PLL and disable sleep
    bc_system_pll_enable();

    // If transmit only is requested ...
    if ((source != NULL) && (destination == NULL))
    {
        // ... execute it

        // Set CS to active level
        GPIOB->BSRR = GPIO_BSRR_BR_12;

        // Update status
        _bc_spi.in_progress = true;

        // Disable SPI2
        SPI2->CR1 &= ~SPI_CR1_SPE;

        // Enable TX DMA request
        SPI2->CR2 |= SPI_CR2_TXDMAEN;

        // Enable SPI2
        SPI2->CR1 |= SPI_CR1_SPE;

        // Setup DMA channel
        _bc_spi_dma_config.address_memory = (void *)source;
        _bc_spi_dma_config.length = length;
        bc_dma_channel_config(BC_DMA_CHANNEL_5, &_bc_spi_dma_config);
        bc_dma_channel_run(BC_DMA_CHANNEL_5);

        return true;
    }
    // If receive only is requested ...
    else if ((source == NULL) && (destination != NULL))
    {
        // TODO Ready to implement another direction

        // Disable PLL and disable sleep
        bc_system_pll_disable();
    }
    // If transmit and receive is requested ...
    else
    {
        // TODO Ready to implement another direction

        // Disable PLL and disable sleep
        bc_system_pll_disable();
    }

    return false;
}

static uint8_t _bc_spi_transfer_byte(uint8_t value)
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

    if (_bc_spi.delay > 0)
    {
        bc_timer_start();
        bc_timer_delay(_bc_spi.delay);
        bc_timer_stop();
    }

    return value;
}

static void _bc_spi_dma_event_handler(bc_dma_channel_t channel, bc_dma_event_t event, void *event_param)
{
	(void) channel;
	(void) event_param;

	if (event == BC_DMA_EVENT_DONE)
	{
	    // Update status
	    _bc_spi.in_progress = false;
	    _bc_spi.pending_event_done = true;

	    GPIOB->BSRR = GPIO_BSRR_BS_12;

	    // Plan task that call event handler
	    bc_scheduler_plan_now(_bc_spi.task_id);
	}
	else if (event == BC_DMA_EVENT_ERROR)
	{
	    bc_system_reset();
	}
}

static void _bc_spi_task()
{
    // If is event handler valid ...
    if (_bc_spi.event_handler != NULL)
    {
        // ... call event handler
        _bc_spi.event_handler(BC_SPI_EVENT_DONE, _bc_spi.event_param);

        // Disable PLL and enable sleep
        bc_system_pll_disable();
    }

    _bc_spi.pending_event_done = false;
}
