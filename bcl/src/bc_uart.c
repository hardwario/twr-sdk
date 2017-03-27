#include <bc_uart.h>
#include <bc_irq.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

typedef struct
{
    struct
    {
        bc_fifo_t *tx_fifo;
        bc_fifo_t *rx_fifo;

    } channel[3];

} bc_uart_t;

static bc_uart_t _bc_uart;

typedef struct
{
    USART_TypeDef *USARTx;
    uint32_t pupdr_mask;
    uint32_t moder_mask;
    uint32_t afr_mask[2];
    uint32_t ccipr_mask;
    uint32_t apb1enr_mask;
    uint32_t apb2enr_mask;
    uint32_t brr[2];
    uint32_t UARTx_IRQn;
} bc_uart_channel_setup_t;

static const bc_uart_channel_setup_t _bc_uart_init_lut[3] =
{
        [BC_UART_UART0] = { USART4, 0x04, 0xfffffffa, { 0x66, 0 }, 0, RCC_APB1ENR_USART4EN, 0, { 0xd05, 0x116 }, USART4_5_IRQn },
        [BC_UART_UART1] = { LPUART1, 0x40, 0xffffffaf, { 0x6600, 0 }, 0x0c00, RCC_APB1ENR_LPUART1EN, 0, { 0x369, 0x00 }, LPUART1_IRQn },
        [BC_UART_UART2] = { USART1, 0x100000, 0xffebffff, { 0, 0x0440 }, 0, 0, RCC_APB2ENR_USART1EN, { 0xd05, 0x116 }, USART1_IRQn }
};

static void _bc_uart_irq_handler(bc_uart_channel_t channel);

void bc_uart_init(bc_uart_channel_t channel, bc_uart_param_t *param, bc_fifo_t *tx_fifo, bc_fifo_t *rx_fifo)
{
    // If channel is UART1 (LPUART) and invalid baudrate desired ...
    if ((channel == BC_UART_UART1) && (param->baudrate != BC_UART_BAUDRATE_9600_BD))
    {
        // ... don t do anything
        return;
    }


    const bc_uart_channel_setup_t *channel_setup = &_bc_uart_init_lut[channel];

    _bc_uart.channel[channel].tx_fifo = tx_fifo;
    _bc_uart.channel[channel].rx_fifo = rx_fifo;

    // Enable GPIOA clock
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Enable pull-up on RXD pin
    GPIOA->PUPDR |= channel_setup->pupdr_mask;

    // Configure TXD and RXD pins as alternate function
    GPIOA->MODER &= channel_setup->moder_mask;

    // Select alternate function for TXD and RXD pins
    GPIOA->AFR[0] |= channel_setup->afr_mask[0];
    GPIOA->AFR[1] |= channel_setup->afr_mask[1];

    // Set clock source
    RCC->CCIPR |= channel_setup->ccipr_mask;

    // Enable clock for UART
    RCC->APB1ENR |= channel_setup->apb1enr_mask;
    RCC->APB2ENR |= channel_setup->apb2enr_mask;

    // Enable transmitter and receiver
    channel_setup->USARTx->CR1 = USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE;

    // Disable overrun detection
    channel_setup->USARTx->CR3 = USART_CR3_OVRDIS;

    // Configure baudrate
    channel_setup->USARTx->BRR = channel_setup->brr[param->baudrate];

    // Enable UART
    channel_setup->USARTx->CR1 |= USART_CR1_UE;

    // Enable UART interrupts
    NVIC_EnableIRQ(channel_setup->UARTx_IRQn);
}

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length, bc_tick_t timeout)
{
    // Enable PLL and disable sleep
    bc_module_core_pll_enable();

    size_t bytes_written = 0;

    USART_TypeDef *USARTx = _bc_uart_init_lut[channel].USARTx;

    if (timeout == 0)
    {
        // Write buffer to FIFO
        bytes_written = bc_fifo_write(_bc_uart.channel[channel].tx_fifo, buffer, length);

        // Disable interrupts
        bc_irq_disable();

        // Enable transmit interrupts of ...
        USARTx->CR1 |= USART_CR1_TXEIE;

        // Enable interrupts
        bc_irq_enable();
    }
    else
    {
        // TODO Implement timeout

        // Write buffer to FIFO
        bytes_written = bc_fifo_write(_bc_uart.channel[channel].tx_fifo, buffer, length);

        // Disable interrupts
        bc_irq_disable();

        // Enable transmit interrupts
        USARTx->CR1 |= USART_CR1_TXEIE;

        // Enable interrupts
        bc_irq_enable();

        while ((USARTx->CR1 & USART_CR1_TXEIE) != 0)
        {
            continue;
        }

        while ((USARTx->ISR & USART_ISR_TC) == 0)
        {
            continue;
        }

    }

    return bytes_written;
}

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout)
{
    // TODO ... implement read on other channels

    if (timeout == 0)
    {
        return bc_fifo_read(_bc_uart.channel[channel].rx_fifo, buffer, length);

        return 0;
    }

    bc_tick_t tick_timeout = bc_tick_get() + timeout;
    
    size_t bytes_read = 0;

    while (length != 0)
    {
        size_t n_bytes = bc_fifo_read(_bc_uart.channel[channel].rx_fifo, (uint8_t *) buffer + bytes_read, length);

        bytes_read += n_bytes;

        length -= n_bytes;

        if (bc_tick_get() >= tick_timeout)
        {
            break;
        }

        // TODO Sleep here
    }

    return bytes_read;
}

static void _bc_uart_irq_handler(bc_uart_channel_t channel)
{
    USART_TypeDef *USARTx = _bc_uart_init_lut[channel].USARTx;

    // If it is TX interrupt...
    if ((USARTx->ISR & USART_ISR_TXE) != 0)
    {
        uint8_t character;

        // If there are still data in FIFO...
        if (bc_fifo_irq_read(_bc_uart.channel[channel].tx_fifo, &character, 1) != 0)
        {
            // Load TX buffer
            USARTx->TDR = character;
        }
        else
        {
            // Disable TX interrupts
            USARTx->CR1 &= ~USART_CR1_TXEIE;

            // Disable PLL and enable sleep
            bc_module_core_pll_disable();
        }
    }

    // If it is RX interrupt...
    if ((USARTx->ISR & USART_ISR_RXNE) != 0)
    {
        uint8_t character;

        character = USARTx->RDR;

        bc_fifo_irq_write(_bc_uart.channel[channel].rx_fifo, &character, 1);
    }
}

void AES_RNG_LPUART1_IRQHandler(void)
{
    _bc_uart_irq_handler(BC_UART_UART1);
}

void USART1_IRQHandler(void)
{
    _bc_uart_irq_handler(BC_UART_UART2);
}

void USART4_5_IRQHandler(void)
{
    _bc_uart_irq_handler(BC_UART_UART0);
}
