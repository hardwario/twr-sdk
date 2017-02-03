#include <bc_spi.h>
#include <stm32l083xx.h>

// cs ... p15
// sclk ... p14
// mosi ... p13
// miso ... p12

typedef struct
{
    uint32_t CLKPolarity;
    uint32_t CLKPhase;
} _bc_spi_mode_t;

static _bc_spi_mode_t _bc_spi_mode_table[4];/* = {
     [BC_SPI_MODE_0] = {SPI_POLARITY_LOW, SPI_PHASE_1EDGE },
     [BC_SPI_MODE_1] = {SPI_POLARITY_LOW, SPI_PHASE_2EDGE },
     [BC_SPI_MODE_2] = {SPI_POLARITY_HIGH, SPI_PHASE_1EDGE },
     [BC_SPI_MODE_3] = {SPI_POLARITY_HIGH, SPI_PHASE_2EDGE }
}; */

/* Stocked API related variables */
static bc_spi_mode_t _bc_spi_mode;
static bc_spi_speed_t _bc_spi_speed;

static void _bc_spi_init_gpio();
static void _bc_spi_init_spi(bc_spi_speed_t speed, bc_spi_mode_t mode);
static void _bc_spi_set_speed(bc_spi_speed_t speed);
static void _bc_spi_set_mode(bc_spi_mode_t mode);

void bc_spi_init(bc_spi_speed_t speed, bc_spi_mode_t mode)
{
    _bc_spi_init_gpio();
    _bc_spi_init_spi(speed, mode);
}

void bc_spi_set_speed(bc_spi_speed_t speed)
{
    _bc_spi_set_speed(speed);
}

bc_spi_speed_t bc_spi_get_speed(void)
{
    return _bc_spi_speed;
}

void bc_spi_set_mode(bc_spi_mode_t mode)
{
    _bc_spi_set_mode(mode);
}

bc_spi_mode_t bc_spi_get_mode(void)
{
    return _bc_spi_mode;
}

void bc_spi_transfer(const void *source, void *destination, size_t length)
{

}



static void _bc_spi_init_gpio()
{
    // Enable clock for GPIOH, GPIOB and GPIOA
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

    // TODO ... co to sakra >/D
    // TODO Errata workaround
    RCC->IOPENR;

    // Pull-down on MISO pin
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD14_1;

    // Very high speed on CS, MOSI and SCLK pins
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED12 | GPIO_OSPEEDER_OSPEED13 | GPIO_OSPEEDER_OSPEED14 | GPIO_OSPEEDER_OSPEED15;

    // General purpose output on CS pin
    GPIOB->MODER &= ~(GPIO_MODER_MODE12_1);

    // Alternate function on SCLK, MISO and MOSI pins
    GPIOB->MODER &= ~(GPIO_MODER_MODE13_0 | GPIO_MODER_MODE14_0 | GPIO_MODER_MODE15_0);
}

static void _bc_spi_init_spi(bc_spi_speed_t speed, bc_spi_mode_t mode)
{
    // Enable clock for SPI2
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    // Software slave management, baud rate control = fPCLK / 8, master configuration
    SPI2->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1 | SPI_CR1_MSTR;

    _bc_spi_set_speed(speed);

    _bc_spi_set_mode(mode);

    // Enable SPI
    SPI2->CR1 |= SPI_CR1_SPE;
}

static void _bc_spi_set_speed(bc_spi_speed_t speed)
{
    // TODO ... add clock switch
    _bc_spi_speed = speed;
}

static void _bc_spi_set_mode(bc_spi_mode_t mode)
{
    // TODO ... add mode switch
    _bc_spi_mode = mode;
}


#if 0
void bc_spirit1_hal_chip_select_low(void)
{
    // Set CS pin to log. 0
    GPIOA->BSRR = GPIO_BSRR_BR_15;

    // Set prescaler
    TIM7->PSC = 0;

    // Set auto-reload register - period 4 us
    TIM7->ARR = 128 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }
}

void bc_spirit1_hal_chip_select_high(void)
{
    // Set prescaler
    TIM7->PSC = 0;

    // Set auto-reload register - period 4 us
    TIM7->ARR = 128 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }

    // Set CS pin to log. 1
    GPIOA->BSRR = GPIO_BSRR_BS_15;

    // Set prescaler
    TIM7->PSC = 0;

    // Set auto-reload register - period 4 us
    TIM7->ARR = 128 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }
}

uint8_t bc_spirit1_hal_transfer_byte(uint8_t value)
{
    // Wait until transmit buffer is empty
    while ((SPI1->SR & SPI_SR_TXE) == 0)
    {
        continue;
    }

    // Write data register
    SPI1->DR = value;

    // Wait until receive buffer is full
    while ((SPI1->SR & SPI_SR_RXNE) == 0)
    {
        continue;
    }

    // Read data register
    value = SPI1->DR;

    return value;
}

static void bc_spirit1_hal_init_spi(void)
{
    // Enable clock for SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Software slave management, baud rate control = fPCLK / 8, master configuration
    SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1 | SPI_CR1_MSTR;

    // Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

static void bc_spirit1_hal_init_timer(void)
{
    // Enable clock for TIM7
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;

    // Enable one-pulse mode
    TIM7->CR1 |= TIM_CR1_OPM;
}
#endif
