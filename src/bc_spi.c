#include <bc_spi.h>
#include <stm32l083xx.h>

#include <stm32l0xx_hal.h>

// cs ... p15
// sclk ... p14
// mosi ... p13
// miso ... p12

typedef struct
{
    uint32_t CLKPolarity;
    uint32_t CLKPhase;
} _bc_spi_mode_t;   


/*
    BC_SPI_SPEED_1_MHZ = 0L, //!< SPI communication speed is 1 MHz
    BC_SPI_SPEED_2_MHZ = 1L, //!< SPI communication speed is 2 MHz
    BC_SPI_SPEED_4_MHZ = 2L, //!< SPI communication speed is 4 MHz
    BC_SPI_SPEED_8_MHZ = 3L  //!< SPI communication speed is 8 MHz
*/
/*
static uint32_t _bc_spi_speed_table[4] = {
    [0] = SPI_CR1_BR_1 | SPI_CR1_BR_0,  // :16
    [1] = SPI_CR1_BR_1,                 // :8
    [2] = SPI_CR1_BR_0,                 // :4
    [3] = 0x00,                         // :2
};
*/

static uint32_t _bc_spi_speed_table[4] = {
    [BC_SPI_SPEED_1_MHZ] = 0x18, // :16 (2MHz)
    [BC_SPI_SPEED_2_MHZ] = 0x10, // :8 (4MHz)
    [BC_SPI_SPEED_4_MHZ] = 0x08, // :4 (8MHz)
    [BC_SPI_SPEED_8_MHZ] = 0x00, // :2 (16MHz)
};

// TODO ... dirty
static uint32_t _bc_spi_mode_table[4] = {
    [BC_SPI_MODE_0] = 0x00 ,
    [BC_SPI_MODE_1] = 0x01 ,
    [BC_SPI_MODE_2] = 0x02 ,
    [BC_SPI_MODE_3] = 0x03
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

static uint8_t _bc_spi_transfer_byte(uint8_t value)
{
    // TODO ... future trouble

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
    SPI2->CR1 &= ~0x03;
    SPI2->CR1 |= _bc_spi_mode_table[mode];
    SPI2->CR1 = temp;
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
