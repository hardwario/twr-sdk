#include <bc_uart.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

// TODO Implement other UART channels and baudrates

typedef struct
{
    struct
    {
        bc_fifo_t *tx_fifo;
        bc_fifo_t *rx_fifo;

    } channel[3];

} bc_uart_t;

static bc_uart_t _bc_uart;

void bc_uart_init(bc_uart_channel_t channel, bc_uart_param_t *param, bc_fifo_t *tx_fifo, bc_fifo_t *rx_fifo)
{
    // If channel is UART1...
    if (channel == BC_UART_UART1 && param->baudrate == BC_UART_BAUDRATE_9600_BD)
    {
        _bc_uart.channel[1].tx_fifo = tx_fifo;
        _bc_uart.channel[1].rx_fifo = rx_fifo;

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
        LPUART1->CR1 = USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

        // Enable clock in stop mode, disable overrun detection
        LPUART1->CR3 = USART_CR3_UCESM | USART_CR3_WUFIE | USART_CR3_WUS_1 | USART_CR3_WUS_0 | USART_CR3_OVRDIS;

        // Configure baudrate
        LPUART1->BRR = 0x369;

        // Enable LPUART1
        LPUART1->CR1 |= USART_CR1_UE;

        // Enable LPUART1 interrupts
        NVIC_EnableIRQ(LPUART1_IRQn);
    }
}

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length, bc_tick_t timeout)
{
    size_t bytes_written = 0;

    if (timeout == 0)
    {
        // If channel is UART1...
        if (channel == BC_UART_UART1)
        {
            // Write buffer to FIFO
            bytes_written = bc_fifo_write(_bc_uart.channel[1].tx_fifo, buffer, length);

            // Disable interrupts
            bc_irq_disable();

            // Enable transmit interrupts
            LPUART1->CR1 |= USART_CR1_TXEIE;

            // Enable interrupts
            bc_irq_enable();
        }
    }
    else
    {
        // TODO Implement timeout

        // Write buffer to FIFO
        bytes_written = bc_fifo_write(_bc_uart.channel[1].tx_fifo, buffer, length);

        // Disable interrupts
        bc_irq_disable();

        // Enable transmit interrupts
        LPUART1->CR1 |= USART_CR1_TXEIE;

        // Enable interrupts
        bc_irq_enable();

        while ((LPUART1->CR1 & USART_CR1_TXEIE) != 0)
        {
            continue;
        }

        while ((LPUART1->ISR & USART_ISR_TC) == 0)
        {
            continue;
        }
    }

    return bytes_written;
}

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout)
{
    if (timeout == 0)
    {
        // If channel is UART1...
        if (channel == BC_UART_UART1)
        {
            return bc_fifo_read(_bc_uart.channel[1].rx_fifo, buffer, length);
        }

        return 0;
    }

    bc_tick_t tick_timeout = bc_tick_get() + timeout;

    size_t bytes_read = 0;

    // If channel is UART1...
    if (channel == BC_UART_UART1)
    {
        while (length != 0)
        {
            size_t n_bytes = bc_fifo_read(_bc_uart.channel[1].rx_fifo, (uint8_t *) buffer + bytes_read, length);

            bytes_read += n_bytes;

            length -= n_bytes;

            if (bc_tick_get() >= tick_timeout)
            {
                break;
            }

            // TODO Sleep here
        }
    }

    return bytes_read;
}

void AES_RNG_LPUART1_IRQHandler(void)
{
    if ((LPUART1->ISR & USART_ISR_WUF) != 0)
    {
        LPUART1->ICR = USART_ICR_WUCF;
    }

    // If it is TX interrupt...
    if ((LPUART1->ISR & USART_ISR_TXE) != 0)
    {
        uint8_t character;

        // If there are still data in FIFO...
        if (bc_fifo_irq_read(_bc_uart.channel[1].tx_fifo, &character, 1) != 0)
        {
            // Load TX buffer
            LPUART1->TDR = character;
        }
        else
        {
            // Disable TX interrupts
            LPUART1->CR1 &= ~USART_CR1_TXEIE;
        }
    }

    // If it is RX interrupt...
    if ((LPUART1->ISR & USART_ISR_RXNE) != 0)
    {
        uint8_t character;

        character = LPUART1->RDR;

        bc_fifo_irq_write(_bc_uart.channel[1].rx_fifo, &character, 1);
    }
}
