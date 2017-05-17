#include <bc_i2c.h>
#include <bc_module_core.h>
#include <bc_tick.h>
#include <stm32l0xx.h>

#define _BC_I2C_TX_TIMEOUT_ADJUST_FACTOR 1.5
#define _BC_I2C_RX_TIMEOUT_ADJUST_FACTOR 1.5

#define _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT    1
#define _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT   2
#define _BC_I2C_RELOAD_MODE                I2C_CR2_RELOAD
#define _BC_I2C_AUTOEND_MODE               I2C_CR2_AUTOEND
#define _BC_I2C_SOFTEND_MODE               (0x00000000U)
#define _BC_I2C_NO_STARTSTOP               (0x00000000U)
#define _BC_I2C_GENERATE_START_WRITE       I2C_CR2_START
#define _BC_I2C_BYTE_TRANSFER_TIME_100     80
#define _BC_I2C_BYTE_TRANSFER_TIME_400     20

static struct
{
    bool i2c0_initialized;
    bool i2c1_initialized;

    bc_i2c_speed_t i2c0_speed;
    bc_i2c_speed_t i2c1_speed;

} _bc_i2c = { .i2c0_initialized = false, .i2c1_initialized = false };

static bool _bc_i2c_mem_write(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length);
static bool _bc_i2c_mem_read(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length);
static inline bool _bc_i2c_req_mem_write(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length);
static inline bool _bc_i2c_req_mem_read(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length);
static void _bc_i2c_config(I2C_TypeDef *i2c, uint8_t device_address, uint8_t length, uint32_t mode, uint32_t Request);
static bool _bc_i2c_watch_flag(I2C_TypeDef *i2c, uint32_t flag, FlagStatus status);
static inline bool _bc_i2c_ack_failed(I2C_TypeDef *i2c);
static bool _bc_i2c_read(I2C_TypeDef *i2c, uint8_t device_address, const void *buffer, size_t length);
static bool _bc_i2c_write(I2C_TypeDef *i2c, const void *buffer, size_t length);
uint32_t bc_i2c_get_timeout(bc_i2c_channel_t channel, size_t length);
static void _bc_i2c_timeout_begin(uint32_t timeout_us);
static inline bool _bc_i2c_timeout_is_too_late();


void bc_i2c_init(bc_i2c_channel_t channel, bc_i2c_speed_t speed)
{
    if (channel == BC_I2C_I2C0)
    {
        if (_bc_i2c.i2c0_initialized)
        {
            return;
        }

        // Initialize I2C2 pins
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

        // Errata workaround
        RCC->IOPENR;

        GPIOB->MODER &= ~(GPIO_MODER_MODE10_0 | GPIO_MODER_MODE11_0);
        GPIOB->OTYPER |= GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_11;
        GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED10 | GPIO_OSPEEDER_OSPEED11;
        GPIOB->PUPDR |= GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0;
        GPIOB->AFR[1] |= 6 << GPIO_AFRH_AFRH3_Pos | 6 << GPIO_AFRH_AFRH2_Pos;

        // Enable I2C2 peripheral clock
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

        // Errata workaround
        RCC->APB1ENR;

        // Enable I2C2 peripheral
        I2C2->CR1 |= I2C_CR1_PE;

        bc_i2c_set_speed(channel, speed);

        // Update state
        _bc_i2c.i2c0_initialized = true;
    }
    else
    {
        if (_bc_i2c.i2c1_initialized)
        {
            return;
        }

        // Initialize I2C1 pins
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

        // Errata workaround
        RCC->IOPENR;

        GPIOB->MODER &= ~(GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0);
        GPIOB->OTYPER |= GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9;
        GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED8 | GPIO_OSPEEDER_OSPEED9;
        GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0;
        GPIOB->AFR[1] |= 4 << GPIO_AFRH_AFRH1_Pos | 4 << GPIO_AFRH_AFRH0_Pos;

        // Enable I2C1 peripheral clock
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

        // Errata workaround
        RCC->APB1ENR;

        // Enable I2C1 peripheral
        I2C1->CR1 |= I2C_CR1_PE;

        bc_i2c_set_speed(channel, speed);

        // Update state
        _bc_i2c.i2c1_initialized = true;
    }

    // Enable clock for TIM6
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    // Enable one-pulse mode
    TIM6->CR1 |= TIM_CR1_OPM;
}

bc_i2c_speed_t bc_i2c_get_speed(bc_i2c_channel_t channel)
{
    if(channel == BC_I2C_I2C0)
    {
        return _bc_i2c.i2c0_speed;
    }
    else
    {
        return _bc_i2c.i2c1_speed;
    }
}

void bc_i2c_set_speed(bc_i2c_channel_t channel, bc_i2c_speed_t speed)
{
    uint32_t timingr;

    if (speed == BC_I2C_SPEED_400_KHZ)
    {
        timingr = 0x301d1d;
    }
    else
    {
        timingr = 0x709595;
    }

    if (channel == BC_I2C_I2C0)
    {
        I2C2->CR1 &= ~I2C_CR1_PE;
        I2C2->TIMINGR = timingr;
        _bc_i2c.i2c0_speed = speed;
        I2C2->CR1 |= I2C_CR1_PE;
    }
    else
    {
        I2C1->CR1 &= ~I2C_CR1_PE;
        I2C1->TIMINGR = timingr;
        _bc_i2c.i2c1_speed = speed;
        I2C1->CR1 |= I2C_CR1_PE;
    }
}

bool bc_i2c_write(bc_i2c_channel_t channel, const bc_i2c_transfer_t *transfer)
{
    I2C_TypeDef *i2c;
    uint32_t timeout_us;

    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        i2c = I2C2;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        i2c = I2C1;
    }

    bool status = false;

    bc_module_core_pll_enable();

    timeout_us = _BC_I2C_TX_TIMEOUT_ADJUST_FACTOR * bc_i2c_get_timeout(channel, transfer->length);

    _bc_i2c_timeout_begin(timeout_us);

    // Wait until bus is not busy
    if (_bc_i2c_watch_flag(i2c, I2C_ISR_BUSY, SET))
    {
        _bc_i2c_config(i2c, transfer->device_address << 1, transfer->length, I2C_CR2_AUTOEND, _BC_I2C_GENERATE_START_WRITE);

        // Wait until TXIS flag is set
        if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TXIS, RESET))
        {
            bc_module_core_pll_disable();

            return false;
        }

        status = _bc_i2c_write(i2c, transfer->buffer, transfer->length);
    }

    bc_module_core_pll_disable();

    return status;
}

bool bc_i2c_read(bc_i2c_channel_t channel, const bc_i2c_transfer_t *transfer)
{
    I2C_TypeDef *i2c;
    uint32_t timeout_us;

    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        i2c = I2C2;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        i2c = I2C1;
    }

    bool status = false;

    bc_module_core_pll_enable();

    timeout_us = _BC_I2C_RX_TIMEOUT_ADJUST_FACTOR * bc_i2c_get_timeout(channel, transfer->length);

    _bc_i2c_timeout_begin(timeout_us);

    // Wait until bus is not busy
    if (_bc_i2c_watch_flag(i2c, I2C_ISR_BUSY, SET))
    {
        status = _bc_i2c_read(i2c, transfer->device_address << 1, transfer->buffer, transfer->length);
    }

    bc_module_core_pll_disable();

    return status;
}

bool bc_i2c_memory_write(bc_i2c_channel_t channel, const bc_i2c_memory_transfer_t *transfer)
{
    uint16_t transfer_memory_address_length;
    I2C_TypeDef *i2c;
    uint32_t timeout_us;

    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        i2c = I2C2;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        i2c = I2C1;
    }

    // Enable PLL and disable sleep
    bc_module_core_pll_enable();

    transfer_memory_address_length = (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT : _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT;

    timeout_us = _BC_I2C_TX_TIMEOUT_ADJUST_FACTOR * bc_i2c_get_timeout(channel, transfer->length);

    _bc_i2c_timeout_begin(timeout_us);

    if (!_bc_i2c_mem_write(i2c, transfer->device_address << 1, transfer->memory_address, transfer_memory_address_length, transfer->buffer, transfer->length))
    {
        // Disable PLL and enable sleep
        bc_module_core_pll_disable();

        return false;
    }

    // Disable PLL and enable sleep
    bc_module_core_pll_disable();

    return true;
}

bool bc_i2c_memory_read(bc_i2c_channel_t channel, const bc_i2c_memory_transfer_t *transfer)
{
    uint16_t transfer_memory_address_length;
    I2C_TypeDef *i2c;
    uint32_t timeout_us;

    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        i2c = I2C2;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        i2c = I2C1;
    }

    // Enable PLL and disable sleep
    bc_module_core_pll_enable();

    transfer_memory_address_length = (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT : _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT;

    timeout_us = _BC_I2C_RX_TIMEOUT_ADJUST_FACTOR * bc_i2c_get_timeout(channel, transfer->length);

    _bc_i2c_timeout_begin(timeout_us);

    if (!_bc_i2c_mem_read(i2c, transfer->device_address << 1, transfer->memory_address, transfer_memory_address_length, transfer->buffer, transfer->length))
    {
        // Disable PLL and enable sleep
        bc_module_core_pll_disable();

        return false;
    }

    // Disable PLL and enable sleep
    bc_module_core_pll_disable();

    return true;
}

bool bc_i2c_memory_write_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t data)
{
    bc_i2c_memory_transfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = &data;
    transfer.length = 1;

    return bc_i2c_memory_write(channel, &transfer);
}

bool bc_i2c_memory_write_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t data)
{
    uint8_t buffer[2];

    buffer[0] = data >> 8;
    buffer[1] = data;

    bc_i2c_memory_transfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = buffer;
    transfer.length = 2;

    return bc_i2c_memory_write(channel, &transfer);
}

bool bc_i2c_memory_read_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t *data)
{
    bc_i2c_memory_transfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = data;
    transfer.length = 1;

    return bc_i2c_memory_read(channel, &transfer);
}

bool bc_i2c_memory_read_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t *data)
{
    uint8_t buffer[2];

    bc_i2c_memory_transfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = buffer;
    transfer.length = 2;

    if (!bc_i2c_memory_read(channel, &transfer))
    {
        return false;
    }

    *data = buffer[0] << 8 | buffer[1];

    return true;
}

static bool _bc_i2c_mem_write(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length)
{
    // Wait until bus is not busy
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_BUSY, SET))
    {
        return false;
    }

    // Send slave address and memory address
    if (!_bc_i2c_req_mem_write(i2c, device_address, memory_address, memory_address_length))
    {
        return false;
    }

    // Set size of data to write
    _bc_i2c_config(i2c, device_address, length, _BC_I2C_AUTOEND_MODE, _BC_I2C_NO_STARTSTOP);

    // Perform I2C transfer
    return _bc_i2c_write(i2c, buffer, length);
}

static bool _bc_i2c_mem_read(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length)
{
    // Wait until bus is not busy
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_BUSY, SET))
    {
        return false;
    }

    // Send slave address and memory address
    if (!_bc_i2c_req_mem_read(i2c, device_address, memory_address, memory_address_length))
    {
        return false;
    }

    // Perform I2C transfer
    return _bc_i2c_read(i2c, device_address, buffer, length);
}

static inline bool _bc_i2c_req_mem_write(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length)
{
    _bc_i2c_config(i2c, device_address, memory_address_length, _BC_I2C_RELOAD_MODE, _BC_I2C_GENERATE_START_WRITE);

    // Wait until TXIS flag is set
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TXIS, RESET))
    {
        return false;
    }

    // If memory address size is 16Bit
    if (memory_address_length == _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT)
    {
        // Send MSB of memory address
        i2c->TXDR = (memory_address >> 8) & 0xff;

        // Wait until TXIS flag is set
        if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TXIS, RESET))
        {
            return false;
        }
    }

    // Send LSB of memory address
    i2c->TXDR = memory_address & 0xff;

    // Wait until TCR flag is set
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TCR, RESET))
    {
        return false;
    }

    return true;
}

static inline bool _bc_i2c_req_mem_read(I2C_TypeDef *i2c, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length)
{
    _bc_i2c_config(i2c, device_address, memory_address_length, _BC_I2C_SOFTEND_MODE, _BC_I2C_GENERATE_START_WRITE);

    // Wait until TXIS flag is set
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TXIS, RESET))
    {
        return false;
    }

    // If memory address size is 16Bit
    if (memory_address_length == _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT)
    {
        // Send MSB of memory address
        i2c->TXDR = (memory_address >> 8) & 0xff;

        // Wait until TXIS flag is set
        if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TXIS, RESET))
        {
            return false;
        }
    }

    // Send LSB of memory address
    i2c->TXDR = memory_address & 0xff;

    // Wait until TC flag is set
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TC, RESET))
    {
        return false;
    }

    return true;
}

static void _bc_i2c_config(I2C_TypeDef *i2c, uint8_t device_address, uint8_t length, uint32_t mode, uint32_t Request)
{
    uint32_t reg;

    // Get the CR2 register value
    reg = i2c->CR2;

    // clear tmpreg specific bits
    reg &= ~(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP);

    // update tmpreg
    reg |= (device_address & I2C_CR2_SADD) | (length << I2C_CR2_NBYTES_Pos) | mode | Request;

    // update CR2 register
    i2c->CR2 = reg;
}

static bool _bc_i2c_watch_flag(I2C_TypeDef *i2c, uint32_t flag, FlagStatus status)
{
    while ((i2c->ISR & flag) == status)
    {
        if ((flag == I2C_ISR_STOPF) || (flag == I2C_ISR_TXIS))
        {
            // Check if a NACK is not detected ...
            if (!_bc_i2c_ack_failed(i2c))
            {
                return false;
            }
        }

        if (_bc_i2c_timeout_is_too_late())
        {
            return false;
        }
    }
    return true;
}

static inline bool _bc_i2c_ack_failed(I2C_TypeDef *i2c)
{
    if ((i2c->ISR & I2C_ISR_NACKF) != 0)
    {
        // Wait until STOP flag is reset
        // AutoEnd should be initialized after AF
        while ((i2c->ISR & I2C_ISR_STOPF) == 0)
        {
            if (_bc_i2c_timeout_is_too_late())
            {
                return false;
            }
        }

        // Clear NACKF flag
        i2c->ICR = I2C_ISR_NACKF;

        // Clear STOP flag
        i2c->ICR = I2C_ISR_STOPF;

        // If a pending TXIS flag is set ...
        if ((i2c->ISR & I2C_ISR_TXIS) != 0)
        {
            // ... write a dummy data in TXDR to clear it
            i2c->TXDR = 0;
        }

        // Flush TX register if not empty
        if ((i2c->ISR & I2C_ISR_TXE) == 0)
        {
            i2c->ISR |= I2C_ISR_TXE;
        }

        // Clear Configuration Register 2
        i2c->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);

        return false;
    }

    return true;
}

static bool _bc_i2c_read(I2C_TypeDef *i2c, uint8_t device_address, const void *buffer, size_t length)
{
    uint8_t *p = (uint8_t *) buffer;

    // Set size of data to read
    _bc_i2c_config(i2c, device_address, length, I2C_CR2_AUTOEND, I2C_CR2_START | I2C_CR2_RD_WRN);

    while (length > 0)
    {
        // Wait until RXNE flag is set
        if (!_bc_i2c_watch_flag(i2c, I2C_ISR_RXNE, RESET))
        {
            return false;
        }

        // Read data from RXDR
        *p++ = i2c->RXDR;

        length--;
    }

    // No need to Check TC flag, with AUTOEND mode the stop is automatically generated

    // Wait until STOPF flag is reset
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_STOPF, RESET))
    {
        return false;
    }

    // Clear STOP flag
    i2c->ICR = I2C_ICR_STOPCF;

    // Clear Configuration Register 2
    i2c->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);

    return true;
}

uint32_t bc_i2c_get_timeout(bc_i2c_channel_t channel, size_t length)
{
    if(bc_i2c_get_speed(channel) == BC_I2C_SPEED_100_KHZ)
    {
        return _BC_I2C_BYTE_TRANSFER_TIME_100 * (length + 3);
    }
    else
    {
        return _BC_I2C_BYTE_TRANSFER_TIME_400 * (length + 3);
    }


}

static bool _bc_i2c_write(I2C_TypeDef *i2c, const void *buffer, size_t length)
{
    uint8_t *p = (uint8_t *) buffer;

    while (length > 0)
    {
        // Wait until TXIS flag is set
        if (!_bc_i2c_watch_flag(i2c, I2C_ISR_TXIS, RESET))
        {
            return false;
        }

        // Write data to TXDR
        i2c->TXDR = *p++;

        length--;
    }

    // No need to Check TC flag, with AUTOEND mode the stop is automatically generated

    // Wait until STOPF flag is reset
    if (!_bc_i2c_watch_flag(i2c, I2C_ISR_STOPF, RESET))
    {
        return false;
    }

    // Clear STOP flag
    i2c->ICR = I2C_ICR_STOPCF;

    // Clear Configuration Register 2
    i2c->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);

    return true;
}

void _bc_i2c_timeout_begin(uint32_t timeout_us)
{
    int shift;

    for (shift = 0; timeout_us > 0xffff; shift++)
    {
        timeout_us >>= 1;
    }

    // Disable counter
    TIM6->CR1 &= ~TIM_CR1_CEN;

    // Clear CNT register
    TIM6->CNT = 0;

    // Set prescaler
    TIM6->PSC = 32 << shift;

    // Set auto-reload register
    TIM6->ARR = timeout_us;

    // Generate update of registers
    TIM6->EGR = TIM_EGR_UG;

    // Enable counter
    TIM6->CR1 |= TIM_CR1_CEN;
}

bool _bc_i2c_timeout_is_too_late()
{
    if ((TIM6->CR1 & TIM_CR1_CEN) == 0)
    {
        return true;
    }
    return false;
}
