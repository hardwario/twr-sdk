#include <hio_uart.h>
#include <hio_scheduler.h>
#include <hio_irq.h>
#include <hio_system.h>
#include <stm32l0xx.h>
#include <hio_dma.h>

typedef struct
{
    bool initialized;
    void (*event_handler)(hio_uart_channel_t, hio_uart_event_t, void *);
    void *event_param;
    hio_fifo_t *write_fifo;
    hio_fifo_t *read_fifo;
    hio_scheduler_task_id_t async_write_task_id;
    hio_scheduler_task_id_t async_read_task_id;
    bool async_write_in_progress;
    bool async_read_in_progress;
    hio_tick_t async_timeout;
    USART_TypeDef *usart;

} hio_uart_t;

static hio_uart_t _hio_uart[3] =
{
    [HIO_UART_UART0] = { .initialized = false },
    [HIO_UART_UART1] = { .initialized = false },
    [HIO_UART_UART2] = { .initialized = false }
};

static struct
{
    hio_scheduler_task_id_t read_task_id;
    size_t length;

} _hio_uart_2_dma;

static uint32_t _hio_uart_brr_t[] =
{
    [HIO_UART_BAUDRATE_9600] = 0xd05,
    [HIO_UART_BAUDRATE_19200] = 0x682,
    [HIO_UART_BAUDRATE_38400] = 0x341,
    [HIO_UART_BAUDRATE_57600] = 0x22b,
    [HIO_UART_BAUDRATE_115200] = 0x116,
    [HIO_UART_BAUDRATE_921600] = 0x22
};

static void _hio_uart_async_write_task(void *param);
static void _hio_uart_async_read_task(void *param);
static void _hio_uart_2_dma_read_task(void *param);
static void _hio_uart_irq_handler(hio_uart_channel_t channel);

void hio_uart_init(hio_uart_channel_t channel, hio_uart_baudrate_t baudrate, hio_uart_setting_t setting)
{
    memset(&_hio_uart[channel], 0, sizeof(_hio_uart[channel]));

    switch(channel)
    {
        case HIO_UART_UART0:
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
            USART4->BRR = _hio_uart_brr_t[baudrate];

            NVIC_EnableIRQ(USART4_5_IRQn);

            _hio_uart[channel].usart = USART4;

            break;
        }
        case HIO_UART_UART1:
        {
            // Enable GPIOA clock
            RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

            // Errata workaround
            RCC->IOPENR;

            // Enable pull-up on RXD1 pin
            GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;

            if (baudrate <= HIO_UART_BAUDRATE_9600)
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

                _hio_uart[channel].usart = LPUART1;
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
                USART2->BRR = _hio_uart_brr_t[baudrate];

                NVIC_EnableIRQ(USART2_IRQn);

                _hio_uart[channel].usart = USART2;
            }

            break;
        }
        case HIO_UART_UART2:
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
            USART1->BRR = _hio_uart_brr_t[baudrate];

            NVIC_EnableIRQ(USART1_IRQn);

            _hio_uart[channel].usart = USART1;

            break;
        }
        default:
        {
            return;
        }
    }

    // Stop bits
    _hio_uart[channel].usart->CR2 &= ~USART_CR2_STOP_Msk;
    _hio_uart[channel].usart->CR2 |= ((uint32_t) setting & 0x03) << USART_CR2_STOP_Pos;

    // Parity
    _hio_uart[channel].usart->CR1 &= ~(USART_CR1_PCE_Msk | USART_CR1_PS_Msk);
    _hio_uart[channel].usart->CR1 |= (((uint32_t) setting >> 2) & 0x03) << USART_CR1_PS_Pos;

    // Word length
    _hio_uart[channel].usart->CR1 &= ~(USART_CR1_M1_Msk | USART_CR1_M0_Msk);

    uint32_t word_length = setting >> 4;

    if ((setting & 0x0c) != 0)
    {
        word_length++;
    }

    if (word_length == 0x07)
    {
        word_length = 0x10;

        _hio_uart[channel].usart->CR1 |= 1 << USART_CR1_M1_Pos;
    }
    else if (word_length == 0x09)
    {
        _hio_uart[channel].usart->CR1 |= 1 << USART_CR1_M0_Pos;
    }

    // Enable UART
    _hio_uart[channel].usart->CR1 |= USART_CR1_UE;

    _hio_uart[channel].initialized = true;
}


void hio_uart_deinit(hio_uart_channel_t channel)
{
    hio_uart_async_read_cancel(channel);

    // Disable UART
    _hio_uart[channel].usart->CR1 &= ~USART_CR1_UE_Msk;

    switch(channel)
    {
        case HIO_UART_UART0:
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
        case HIO_UART_UART1:
        {
            if (_hio_uart[channel].usart == LPUART1)
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
        case HIO_UART_UART2:
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

    _hio_uart[channel].initialized = false;
}

size_t hio_uart_write(hio_uart_channel_t channel, const void *buffer, size_t length)
{
    if (!_hio_uart[channel].initialized || _hio_uart[channel].async_write_in_progress)
    {
        return 0;
    }

    USART_TypeDef *usart = _hio_uart[channel].usart;

    size_t bytes_written = 0;

    if (_hio_uart[channel].usart != LPUART1)
    {
        hio_system_pll_enable();
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

    if (_hio_uart[channel].usart != LPUART1)
    {
        hio_system_pll_disable();
    }

    return bytes_written;
}

size_t hio_uart_read(hio_uart_channel_t channel, void *buffer, size_t length, hio_tick_t timeout)
{
    if (!_hio_uart[channel].initialized)
    {
        return 0;
    }

    USART_TypeDef *usart = _hio_uart[channel].usart;

    size_t bytes_read = 0;

    if (_hio_uart[channel].usart != LPUART1)
    {
        hio_system_pll_enable();
    }

    hio_tick_t tick_timeout = timeout == HIO_TICK_INFINITY ? HIO_TICK_INFINITY : hio_tick_get() + timeout;

    while (bytes_read != length)
    {
        // If timeout condition is met...
        if (hio_tick_get() >= tick_timeout)
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

    if (_hio_uart[channel].usart != LPUART1)
    {
        hio_system_pll_disable();
    }

    return bytes_read;
}

void hio_uart_set_event_handler(hio_uart_channel_t channel, void (*event_handler)(hio_uart_channel_t, hio_uart_event_t, void *), void *event_param)
{
    _hio_uart[channel].event_handler = event_handler;
    _hio_uart[channel].event_param = event_param;
}

void hio_uart_set_async_fifo(hio_uart_channel_t channel, hio_fifo_t *write_fifo, hio_fifo_t *read_fifo)
{
    _hio_uart[channel].write_fifo = write_fifo;
    _hio_uart[channel].read_fifo = read_fifo;
}

size_t hio_uart_async_write(hio_uart_channel_t channel, const void *buffer, size_t length)
{
    if (!_hio_uart[channel].initialized || _hio_uart[channel].write_fifo == NULL)
    {
        return 0;
    }

    size_t bytes_written = 0;

    for (size_t i = 0; i < length; i += 16)
    {
        bytes_written += hio_fifo_write(_hio_uart[channel].write_fifo, (uint8_t *)buffer + i, length - i  > 16 ? 16 : length - i);
    }

    if (bytes_written != 0)
    {
        if (!_hio_uart[channel].async_write_in_progress)
        {
            _hio_uart[channel].async_write_task_id = hio_scheduler_register(_hio_uart_async_write_task, (void *) channel, HIO_TICK_INFINITY);

            if (_hio_uart[channel].usart == LPUART1)
            {
                hio_system_deep_sleep_disable();
            }
            else
            {
                hio_system_pll_enable();
            }
        }
        else
        {
            hio_scheduler_plan_absolute(_hio_uart[channel].async_write_task_id, HIO_TICK_INFINITY);
        }

        hio_irq_disable();

        // Enable transmit interrupt
        _hio_uart[channel].usart->CR1 |= USART_CR1_TXEIE;

        hio_irq_enable();

        _hio_uart[channel].async_write_in_progress = true;
    }

    return bytes_written;
}

bool hio_uart_async_read_start(hio_uart_channel_t channel, hio_tick_t timeout)
{
    if (!_hio_uart[channel].initialized || _hio_uart[channel].read_fifo == NULL || _hio_uart[channel].async_read_in_progress)
    {
        return false;
    }

    _hio_uart[channel].async_timeout = timeout;

    _hio_uart[channel].async_read_task_id = hio_scheduler_register(_hio_uart_async_read_task, (void *) channel, _hio_uart[channel].async_timeout);

    if (channel == HIO_UART_UART2)
    {
        hio_dma_channel_config_t config = {
                .request = HIO_DMA_REQUEST_3,
                .direction = HIO_DMA_DIRECTION_TO_RAM,
                .data_size_memory = HIO_DMA_SIZE_1,
                .data_size_peripheral = HIO_DMA_SIZE_1,
                .length = _hio_uart[channel].read_fifo->size,
                .mode = HIO_DMA_MODE_CIRCULAR,
                .address_memory = _hio_uart[channel].read_fifo->buffer,
                .address_peripheral = (void *) &_hio_uart[channel].usart->RDR,
                .priority = HIO_DMA_PRIORITY_HIGH
        };

        hio_dma_init();

        hio_dma_channel_config(HIO_DMA_CHANNEL_3, &config);

        _hio_uart_2_dma.read_task_id = hio_scheduler_register(_hio_uart_2_dma_read_task, (void *) channel, 0);

        hio_irq_disable();
        // Enable receive DMA interrupt
        _hio_uart[channel].usart->CR3 |=  USART_CR3_DMAR;
        hio_irq_enable();

        hio_dma_channel_run(HIO_DMA_CHANNEL_3);
    }
    else
    {
        hio_irq_disable();
        // Enable receive interrupt
        _hio_uart[channel].usart->CR1 |= USART_CR1_RXNEIE;
        hio_irq_enable();
    }

    if (_hio_uart[channel].usart != LPUART1)
    {
        hio_system_pll_enable();
    }

    _hio_uart[channel].async_read_in_progress = true;

    return true;
}

bool hio_uart_async_read_cancel(hio_uart_channel_t channel)
{
    if (!_hio_uart[channel].initialized || !_hio_uart[channel].async_read_in_progress)
    {
        return false;
    }

    _hio_uart[channel].async_read_in_progress = false;

    if (channel == HIO_UART_UART2)
    {
        hio_dma_channel_stop(HIO_DMA_CHANNEL_3);

        hio_scheduler_unregister(_hio_uart_2_dma.read_task_id);

        hio_irq_disable();
        // Disable receive DMA interrupt
        _hio_uart[channel].usart->CR3 &= ~USART_CR3_DMAR_Msk;
        hio_irq_enable();
    }
    else
    {
        hio_irq_disable();

        // Disable receive interrupt
        _hio_uart[channel].usart->CR1 &= ~USART_CR1_RXNEIE_Msk;

        hio_irq_enable();
    }

    if (_hio_uart[channel].usart != LPUART1)
    {
        hio_system_pll_disable();
    }

    hio_scheduler_unregister(_hio_uart[channel].async_read_task_id);

    return false;
}

size_t hio_uart_async_read(hio_uart_channel_t channel, void *buffer, size_t length)
{
    if (!_hio_uart[channel].initialized || !_hio_uart[channel].async_read_in_progress)
    {
        return 0;
    }

    size_t bytes_read = hio_fifo_read(_hio_uart[channel].read_fifo, buffer, length);

    return bytes_read;
}

static void _hio_uart_async_write_task(void *param)
{
    hio_uart_channel_t channel = (hio_uart_channel_t) param;
    hio_uart_t *uart = &_hio_uart[channel];

    uart->async_write_in_progress = false;

    hio_scheduler_unregister(uart->async_write_task_id);

    if (uart->usart == LPUART1)
    {
        hio_system_deep_sleep_enable();
    }
    else
    {
        hio_system_pll_disable();
    }

    if (uart->event_handler != NULL)
    {
        uart->event_handler(channel, HIO_UART_EVENT_ASYNC_WRITE_DONE, uart->event_param);
    }
}

static void _hio_uart_async_read_task(void *param)
{
    hio_uart_channel_t channel = (hio_uart_channel_t) param;
    hio_uart_t *uart = &_hio_uart[channel];

    hio_scheduler_plan_current_relative(uart->async_timeout);

    if (uart->event_handler != NULL)
    {
        if (hio_fifo_is_empty(uart->read_fifo))
        {
            uart->event_handler(channel, HIO_UART_EVENT_ASYNC_READ_TIMEOUT, uart->event_param);
        }
        else
        {
            uart->event_handler(channel, HIO_UART_EVENT_ASYNC_READ_DATA, uart->event_param);
        }
    }
}

static void _hio_uart_2_dma_read_task(void *param)
{
    (void) param;

    size_t length = hio_dma_channel_get_length(HIO_DMA_CHANNEL_3);

    hio_uart_t *uart = &_hio_uart[HIO_UART_UART2];

    if (_hio_uart_2_dma.length != length)
    {
        uart->read_fifo->head = uart->read_fifo->size - length;

        _hio_uart_2_dma.length = length;

        hio_scheduler_plan_now(uart->async_read_task_id);
    }

    hio_scheduler_plan_current_now();
}

static void _hio_uart_irq_handler(hio_uart_channel_t channel)
{
    USART_TypeDef *usart = _hio_uart[channel].usart;

    if ((usart->CR1 & USART_CR1_RXNEIE) != 0 && (usart->ISR & USART_ISR_RXNE) != 0)
    {
        uint8_t character;

        // Read receive data register
        character = usart->RDR;

        hio_fifo_irq_write(_hio_uart[channel].read_fifo, &character, 1);

        hio_scheduler_plan_now(_hio_uart[channel].async_read_task_id);
    }

    // If it is transmit interrupt...
    if ((usart->CR1 & USART_CR1_TXEIE) != 0 && (usart->ISR & USART_ISR_TXE) != 0)
    {
        uint8_t character;

        // If there are still data in FIFO...
        if (hio_fifo_irq_read(_hio_uart[channel].write_fifo, &character, 1) != 0)
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

        hio_scheduler_plan_now(_hio_uart[channel].async_write_task_id);
    }
}

void AES_RNG_LPUART1_IRQHandler(void)
{
    _hio_uart_irq_handler(HIO_UART_UART1);
}

void USART1_IRQHandler(void)
{
    _hio_uart_irq_handler(HIO_UART_UART2);
}

void USART2_IRQHandler(void)
{
    _hio_uart_irq_handler(HIO_UART_UART1);
}

void USART4_5_IRQHandler(void)
{
    _hio_uart_irq_handler(HIO_UART_UART0);
}
