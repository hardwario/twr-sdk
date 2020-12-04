#include <bc_uart.h>
#include <bc_scheduler.h>
#include <bc_irq.h>
#include <bc_system.h>
#include <stm32l0xx.h>
#include <bc_dma.h>

typedef struct
{
    bool initialized;
    void (*event_handler)(bc_uart_channel_t, bc_uart_event_t, void *);
    void *event_param;
    bc_fifo_t *write_fifo;
    bc_fifo_t *read_fifo;
    bc_scheduler_task_id_t async_write_task_id;
    bc_scheduler_task_id_t async_read_task_id;
    bool async_write_in_progress;
    bool async_read_in_progress;
    bc_tick_t async_timeout;
    USART_TypeDef *usart;

} bc_uart_t;

static bc_uart_t _bc_uart[3] =
{
    [BC_UART_UART0] = { .initialized = false },
    [BC_UART_UART1] = { .initialized = false },
    [BC_UART_UART2] = { .initialized = false }
};

static struct
{
    bc_scheduler_task_id_t read_task_id;
    size_t length;

} _bc_uart_2_dma;

static uint32_t _bc_uart_brr_t[] =
{
    [BC_UART_BAUDRATE_9600] = 0xd05,
    [BC_UART_BAUDRATE_19200] = 0x682,
    [BC_UART_BAUDRATE_38400] = 0x341,
    [BC_UART_BAUDRATE_57600] = 0x22b,
    [BC_UART_BAUDRATE_115200] = 0x116,
    [BC_UART_BAUDRATE_921600] = 0x22
};

static void _bc_uart_async_write_task(void *param);
static void _bc_uart_async_read_task(void *param);
static void _bc_uart_2_dma_read_task(void *param);
static void _bc_uart_irq_handler(bc_uart_channel_t channel);

void bc_uart_init(bc_uart_channel_t channel, bc_uart_baudrate_t baudrate, bc_uart_setting_t setting)
{
    memset(&_bc_uart[channel], 0, sizeof(_bc_uart[channel]));

    switch(channel)
    {
        case BC_UART_UART0:
        {
            // Enable GPIOA clock
            RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

            // Errata workaround
            RCC->IOPENR;

            // Enable pull-up on RXD0 pin
            GPIOA->PUPDR |= GPIO_PUPDR_PUPD1_0;

            // Select AF6 alternate function for TXD0 and RXD0 pins
            GPIOA->AFR[0] |= 6 << GPIO_AFRL_AFRL1_Pos | 6 << GPIO_AFRL_AFRL0_Pos;

            // Configure TXD0 and RXD0 pins as alternate function
            GPIOA->MODER &= ~(1 << GPIO_MODER_MODE1_Pos | 1 << GPIO_MODER_MODE0_Pos);

            // Enable clock for USART4
            RCC->APB1ENR |= RCC_APB1ENR_USART4EN;

            // Errata workaround
            RCC->APB1ENR;

            // Enable transmitter and receiver, peripheral enabled in stop mode
            USART4->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

            // Clock enabled in stop mode, disable overrun detection, one bit sampling method
            USART4->CR3 = USART_CR3_UCESM | USART_CR3_OVRDIS | USART_CR3_ONEBIT;

            // Configure baudrate
            USART4->BRR = _bc_uart_brr_t[baudrate];

            NVIC_EnableIRQ(USART4_5_IRQn);

            _bc_uart[channel].usart = USART4;

            break;
        }
        case BC_UART_UART1:
        {
            // Enable GPIOA clock
            RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

            // Errata workaround
            RCC->IOPENR;

            // Enable pull-up on RXD1 pin
            GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;

            if (baudrate <= BC_UART_BAUDRATE_9600)
            {
                // Select AF6 alternate function for TXD1 and RXD1 pins
                GPIOA->AFR[0] |= 6 << GPIO_AFRL_AFRL3_Pos | 6 << GPIO_AFRL_AFRL2_Pos;

                // Configure TXD1 and RXD1 pins as alternate function
                GPIOA->MODER &= ~(1 << GPIO_MODER_MODE3_Pos | 1 << GPIO_MODER_MODE2_Pos);

                // Set LSE as LPUART1 clock source
                RCC->CCIPR |= RCC_CCIPR_LPUART1SEL_1 | RCC_CCIPR_LPUART1SEL_0;

                // Enable clock for LPUART1
                RCC->APB1ENR |= RCC_APB1ENR_LPUART1EN;

                // Errata workaround
                RCC->APB1ENR;

                // Enable transmitter and receiver, peripheral enabled in stop mode
                LPUART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

                // Clock enabled in stop mode, disable overrun detection, one bit sampling method
                LPUART1->CR3 = USART_CR3_UCESM | USART_CR3_OVRDIS | USART_CR3_ONEBIT;

                // Configure baudrate
                LPUART1->BRR = 0x369;

                NVIC_EnableIRQ(LPUART1_IRQn);

                _bc_uart[channel].usart = LPUART1;
            }
            else
            {
                // Select AF4 alternate function for TXD1 and RXD1 pins
                GPIOA->AFR[0] |= 4 << GPIO_AFRL_AFRL3_Pos | 4 << GPIO_AFRL_AFRL2_Pos;

                // Configure TXD1 and RXD1 pins as alternate function
                GPIOA->MODER &= ~(1 << GPIO_MODER_MODE3_Pos | 1 << GPIO_MODER_MODE2_Pos);

                // Enable clock for USART2
                RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

                // Errata workaround
                RCC->APB1ENR;

                // Enable transmitter and receiver, peripheral enabled in stop mode
                USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

                // Clock enabled in stop mode, disable overrun detection, one bit sampling method
                USART2->CR3 = USART_CR3_UCESM | USART_CR3_OVRDIS | USART_CR3_ONEBIT;

                // Configure baudrate
                USART2->BRR = _bc_uart_brr_t[baudrate];

                NVIC_EnableIRQ(USART2_IRQn);

                _bc_uart[channel].usart = USART2;
            }

            break;
        }
        case BC_UART_UART2:
        {
            // Enable GPIOA clock
            RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

            // Errata workaround
            RCC->IOPENR;

            // Enable pull-up on RXD2 pin
            GPIOA->PUPDR |= GPIO_PUPDR_PUPD10_0;

            // Select AF4 alternate function for TXD2 and RXD2 pins
            GPIOA->AFR[1] |= 4 << GPIO_AFRH_AFRH2_Pos | 4 << GPIO_AFRH_AFRH1_Pos;

            // Configure TXD2 and RXD2 pins as alternate function
            GPIOA->MODER &= ~(1 << GPIO_MODER_MODE10_Pos | 1 << GPIO_MODER_MODE9_Pos);

            // Enable clock for USART1
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

            // Errata workaround
            RCC->APB2ENR;

            // Enable transmitter and receiver, peripheral enabled in stop mode
            USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

            // Clock enabled in stop mode, disable overrun detection, one bit sampling method
            USART1->CR3 = USART_CR3_UCESM | USART_CR3_OVRDIS | USART_CR3_ONEBIT;

            // Configure baudrate
            USART1->BRR = _bc_uart_brr_t[baudrate];

            NVIC_EnableIRQ(USART1_IRQn);

            _bc_uart[channel].usart = USART1;

            break;
        }
        default:
        {
            return;
        }
    }

    // Stop bits
    _bc_uart[channel].usart->CR2 &= ~USART_CR2_STOP_Msk;
    _bc_uart[channel].usart->CR2 |= ((uint32_t) setting & 0x03) << USART_CR2_STOP_Pos;

    // Parity
    _bc_uart[channel].usart->CR1 &= ~(USART_CR1_PCE_Msk | USART_CR1_PS_Msk);
    _bc_uart[channel].usart->CR1 |= (((uint32_t) setting >> 2) & 0x03) << USART_CR1_PS_Pos;

    // Word length
    _bc_uart[channel].usart->CR1 &= ~(USART_CR1_M1_Msk | USART_CR1_M0_Msk);

    uint32_t word_length = setting >> 4;

    if ((setting & 0x0c) != 0)
    {
        word_length++;
    }

    if (word_length == 0x07)
    {
        word_length = 0x10;

        _bc_uart[channel].usart->CR1 |= 1 << USART_CR1_M1_Pos;
    }
    else if (word_length == 0x09)
    {
        _bc_uart[channel].usart->CR1 |= 1 << USART_CR1_M0_Pos;
    }

    // Enable UART
    _bc_uart[channel].usart->CR1 |= USART_CR1_UE;

    _bc_uart[channel].initialized = true;
}


void bc_uart_deinit(bc_uart_channel_t channel)
{
    bc_uart_async_read_cancel(channel);

    // Disable UART
    _bc_uart[channel].usart->CR1 &= ~USART_CR1_UE_Msk;

    switch(channel)
    {
        case BC_UART_UART0:
        {
            NVIC_DisableIRQ(USART4_5_IRQn);

            // Disable clock for USART4
            RCC->APB1ENR &= ~RCC_APB1ENR_USART4EN;

            // Configure TXD0 and RXD0 pins as Analog
            GPIOA->MODER |= (GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE0_Msk);

            // Clear alternate function for TXD0 and RXD0 pins
            GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL1_Msk | GPIO_AFRL_AFRL0_Msk);

            // Disable pull-up on RXD0 pin
            GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD1_Msk;

            break;
        }
        case BC_UART_UART1:
        {
            if (_bc_uart[channel].usart == LPUART1)
            {
                NVIC_DisableIRQ(LPUART1_IRQn);

                // Disable clock for LPUART1
                RCC->APB1ENR &= ~RCC_APB1ENR_LPUART1EN;
            }
            else
            {
                NVIC_DisableIRQ(USART2_IRQn);

                // Disable clock for USART2
                RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
            }

            // Configure TXD1 and RXD1 pins as Analog
            GPIOA->MODER |= (GPIO_MODER_MODE3_Msk | GPIO_MODER_MODE2_Msk);

            // Clear alternate function for TXD1 and RXD1 pins
            GPIOA->AFR[0] &= ~(GPIO_AFRL_AFRL3_Msk | GPIO_AFRL_AFRL2_Msk);

            // Disable pull-up on RXD1 pin
            GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3_Msk;

            break;
        }
        case BC_UART_UART2:
        {
            NVIC_DisableIRQ(USART1_IRQn);

            // Disable clock for USART1
            RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN_Msk;

            // Configure TXD2 and RXD2 pins as Analog
            GPIOA->MODER |= (GPIO_MODER_MODE10_Msk | GPIO_MODER_MODE9_Msk);

            // Clear alternate function for TXD2 and RXD2 pins
            GPIOA->AFR[1] &= ~(GPIO_AFRH_AFRH2_Msk | GPIO_AFRH_AFRH1_Msk);

            // Disable pull-up on RXD2 pin
            GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD10_Msk;

            break;
        }
        default:
        {
            return;
        }
    }

    _bc_uart[channel].initialized = false;
}

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length)
{
    if (!_bc_uart[channel].initialized || _bc_uart[channel].async_write_in_progress)
    {
        return 0;
    }

    USART_TypeDef *usart = _bc_uart[channel].usart;

    size_t bytes_written = 0;

    if (_bc_uart[channel].usart != LPUART1)
    {
        bc_system_pll_enable();
    }

    while (bytes_written != length)
    {
        // Until transmit data register is not empty...
        while ((usart->ISR & USART_ISR_TXE) == 0)
        {
            continue;
        }

        // Load transmit data register
        usart->TDR = *((uint8_t *) buffer + bytes_written++);
    }

    // Until transmission is not complete...
    while ((usart->ISR & USART_ISR_TC) == 0)
    {
        continue;
    }

    if (_bc_uart[channel].usart != LPUART1)
    {
        bc_system_pll_disable();
    }

    return bytes_written;
}

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout)
{
    if (!_bc_uart[channel].initialized)
    {
        return 0;
    }

    USART_TypeDef *usart = _bc_uart[channel].usart;

    size_t bytes_read = 0;

    if (_bc_uart[channel].usart != LPUART1)
    {
        bc_system_pll_enable();
    }

    bc_tick_t tick_timeout = timeout == BC_TICK_INFINITY ? BC_TICK_INFINITY : bc_tick_get() + timeout;

    while (bytes_read != length)
    {
        // If timeout condition is met...
        if (bc_tick_get() >= tick_timeout)
        {
            break;
        }

        // If receive data register is empty...
        if ((usart->ISR & USART_ISR_RXNE) == 0)
        {
            continue;
        }

        // Read receive data register
        *((uint8_t *) buffer + bytes_read++) = usart->RDR;
    }

    if (_bc_uart[channel].usart != LPUART1)
    {
        bc_system_pll_disable();
    }

    return bytes_read;
}

void bc_uart_set_event_handler(bc_uart_channel_t channel, void (*event_handler)(bc_uart_channel_t, bc_uart_event_t, void *), void *event_param)
{
    _bc_uart[channel].event_handler = event_handler;
    _bc_uart[channel].event_param = event_param;
}

void bc_uart_set_async_fifo(bc_uart_channel_t channel, bc_fifo_t *write_fifo, bc_fifo_t *read_fifo)
{
    _bc_uart[channel].write_fifo = write_fifo;
    _bc_uart[channel].read_fifo = read_fifo;
}

size_t bc_uart_async_write(bc_uart_channel_t channel, const void *buffer, size_t length)
{
    if (!_bc_uart[channel].initialized || _bc_uart[channel].write_fifo == NULL)
    {
        return 0;
    }

    size_t bytes_written = 0;

    for (size_t i = 0; i < length; i += 16)
    {
        bytes_written += bc_fifo_write(_bc_uart[channel].write_fifo, (uint8_t *)buffer + i, length - i  > 16 ? 16 : length - i);
    }

    if (bytes_written != 0)
    {
        if (!_bc_uart[channel].async_write_in_progress)
        {
            _bc_uart[channel].async_write_task_id = bc_scheduler_register(_bc_uart_async_write_task, (void *) channel, BC_TICK_INFINITY);

            if (_bc_uart[channel].usart == LPUART1)
            {
                bc_system_deep_sleep_disable();
            }
            else
            {
                bc_system_pll_enable();
            }
        }
        else
        {
            bc_scheduler_plan_absolute(_bc_uart[channel].async_write_task_id, BC_TICK_INFINITY);
        }

        bc_irq_disable();

        // Enable transmit interrupt
        _bc_uart[channel].usart->CR1 |= USART_CR1_TXEIE;

        bc_irq_enable();

        _bc_uart[channel].async_write_in_progress = true;
    }

    return bytes_written;
}

bool bc_uart_async_read_start(bc_uart_channel_t channel, bc_tick_t timeout)
{
    if (!_bc_uart[channel].initialized || _bc_uart[channel].read_fifo == NULL || _bc_uart[channel].async_read_in_progress)
    {
        return false;
    }

    _bc_uart[channel].async_timeout = timeout;

    _bc_uart[channel].async_read_task_id = bc_scheduler_register(_bc_uart_async_read_task, (void *) channel, _bc_uart[channel].async_timeout);

    if (channel == BC_UART_UART2)
    {
        bc_dma_channel_config_t config = {
                .request = BC_DMA_REQUEST_3,
                .direction = BC_DMA_DIRECTION_TO_RAM,
                .data_size_memory = BC_DMA_SIZE_1,
                .data_size_peripheral = BC_DMA_SIZE_1,
                .length = _bc_uart[channel].read_fifo->size,
                .mode = BC_DMA_MODE_CIRCULAR,
                .address_memory = _bc_uart[channel].read_fifo->buffer,
                .address_peripheral = (void *) &_bc_uart[channel].usart->RDR,
                .priority = BC_DMA_PRIORITY_HIGH
        };

        bc_dma_init();

        bc_dma_channel_config(BC_DMA_CHANNEL_3, &config);

        _bc_uart_2_dma.read_task_id = bc_scheduler_register(_bc_uart_2_dma_read_task, (void *) channel, 0);

        bc_irq_disable();
        // Enable receive DMA interrupt
        _bc_uart[channel].usart->CR3 |=  USART_CR3_DMAR;
        bc_irq_enable();

        bc_dma_channel_run(BC_DMA_CHANNEL_3);
    }
    else
    {
        bc_irq_disable();
        // Enable receive interrupt
        _bc_uart[channel].usart->CR1 |= USART_CR1_RXNEIE;
        bc_irq_enable();
    }

    if (_bc_uart[channel].usart != LPUART1)
    {
        bc_system_pll_enable();
    }

    _bc_uart[channel].async_read_in_progress = true;

    return true;
}

bool bc_uart_async_read_cancel(bc_uart_channel_t channel)
{
    if (!_bc_uart[channel].initialized || !_bc_uart[channel].async_read_in_progress)
    {
        return false;
    }

    _bc_uart[channel].async_read_in_progress = false;

    if (channel == BC_UART_UART2)
    {
        bc_dma_channel_stop(BC_DMA_CHANNEL_3);

        bc_scheduler_unregister(_bc_uart_2_dma.read_task_id);

        bc_irq_disable();
        // Disable receive DMA interrupt
        _bc_uart[channel].usart->CR3 &= ~USART_CR3_DMAR_Msk;
        bc_irq_enable();
    }
    else
    {
        bc_irq_disable();

        // Disable receive interrupt
        _bc_uart[channel].usart->CR1 &= ~USART_CR1_RXNEIE_Msk;

        bc_irq_enable();
    }

    if (_bc_uart[channel].usart != LPUART1)
    {
        bc_system_pll_disable();
    }

    bc_scheduler_unregister(_bc_uart[channel].async_read_task_id);

    return false;
}

size_t bc_uart_async_read(bc_uart_channel_t channel, void *buffer, size_t length)
{
    if (!_bc_uart[channel].initialized || !_bc_uart[channel].async_read_in_progress)
    {
        return 0;
    }

    size_t bytes_read = bc_fifo_read(_bc_uart[channel].read_fifo, buffer, length);

    return bytes_read;
}

static void _bc_uart_async_write_task(void *param)
{
    bc_uart_channel_t channel = (bc_uart_channel_t) param;
    bc_uart_t *uart = &_bc_uart[channel];

    uart->async_write_in_progress = false;

    bc_scheduler_unregister(uart->async_write_task_id);

    if (uart->usart == LPUART1)
    {
        bc_system_deep_sleep_enable();
    }
    else
    {
        bc_system_pll_disable();
    }

    if (uart->event_handler != NULL)
    {
        uart->event_handler(channel, BC_UART_EVENT_ASYNC_WRITE_DONE, uart->event_param);
    }
}

static void _bc_uart_async_read_task(void *param)
{
    bc_uart_channel_t channel = (bc_uart_channel_t) param;
    bc_uart_t *uart = &_bc_uart[channel];

    bc_scheduler_plan_current_relative(uart->async_timeout);

    if (uart->event_handler != NULL)
    {
        if (bc_fifo_is_empty(uart->read_fifo))
        {
            uart->event_handler(channel, BC_UART_EVENT_ASYNC_READ_TIMEOUT, uart->event_param);
        }
        else
        {
            uart->event_handler(channel, BC_UART_EVENT_ASYNC_READ_DATA, uart->event_param);
        }
    }
}

static void _bc_uart_2_dma_read_task(void *param)
{
    (void) param;

    size_t length = bc_dma_channel_get_length(BC_DMA_CHANNEL_3);

    bc_uart_t *uart = &_bc_uart[BC_UART_UART2];

    if (_bc_uart_2_dma.length != length)
    {
        uart->read_fifo->head = uart->read_fifo->size - length;

        _bc_uart_2_dma.length = length;

        bc_scheduler_plan_now(uart->async_read_task_id);
    }

    bc_scheduler_plan_current_now();
}

static void _bc_uart_irq_handler(bc_uart_channel_t channel)
{
    USART_TypeDef *usart = _bc_uart[channel].usart;

    if ((usart->CR1 & USART_CR1_RXNEIE) != 0 && (usart->ISR & USART_ISR_RXNE) != 0)
    {
        uint8_t character;

        // Read receive data register
        character = usart->RDR;

        bc_fifo_irq_write(_bc_uart[channel].read_fifo, &character, 1);

        bc_scheduler_plan_now(_bc_uart[channel].async_read_task_id);
    }

    // If it is transmit interrupt...
    if ((usart->CR1 & USART_CR1_TXEIE) != 0 && (usart->ISR & USART_ISR_TXE) != 0)
    {
        uint8_t character;

        // If there are still data in FIFO...
        if (bc_fifo_irq_read(_bc_uart[channel].write_fifo, &character, 1) != 0)
        {
            // Load transmit data register
            usart->TDR = character;
        }
        else
        {
            // Disable transmit interrupt
            usart->CR1 &= ~USART_CR1_TXEIE;

            // Enable transmission complete interrupt
            usart->CR1 |= USART_CR1_TCIE;
        }
    }

    // If it is transmit interrupt...
    if ((usart->CR1 & USART_CR1_TCIE) != 0 && (usart->ISR & USART_ISR_TC) != 0)
    {
        // Disable transmission complete interrupt
        usart->CR1 &= ~USART_CR1_TCIE;

        bc_scheduler_plan_now(_bc_uart[channel].async_write_task_id);
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

void USART2_IRQHandler(void)
{
    _bc_uart_irq_handler(BC_UART_UART1);
}

void USART4_5_IRQHandler(void)
{
    _bc_uart_irq_handler(BC_UART_UART0);
}
