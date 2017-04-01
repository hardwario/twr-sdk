#include <bc_spi.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

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
    [BC_SPI_MODE_0] = 0x00,  // SPI mode of operation is 0 (CPOL = 0, CPHA = 0)
    [BC_SPI_MODE_1] = 0x01,  // SPI mode of operation is 1 (CPOL = 0, CPHA = 1)
    [BC_SPI_MODE_2] = 0x02,  // SPI mode of operation is 2 (CPOL = 1, CPHA = 0)
    [BC_SPI_MODE_3] = 0x03   // SPI mode of operation is 3 (CPOL = 1, CPHA = 1)
};

static bc_spi_mode_t _bc_spi_mode;
static bc_spi_speed_t _bc_spi_speed;

static uint8_t _bc_spi_transfer_byte(uint8_t value);

void bc_spi_init(bc_spi_speed_t speed, bc_spi_mode_t mode)
{
    // Enable PLL
    bc_module_core_pll_enable();

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
}

void bc_spi_set_speed(bc_spi_speed_t speed)
{
    uint32_t reg;

    // Store desired speed
    _bc_spi_speed = speed;

    // Modify Control Register
    reg = SPI2->CR1;
    reg &= ~(SPI_CR1_BR_Msk | SPI_CR1_SPE);
    reg |= _bc_spi_speed_table[speed];

    // Write back Control Register

    // Disable SPI
    SPI2->CR1 &= ~SPI_CR1_SPE;

    // Update register content
    SPI2->CR1 = reg;

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

bc_spi_speed_t bc_spi_get_speed(void)
{
    return _bc_spi_speed;
}

void bc_spi_set_mode(bc_spi_mode_t mode)
{
    uint32_t reg;

    // Store desired mode
    _bc_spi_mode = mode;

    // Modify Control Register
    reg = SPI2->CR1;
    reg &= ~(SPI_CR1_CPHA_Msk | SPI_CR1_CPOL_Msk | SPI_CR1_SPE);
    reg |= _bc_spi_mode_table[mode];

    // Disable SPI
    SPI2->CR1 &= ~SPI_CR1_SPE;

    // Update register content
    SPI2->CR1 = reg;

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

bc_spi_mode_t bc_spi_get_mode(void)
{
    return _bc_spi_mode;
}

void bc_spi_transfer(const void *source, void *destination, size_t length)
{
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
