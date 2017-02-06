#include <bc_spi.h>
#include <stm32l083xx.h>

static uint32_t _bc_spi_speed_table[4] =
{
    [BC_SPI_SPEED_1_MHZ] = 0x20, // :16 (1MHz)
    [BC_SPI_SPEED_2_MHZ] = 0x18, // :8  (2MHz)
    [BC_SPI_SPEED_4_MHZ] = 0x10, // :4  (4MHz)
    [BC_SPI_SPEED_8_MHZ] = 0x08  // :2  (8MHz)
};

static uint32_t _bc_spi_mode_table[4] =
{
    [BC_SPI_MODE_0] = 0x00,  // SPI mode of operation is 0 (CPOL = 0, CPHA = 0)
    [BC_SPI_MODE_1] = 0x01,  // SPI mode of operation is 1 (CPOL = 0, CPHA = 1)
    [BC_SPI_MODE_2] = 0x02,  // SPI mode of operation is 2 (CPOL = 1, CPHA = 0)
    [BC_SPI_MODE_3] = 0x03   // SPI mode of operation is 3 (CPOL = 1, CPHA = 1)
};

/* Stocked API related variables */
static bc_spi_mode_t _bc_spi_mode;
static bc_spi_speed_t _bc_spi_speed;

static void _bc_spi_init_gpio();
static void _bc_spi_init_spi(bc_spi_speed_t speed, bc_spi_mode_t mode);
static void _bc_spi_set_speed(bc_spi_speed_t speed);
static void _bc_spi_set_mode(bc_spi_mode_t mode);
static uint8_t _bc_spi_transfer_byte(uint8_t value);

void bc_spi_init(bc_spi_speed_t speed, bc_spi_mode_t mode)
{
    // Initialize GPIO pins
    _bc_spi_init_gpio();

    // Initialize SPI peripheral with specific speed and mode
    _bc_spi_init_spi(speed, mode);
}

void bc_spi_set_speed(bc_spi_speed_t speed)
{
    // Set SPI peripheral speed
    _bc_spi_set_speed(speed);
}

bc_spi_speed_t bc_spi_get_speed(void)
{
    // Return SPI peripheral speed
    return _bc_spi_speed;
}

void bc_spi_set_mode(bc_spi_mode_t mode)
{
    // Set SPI peripheral mode
    _bc_spi_set_mode(mode);
}

bc_spi_mode_t bc_spi_get_mode(void)
{
    // Return SPI peripheral mode
    return _bc_spi_mode;
}

void bc_spi_transfer(const void *source, void *destination, size_t length)
{
    uint8_t *src = (uint8_t *) source;
    uint8_t *dst = (uint8_t *) destination;

    GPIOB->BSRR = GPIO_BSRR_BR_12;

    while (length--)
    {
        if (src == NULL)
        {
            // Read byte
            *dst = _bc_spi_transfer_byte(0x00);

            // Increment destination address
            dst++;
        }
        else if (dst == NULL)
        {
            // TODO ... test

            // Read byte
            _bc_spi_transfer_byte(*src);

            // Increment source address
            src++;
        }
        else
        {
            // TODO ... test

            // Read-write byte
            *dst = _bc_spi_transfer_byte(*src);

            // Increment source and destination addresses
            dst++;
            src++;
        }
    }

    GPIOB->BSRR = GPIO_BSRR_BS_12;
}

static uint8_t _bc_spi_transfer_byte(uint8_t value)
{
// TODO ... future trouble

    // Wait until transmit buffer is empty
    while ((SPI2->SR & SPI_SR_TXE) == 0)
    {
        // TODO ... m8 tadz v86n2 b7t continue

        continue;
    }

    // Write data register
    SPI2->DR = value;

    // Wait until receive buffer is full
    while ((SPI2->SR & SPI_SR_RXNE) == 0)
    {
        // TODO ... m8 tadz v86n2 b7t continue

        continue;
    }

    // Read data register
    value = SPI2->DR;

    return value;
}

static void _bc_spi_init_gpio()
{
    // Enable GPIOB clock
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

    // TODO Errata workaround
    RCC->IOPENR;

    // Pull-down on MISO pin
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD14_1;

    // Very high speed on CS, SCLK, MISO and MOSI pins
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED12 | GPIO_OSPEEDER_OSPEED13 | GPIO_OSPEEDER_OSPEED14 | GPIO_OSPEEDER_OSPEED15;

    // General purpose output on CS pin and alternate function on SCLK, MISO and MOSI pins
    GPIOB->MODER &= ~(GPIO_MODER_MODE12_1 | GPIO_MODER_MODE13_0 | GPIO_MODER_MODE14_0 | GPIO_MODER_MODE15_0);

    // Set CS to inactive level;
    GPIOB->BSRR = GPIO_BSRR_BS_12;
}

static void _bc_spi_init_spi(bc_spi_speed_t speed, bc_spi_mode_t mode)
{
    // Enable clock for SPI2
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    // Software slave management, master configuration
    SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR;

    _bc_spi_set_speed(speed);

    _bc_spi_set_mode(mode);

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

static void _bc_spi_set_speed(bc_spi_speed_t speed)
{
    uint32_t temp;

    // Store speed to private variable
    _bc_spi_speed = speed;

    // Edit baud rate divider
    temp = SPI2->CR1;
    temp &= ~SPI_CR1_BR_Msk;
    temp |= _bc_spi_speed_table[speed];
    SPI2->CR1 = temp;
}

static void _bc_spi_set_mode(bc_spi_mode_t mode)
{
    uint32_t temp;

    // Store mode to private variable
    _bc_spi_mode = mode;

    // Edit mode
    temp = SPI2->CR1;
    temp &= ~(SPI_CR1_CPHA_Msk | SPI_CR1_CPOL_Msk);
    temp |= _bc_spi_mode_table[mode];
    SPI2->CR1 = temp;
}
