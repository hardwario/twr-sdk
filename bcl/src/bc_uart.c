#include <bc_uart.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

static void bc_uart_init_uart1(bc_uart_config_t config);

static size_t bc_uart_write_uart1(const void *buffer, size_t length);

static size_t bc_uart_read_uart1(void *buffer, size_t length, bc_tick_t timeout);

void bc_uart_init(bc_uart_channel_t channel, bc_uart_config_t config)
{
    if (channel == BC_UART_UART1)
    {
        bc_uart_init_uart1(config);
    }
}

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length)
{
    if (channel == BC_UART_UART1)
    {
        return bc_uart_write_uart1(buffer, length);
    }

    return 0;
}

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout)
{
    if (channel == BC_UART_UART1)
    {
        return bc_uart_read_uart1(buffer, length, timeout);
    }

    return 0;
}

static void bc_uart_init_uart1(bc_uart_config_t config)
{
    // Enable GPIOA clock
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Errata workaround
    RCC->IOPENR;

    // Enable pull-up on RXD1 pin
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;

    // Configure TXD1 and RXD1 pins as alternate function
    GPIOA->MODER &= ~(1 << GPIO_MODER_MODE3_Pos | 1 << GPIO_MODER_MODE2_Pos);

    // Select AF6 alternate function for TXD1 and RXD1 pins
    GPIOA->AFR[0] |= GPIO_AF6_LPUART1 << GPIO_AFRL_AFRL3_Pos | GPIO_AF6_LPUART1 << GPIO_AFRL_AFRL2_Pos;

    // Set LSE as LPUART1 clock source
    RCC->CCIPR |= RCC_CCIPR_LPUART1SEL_1 | RCC_CCIPR_LPUART1SEL_0;

    // Enable clock for LPUART1
    RCC->APB1ENR |= RCC_APB1ENR_LPUART1EN;

    // Errata workaround
    RCC->APB1ENR;

    // Enable transmitter and receiver
    LPUART1->CR1 = USART_CR1_TE | USART_CR1_RE;

    // Disable overrun detection
    LPUART1->CR3 = USART_CR3_OVRDIS;

    // Configure baudrate
    LPUART1->BRR = 0x369;

    // Enable LPUART1
    LPUART1->CR1 |= USART_CR1_UE;
}

static size_t bc_uart_write_uart1(const void *buffer, size_t length)
{
    size_t bytes_written = 0;

    bc_module_core_pll_enable();

    while (bytes_written != length)
    {
        // Until transmit data register is not empty...
        while ((LPUART1->ISR & USART_ISR_TXE) == 0)
        {
            continue;
        }

        // Load transmit data register
        LPUART1->TDR = *((uint8_t *) buffer + bytes_written++);
    }

    // Until transmission is not complete...
    while ((LPUART1->ISR & USART_ISR_TC) == 0)
    {
        continue;
    }

    bc_module_core_pll_disable();

    return bytes_written;
}

static size_t bc_uart_read_uart1(void *buffer, size_t length, bc_tick_t timeout)
{
    size_t bytes_read = 0;

    bc_module_core_pll_enable();

    bc_tick_t tick_timeout = timeout == BC_TICK_INFINITY ? BC_TICK_INFINITY : bc_tick_get() + timeout;

    while (bytes_read != length)
    {
        // If timeout condition is met...
        if (bc_tick_get() >= tick_timeout)
        {
            break;
        }

        // If receive data register is empty...
        if ((LPUART1->ISR & USART_ISR_RXNE) == 0)
        {
            continue;
        }

        // Read receive data register
        *((uint8_t *) buffer + bytes_read++) = LPUART1->RDR;
    }

    bc_module_core_pll_disable();

    return bytes_read;
}
