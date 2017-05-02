#include <bc_i2c.h>
#include <bc_module_core.h>
#include <bc_tick.h>
#include <stm32l0xx.h>

#define _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT    1
#define _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT   2
#define _BC_I2C_RELOAD_MODE                I2C_CR2_RELOAD
#define _BC_I2C_AUTOEND_MODE               I2C_CR2_AUTOEND
#define _BC_I2C_SOFTEND_MODE               (0x00000000U)
#define _BC_I2C_NO_STARTSTOP               (0x00000000U)
#define _BC_I2C_GENERATE_START_WRITE       I2C_CR2_START

static inline bool _bc_i2c_mem_write(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length);
static inline bool _bc_i2c_mem_read(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length);
static inline bool _bc_i2c_req_mem_write(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, bc_tick_t timeout, bc_tick_t *tick_start);
static inline bool _bc_i2c_req_mem_read(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, bc_tick_t timeout, bc_tick_t *tick_start);
static void _bc_i2c_config(I2C_TypeDef *I2Cx, uint8_t device_address, uint8_t Size, uint32_t mode, uint32_t Request);
static bool _bc_i2c_watch_flag(I2C_TypeDef *I2Cx, uint32_t flag, FlagStatus status, bc_tick_t timeout, bc_tick_t *tick_start);
static inline bool _bc_i2c_ack_failed(I2C_TypeDef *I2Cx, bc_tick_t timeout, bc_tick_t *tick_start);
static inline bool _bc_i2c_read(I2C_TypeDef *I2Cx, uint8_t device_address, uint8_t *buffer, size_t length, bc_tick_t *tick_start);
static inline bool _bc_i2c_write(I2C_TypeDef *I2Cx, uint8_t *buffer, size_t length, bc_tick_t *tick_start);

static struct
{
    bool i2c0_initialized;
    bool i2c1_initialized;

} _bc_i2c = {.i2c0_initialized = false, .i2c1_initialized = false};

void bc_i2c_init(bc_i2c_channel_t channel, bc_i2c_speed_t speed)
{
    if (channel == BC_I2C_I2C0)
    {
        if (_bc_i2c.i2c0_initialized)
        {
            return;
        }

        // Initialize I2C0 pins
        RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
        GPIOB->MODER &= ~(GPIO_MODER_MODE10_0 | GPIO_MODER_MODE11_0);
        GPIOB->OTYPER |= GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_11;
        GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED10 | GPIO_OSPEEDER_OSPEED11;
        GPIOB->PUPDR |= GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD11_0;
        GPIOB->AFR[1] &= 0xffff00ff;
        GPIOB->AFR[1] |= 0x6600;

        // Enable I2C0 peripheral
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

        // Initialize I2C0 peripheral
        I2C2->CR1 &= I2C_CR1_PE;
        I2C2->CR2 = I2C_CR2_AUTOEND;
        I2C2->OAR1 = I2C_OAR1_OA1EN;

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
        GPIOB->MODER &= ~(GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0);
        GPIOB->OTYPER |= GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9;
        GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED8 | GPIO_OSPEEDER_OSPEED9;
        GPIOB->PUPDR |= GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0;
        GPIOB->AFR[1] &= 0xffffff00;
        GPIOB->AFR[1] |= 0x44;

        // Enable I2C1 peripheral
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

        // Initialize I2C1 peripheral
        I2C1->CR1 &= I2C_CR1_PE;
        I2C1->CR2 = I2C_CR2_AUTOEND;
        I2C1->OAR1 = I2C_OAR1_OA1EN;

        bc_i2c_set_speed(channel, speed);

        // Update state
        _bc_i2c.i2c1_initialized = true;
    }
}

bool bc_i2c_set_speed(bc_i2c_channel_t channel, bc_i2c_speed_t speed)
{
    uint32_t timingr;

    if (speed == BC_I2C_SPEED_400_KHZ)
    {
        timingr = 0x301110;
    }
    else if (speed == BC_I2C_SPEED_100_KHZ)
    {
        timingr = 0x707cbb;
    }
    else
    {
        return false;
    }

    if (channel == BC_I2C_I2C0)
    {
        I2C2->CR1 &= ~I2C_CR1_PE;
        I2C2->TIMINGR = timingr;
        I2C2->CR1 |= I2C_CR1_PE;
    }
    else
    {
        I2C1->CR1 &= ~I2C_CR1_PE;
        I2C1->TIMINGR = timingr;
        I2C1->CR1 |= I2C_CR1_PE;
    }

    return true;
}

bool bc_i2c_write(bc_i2c_channel_t channel, const bc_i2c_tranfer_t *transfer)
{
    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        // Enable PLL and disable sleep
        bc_module_core_pll_enable();

        if (!_bc_i2c_mem_write(I2C2, transfer->device_address << 1, transfer->memory_address,
                (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT : _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT, transfer->buffer, transfer->length))
        {
            // Disable PLL and enable sleep
            bc_module_core_pll_disable();

            return false;
        }

        // Disable PLL and enable sleep
        bc_module_core_pll_disable();

        return true;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        // Enable PLL and disable sleep
        bc_module_core_pll_enable();

        if (!_bc_i2c_mem_write(I2C1, transfer->device_address << 1, transfer->memory_address,
                (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT : _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT, transfer->buffer, transfer->length))
        {
            // Disable PLL and enable sleep
            bc_module_core_pll_disable();

            return false;
        }

        // Disable PLL and enable sleep
        bc_module_core_pll_disable();

        return true;
    }
}

bool bc_i2c_write_raw(bc_i2c_channel_t channel, uint8_t device_address, void *buffer, size_t length)
{
    I2C_TypeDef *I2Cx;

    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        I2Cx = I2C2;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        I2Cx = I2C1;
    }

    bool status = false;

    bc_tick_t tick_start = bc_tick_get();

    bc_module_core_pll_enable();

    if  (_bc_i2c_watch_flag(I2Cx, I2C_ISR_BUSY, SET, 25, &tick_start))
    {
        _bc_i2c_config(I2Cx, device_address << 1, length, I2C_CR2_AUTOEND, _BC_I2C_GENERATE_START_WRITE);

        /* Wait until TXIS flag is set */
        if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_TXIS, RESET, 10, &tick_start))
        {
            return false;
        }

        status = _bc_i2c_write(I2Cx, buffer, length, &tick_start);
    }

    bc_module_core_pll_disable();

    return status;
}

bool bc_i2c_read(bc_i2c_channel_t channel, const bc_i2c_tranfer_t *transfer)
{
    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        // Enable PLL and disable sleep
        bc_module_core_pll_enable();

        if (!_bc_i2c_mem_read(I2C2, transfer->device_address << 1, transfer->memory_address,
                (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT : _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT, transfer->buffer, transfer->length))
        {
            // Disable PLL and enable sleep
            bc_module_core_pll_disable();

            return false;
        }

        // Disable PLL and enable sleep
        bc_module_core_pll_disable();

        return true;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        // Enable PLL and disable sleep
        bc_module_core_pll_enable();

        if (!_bc_i2c_mem_read(I2C1, transfer->device_address << 1, transfer->memory_address,
                (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? _BC_I2C_MEMORY_ADDRESS_SIZE_16BIT : _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT, transfer->buffer, transfer->length))
        {
            // Disable PLL and enable sleep
            bc_module_core_pll_disable();

            return false;
        }

        // Disable PLL and enable sleep
        bc_module_core_pll_disable();

        return true;
    }
}

bool bc_i2c_read_raw(bc_i2c_channel_t channel, uint8_t device_address, void *buffer, size_t length)
{
    I2C_TypeDef *I2Cx;

    if (channel == BC_I2C_I2C0)
    {
        if (!_bc_i2c.i2c0_initialized)
        {
            return false;
        }

        I2Cx = I2C2;
    }
    else
    {
        if (!_bc_i2c.i2c1_initialized)
        {
            return false;
        }

        I2Cx = I2C1;
    }

    bc_tick_t tick_start = bc_tick_get();

    bool status = false;

    bc_module_core_pll_enable();

    if  (_bc_i2c_watch_flag(I2Cx, I2C_ISR_BUSY, SET, 25, &tick_start))
    {
        status = _bc_i2c_read(I2Cx, device_address << 1, buffer, length, &tick_start);
    }

    bc_module_core_pll_disable();

    return status;
}

bool bc_i2c_write_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t data)
{
    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = &data;
    transfer.length = 1;

    return bc_i2c_write(channel, &transfer);
}

bool bc_i2c_write_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t data)
{
    uint8_t buffer[2];

    buffer[0] = data >> 8;
    buffer[1] = data;

    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = buffer;
    transfer.length = 2;

    return bc_i2c_write(channel, &transfer);
}

bool bc_i2c_read_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t *data)
{
    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = data;
    transfer.length = 1;

    return bc_i2c_read(channel, &transfer);
}

bool bc_i2c_read_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t *data)
{
    uint8_t buffer[2];

    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = buffer;
    transfer.length = 2;

    if (!bc_i2c_read(channel, &transfer))
    {
        return false;
    }

    *data = buffer[0] << 8 | buffer[1];

    return true;
}

static inline bool _bc_i2c_mem_write(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length)
{
    bc_tick_t tick_start = bc_tick_get();

    // Wait till I2Cx is busy
    if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_BUSY, SET, 25, &tick_start))
    {
        return false;
    }

    // Send Slave Address and Memory Address
    if (!_bc_i2c_req_mem_write(I2Cx, device_address, memory_address, memory_address_length, 10, &tick_start))
    {
        return false;
    }

    // Set size of data to write
    _bc_i2c_config(I2Cx, device_address, length, _BC_I2C_AUTOEND_MODE, _BC_I2C_NO_STARTSTOP);

    return _bc_i2c_write(I2Cx, buffer, length, &tick_start);
}

static inline bool _bc_i2c_mem_read(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, uint8_t *buffer, uint16_t length)
{
    bc_tick_t tick_start = bc_tick_get();

    if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_BUSY, SET, 25, &tick_start))
    {
        return false;
    }

    /* Send Slave Address and Memory Address */
    if (!_bc_i2c_req_mem_read(I2Cx, device_address, memory_address, memory_address_length, 10, &tick_start))
    {
        return false;
    }

    return _bc_i2c_read(I2Cx, device_address, buffer, length, &tick_start);
}

static inline bool _bc_i2c_req_mem_write(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, bc_tick_t timeout, bc_tick_t *tick_start)
{
    _bc_i2c_config(I2Cx, device_address, memory_address_length, _BC_I2C_RELOAD_MODE, _BC_I2C_GENERATE_START_WRITE);

    /* Wait until TXIS flag is set */
    if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_TXIS, RESET, timeout, tick_start))
    {
        return false;
    }

    /* If Memory address size is 8Bit */
    if (memory_address_length == _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT)
    {
        /* Send Memory Address */
        I2Cx->TXDR = memory_address & 0xff;
    }
    /* If Memory address size is 16Bit */
    else
    {
        /* Send MSB of Memory Address */
        I2Cx->TXDR = (memory_address >> 8) & 0xff;

        /* Wait until TXIS flag is set */
        if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_TXIS, RESET, timeout, tick_start))
        {
            return false;
        }

        /* Send LSB of Memory Address */
        I2Cx->TXDR = memory_address & 0xff;
    }

    /* Wait until TCR flag is set */
    if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_TCR, RESET, timeout, tick_start))
    {
        return false;
    }

    return true;
}

static inline bool _bc_i2c_req_mem_read(I2C_TypeDef *I2Cx, uint8_t device_address, uint16_t memory_address, uint16_t memory_address_length, bc_tick_t timeout, bc_tick_t *tick_start)
{
    _bc_i2c_config(I2Cx, device_address, memory_address_length, _BC_I2C_SOFTEND_MODE, _BC_I2C_GENERATE_START_WRITE);

    /* Wait until TXIS flag is set */
    if (_bc_i2c_watch_flag(I2Cx, I2C_ISR_TXIS, RESET, timeout, tick_start) != true)
    {
        return false;
    }

    /* If Memory address size is 8Bit */
    if (memory_address_length == _BC_I2C_MEMORY_ADDRESS_SIZE_8BIT)
    {
        /* Send Memory Address */
        I2Cx->TXDR = memory_address & 0xff;
    }
    /* If Memory address size is 16Bit */
    else
    {
        /* Send MSB of Memory Address */
        I2Cx->TXDR = (memory_address >> 8) & 0xff;

        /* Wait until TXIS flag is set */
        if (_bc_i2c_watch_flag(I2Cx, I2C_ISR_TXIS, RESET, timeout, tick_start) != true)
        {
            return false;
        }

        /* Send LSB of Memory Address */
        I2Cx->TXDR = memory_address & 0xff;
    }

    /* Wait until TC flag is set */
    if (_bc_i2c_watch_flag(I2Cx, I2C_ISR_TC, RESET, timeout, tick_start) != true)
    {
        return false;
    }

    return true;
}

static void _bc_i2c_config(I2C_TypeDef *I2Cx, uint8_t device_address, uint8_t Size, uint32_t mode, uint32_t Request)
{
    uint32_t reg = 0U;

    /* Get the CR2 register value */
    reg = I2Cx->CR2;

    /* clear tmpreg specific bits */
    reg &= (uint32_t) ~((uint32_t) (I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));

    /* update tmpreg */
    reg |= (uint32_t) (((uint32_t) device_address & I2C_CR2_SADD) | (((uint32_t) Size << 16) & I2C_CR2_NBYTES) |
            (uint32_t) mode | (uint32_t) Request);

    /* update CR2 register */
    I2Cx->CR2 = reg;
}

static bool _bc_i2c_watch_flag(I2C_TypeDef *I2Cx, uint32_t flag, FlagStatus status, bc_tick_t timeout, bc_tick_t *tick_start)
{

    timeout += *tick_start;

    while ((I2Cx->ISR & flag) == status)
    {
        if ((flag == I2C_ISR_STOPF) || (flag == I2C_ISR_TXIS))
        {
            /* Check if a NACK is detected */
            if (_bc_i2c_ack_failed(I2Cx, timeout, tick_start) != true)
            {
                return false;
            }
        }

        if (bc_tick_get() > timeout)
        {
            return false;
        }
    }
    return true;
}

static inline bool _bc_i2c_ack_failed(I2C_TypeDef *I2Cx, bc_tick_t timeout, bc_tick_t *tick_start)
{
    timeout += *tick_start;

    if ((I2Cx->ISR & I2C_ISR_NACKF) == true)
    {
        /* Wait until STOP Flag is reset */
        /* AutoEnd should be initiate after AF */
        while ((I2Cx->ISR & I2C_ISR_STOPF) == 0)
        {
            if (bc_tick_get() > timeout)
            {
                return false;
            }
        }

        /* Clear NACKF Flag */
        I2Cx->ICR = I2C_ISR_NACKF;

        /* Clear STOP Flag */
        I2Cx->ICR = I2C_ISR_STOPF;

        /* Flush TX register */
        /* If a pending TXIS flag is set */
        /* Write a dummy data in TXDR to clear it */
        if ((I2Cx->ISR & I2C_ISR_TXIS) != 0)
        {
            I2Cx->TXDR = 0x00U;
        }

        /* Flush TX register if not empty */
        if ((I2Cx->ISR & I2C_ISR_TXE) == 0)
        {
            I2Cx->ISR |= I2C_ISR_TXE;
        }

        /* Clear Configuration Register 2 */
        I2Cx->CR2 &= (uint32_t) ~((uint32_t) (I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN));

        return false;
    }
    return true;
}

static inline bool _bc_i2c_read(I2C_TypeDef *I2Cx, uint8_t device_address, uint8_t *buffer, size_t length, bc_tick_t *tick_start)
{
    // Set size of data to read
    _bc_i2c_config(I2Cx, device_address, length, I2C_CR2_AUTOEND, (uint32_t)(I2C_CR2_START | I2C_CR2_RD_WRN));

    while(length > 0U)
    {
        /* Wait until RXNE flag is set */
        if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_RXNE, RESET, 10, tick_start))
        {
            return false;
        }

        /* Read data from RXDR */
        (*buffer++) = I2Cx->RXDR;
        length--;
    }

    /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is reset */
    if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_STOPF, RESET, 10, tick_start))
    {
        return false;
    }

    /* Clear STOP Flag */
    I2Cx->ICR = I2C_ISR_STOPF;

    /* Clear Configuration Register 2 */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN));

    return true;
}

static inline bool _bc_i2c_write(I2C_TypeDef *I2Cx, uint8_t *buffer, size_t length, bc_tick_t *tick_start)
{

    while (length > 0U)
    {
        /* Wait until TXIS flag is set */
        if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_TXIS, RESET, 10, tick_start))
        {
            return false;
        }

        /* Write data to TXDR */
        I2Cx->TXDR = (*buffer++);
        length--;

    }

    /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is reset */
    if (!_bc_i2c_watch_flag(I2Cx, I2C_ISR_STOPF, RESET, 10, tick_start))
    {
        return false;
    }

    /* Clear STOP Flag */
    I2Cx->ICR = I2C_ISR_STOPF;

    /* Clear Configuration Register 2 */
    I2Cx->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN));

    return true;
}
