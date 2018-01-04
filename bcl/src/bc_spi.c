#include <bc_spi.h>
#include <bc_module_core.h>
#include <bc_scheduler.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

#define _BC_SPI_EVENT_CLEAR 0

static const uint32_t _bc_spi_speed_table[5] =
{
    [BC_SPI_SPEED_1_MHZ] = 0x20, // :16 (1MHz)
    [BC_SPI_SPEED_2_MHZ] = 0x18, // :8  (2MHz)
    [BC_SPI_SPEED_4_MHZ] = 0x10, // :4  (4MHz)
    [BC_SPI_SPEED_8_MHZ] = 0x08, // :2  (8MHz)
    [BC_SPI_SPEED_16_MHZ] = 0x00 // :1  (16MHz)
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
    bc_spi_event_t pending_event;
    bool initilized;
    bc_scheduler_task_id_t task_id;
} _bc_spi;

static uint8_t _bc_spi_transfer_byte(uint8_t value);

static inline void _bc_spi_transfer_error_handler();

static inline void _bc_spi_transfer_done_handler();

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

    // Software slave management, master configuration
    SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR;

    // Set SPI speed
    bc_spi_set_speed(speed);

    // Set SPI mode
    bc_spi_set_mode(mode);

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;

    // Enable DMA 1 channel 5 interrupts
    NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

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

bool bc_spi_is_ready(void)
{
	return (!_bc_spi.in_progress) && (_bc_spi.pending_event == _BC_SPI_EVENT_CLEAR);
}

bool bc_spi_lock(void)
{
    bc_irq_disable();

    if (bc_spi_is_ready())
    {
        _bc_spi.in_progress = true;

        return true;
    }

    return false;

    bc_irq_enable();
}

void bc_spi_unlock(void)
{
    bc_irq_disable();

    _bc_spi.in_progress = false;

    bc_irq_enable();
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
    bc_module_core_pll_enable();

    // Set CS to active level
    GPIOB->BSRR = GPIO_BSRR_BR_12;

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

    // Set CS to inactive level
    GPIOB->BSRR = GPIO_BSRR_BS_12;

    // Disable PLL and enable sleep
    bc_module_core_pll_disable();

    // Update status
    _bc_spi.in_progress = false;

    return true;
}

bool bc_spi_async_transfer(const void *source, void *destination, size_t length, void (*event_handler)(bc_spi_event_t event, void *event_param), void (*event_param))
{
    // If another transfer cannot be executed now ...
    if((_bc_spi.in_progress == true) || (_bc_spi.pending_event != _BC_SPI_EVENT_CLEAR))
    {
        // ... dont do it
        return false;
    }

    // Update event related variables
    _bc_spi.event_handler = event_handler;
    _bc_spi.event_param = event_param;

    // Enable DMA1
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    // Enable PLL and disable sleep
    bc_module_core_pll_enable();

    // If transmit only is requested ...
    if ((source != NULL) && (destination == NULL))
    {
        // ... execute it

        // Set CS to active level
        GPIOB->BSRR = GPIO_BSRR_BR_12;

        // Update status
        _bc_spi.in_progress = true;

        // Set memory incrementation, direction from memory and disable peripheral
        DMA1_Channel5->CCR = DMA_CCR_DIR | DMA_CCR_MINC;

        // Reset request selection for DMA1 Channel5
        DMA1_CSELR->CSELR &= ~DMA_CSELR_C5S;

        // Configure request 2 selection for DMA1 Channel5
        DMA1_CSELR->CSELR |= (uint32_t) (2 << DMA_CSELR_C5S_Pos);

        // Configure DMA channel data length
        DMA1_Channel5->CNDTR = length;

        // Configure DMA channel source address
        DMA1_Channel5->CPAR = (uint32_t) &SPI2->DR;

        // Configure DMA channel destination address
        DMA1_Channel5->CMAR = (uint32_t) source;

        // Enable the transfer complete and error interrupts
        DMA1_Channel5->CCR |= DMA_IT_TC | DMA_IT_TE;

        // Enable the peripheral
        DMA1_Channel5->CCR |= DMA_CCR_EN;

        // Disable SPI2
        SPI2->CR1 &= ~SPI_CR1_SPE;

        // Enable Tx DMA request
        SPI2->CR2 |= SPI_CR2_TXDMAEN;

        // Enable SPI2
        SPI2->CR1 |= SPI_CR1_SPE;

        return true;
    }
    // If receive only is requested ...
    else if ((source == NULL) && (destination != NULL))
    {
        // TODO Ready to implement another dirrection

        // Disable PLL and disable sleep
        bc_module_core_pll_disable();
    }
    // If transmit and receive is requested ...
    else
    {
        // TODO Ready to implement another dirrection

        // Disable PLL and disable sleep
        bc_module_core_pll_disable();
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

    return value;
}

void DMA1_Channel4_5_6_7_IRQHandler(void)
{
    // Transfer error interrupt management
    if ((DMA1->ISR & DMA_ISR_TEIF5) != 0)
    {
        // TODO Not sure what should happen here

        // Disable the transfer error interrupt
        DMA1_Channel5->CCR &= ~DMA_CCR_TEIE;

        // Clear the transfer error flag
        DMA1->IFCR = DMA_IFCR_CTEIF5;

        // Transfer error callback
        _bc_spi_transfer_error_handler();
    }

    // Transfer complete interrupt management
    if ((DMA1->ISR & DMA_ISR_TCIF5) != 0)
    {
        // Disable the transfer error and complete interrupts
        DMA1_Channel5->CCR &= ~(DMA_CCR_TEIE | DMA_CCR_TCIE);

        // Clear the transfer complete flag
        DMA1->IFCR = DMA_IFCR_CTCIF5;

        // Transfer complete callback
        _bc_spi_transfer_done_handler();
    }
}

static inline void _bc_spi_transfer_error_handler()
{
    // TODO Not sure what should happen here

    // Update status
    _bc_spi.in_progress = false;
    _bc_spi.pending_event |= BC_SPI_EVENT_ERROR;

    GPIOB->BSRR = GPIO_BSRR_BS_12;

    // Plan task that call event handler
    bc_scheduler_plan_now(_bc_spi.task_id);
}

static inline void _bc_spi_transfer_done_handler()
{
    // Update status
    _bc_spi.in_progress = false;
    _bc_spi.pending_event |= BC_SPI_EVENT_DONE;

    GPIOB->BSRR = GPIO_BSRR_BS_12;

    // Plan task that call event handler
    bc_scheduler_plan_now(_bc_spi.task_id);
}

static void _bc_spi_task()
{
    // If is event handler valid ...
    if (_bc_spi.event_handler != NULL)
    {
        if((_bc_spi.pending_event & BC_SPI_EVENT_ERROR) != 0)
        {
            // ... call event handler with BC_SPI_EVENT_ERROR
            _bc_spi.event_handler(BC_SPI_EVENT_ERROR, _bc_spi.event_param);
            _bc_spi.pending_event &= ~BC_SPI_EVENT_ERROR;

            // TODO Not sure what should happen here (viz. _bc_spi_transfer_error_handler)

            // Disable PLL and enable sleep
            bc_module_core_pll_disable();
        }
        if ((_bc_spi.pending_event & BC_SPI_EVENT_DONE) != 0)
        {
            // ... call event handler with BC_SPI_EVENT_CPLT
            _bc_spi.event_handler(BC_SPI_EVENT_DONE, _bc_spi.event_param);
            _bc_spi.pending_event &= ~BC_SPI_EVENT_DONE;

            // Disable PLL and enable sleep
            bc_module_core_pll_disable();
        }
    }
    else
    {
        _bc_spi.pending_event = _BC_SPI_EVENT_CLEAR;
    }
}
