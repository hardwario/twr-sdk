#include <twr_uart.h>
#include <twr_scheduler.h>
#include <twr_irq.h>
#include <twr_system.h>
#include <stm32l0xx.h>
#include <twr_dma.h>
#include <twr_gpio.h>

typedef struct
{
    bool initialized;
    void (*event_handler)(twr_uart_channel_t, twr_uart_event_t, void *);
    void *event_param;
    twr_fifo_t *write_fifo;
    twr_fifo_t *read_fifo;
    twr_scheduler_task_id_t async_write_task_id;
    twr_scheduler_task_id_t async_read_task_id;
    bool async_write_in_progress;
    bool async_read_in_progress;
    twr_tick_t async_timeout;
    USART_TypeDef *usart;

} twr_uart_t;

static twr_uart_t _twr_uart[3] =
{
    [TWR_UART_UART0] = { .initialized = false },
    [TWR_UART_UART1] = { .initialized = false },
    [TWR_UART_UART2] = { .initialized = false }
};

static struct
{
    twr_scheduler_task_id_t read_task_id;
    size_t length;

} _twr_uart_2_dma;

static uint32_t _twr_uart_brr_t[] =
{
    [TWR_UART_BAUDRATE_9600] = 0xd05,
    [TWR_UART_BAUDRATE_19200] = 0x682,
    [TWR_UART_BAUDRATE_38400] = 0x341,
    [TWR_UART_BAUDRATE_57600] = 0x22b,
    [TWR_UART_BAUDRATE_115200] = 0x116,
    [TWR_UART_BAUDRATE_921600] = 0x22
};

static void _twr_uart_async_write_task(void *param);
static void _twr_uart_async_read_task(void *param);
static void _twr_uart_2_dma_read_task(void *param);
static void _twr_uart_irq_handler(twr_uart_channel_t channel);

void twr_uart_init(twr_uart_channel_t channel, twr_uart_baudrate_t baudrate, twr_uart_setting_t setting)
{
    memset(&_twr_uart[channel], 0, sizeof(_twr_uart[channel]));

    switch(channel)
    {
        case TWR_UART_UART0:
        {
            twr_gpio_init(TWR_GPIO_P0); // TXD0
            twr_gpio_init(TWR_GPIO_P1); // RXD0

            // Enable pull-up on RXD0 pin
            twr_gpio_set_pull(TWR_GPIO_P1, TWR_GPIO_PULL_UP);

            // Select AF6 alternate function for TXD0 and RXD0 pins
            twr_gpio_set_mode(TWR_GPIO_P0, TWR_GPIO_MODE_ALTERNATE_6);
            twr_gpio_set_mode(TWR_GPIO_P1, TWR_GPIO_MODE_ALTERNATE_6);

            // Enable clock for USART4
            RCC->APB1ENR |= RCC_APB1ENR_USART4EN;

            // Errata workaround
            RCC->APB1ENR;

            // Enable transmitter and receiver, peripheral enabled in stop mode
            USART4->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

            // Clock enabled in stop mode, disable overrun detection, one bit sampling method
            USART4->CR3 = USART_CR3_UCESM | USART_CR3_OVRDIS | USART_CR3_ONEBIT;

            // Configure baudrate
            USART4->BRR = _twr_uart_brr_t[baudrate];

            NVIC_EnableIRQ(USART4_5_IRQn);

            _twr_uart[channel].usart = USART4;

            break;
        }
        case TWR_UART_UART1:
        {

            twr_gpio_init(TWR_GPIO_P2); // TXD1
            twr_gpio_init(TWR_GPIO_P3); // RXD1

            // Enable pull-up on RXD1 pin
            twr_gpio_set_pull(TWR_GPIO_P3, TWR_GPIO_PULL_UP);

            if (baudrate <= TWR_UART_BAUDRATE_9600)
            {
                // Select AF6 alternate function for TXD1 and RXD1 pins
                twr_gpio_set_mode(TWR_GPIO_P2, TWR_GPIO_MODE_ALTERNATE_6);
                twr_gpio_set_mode(TWR_GPIO_P3, TWR_GPIO_MODE_ALTERNATE_6);

                // Set HSI16 as LPUART1 clock source
                RCC->CCIPR |= RCC_CCIPR_LPUART1SEL_1;
                RCC->CCIPR &= ~RCC_CCIPR_LPUART1SEL_0;

                // Enable clock for LPUART1
                RCC->APB1ENR |= RCC_APB1ENR_LPUART1EN;

                // Errata workaround
                RCC->APB1ENR;

                // Enable transmitter and receiver, peripheral enabled in stop mode
                LPUART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

                // Clock disabled in stop mode, disable overrun detection, one bit sampling method
                LPUART1->CR3 = USART_CR3_OVRDIS | USART_CR3_ONEBIT;

                // Configure baudrate (256 * 16E6 / 9600)
                LPUART1->BRR = 426667;

                NVIC_EnableIRQ(LPUART1_IRQn);

                _twr_uart[channel].usart = LPUART1;
            }
            else
            {
                // Select AF4 alternate function for TXD1 and RXD1 pins
                twr_gpio_set_mode(TWR_GPIO_P2, TWR_GPIO_MODE_ALTERNATE_4);
                twr_gpio_set_mode(TWR_GPIO_P3, TWR_GPIO_MODE_ALTERNATE_4);

                // Enable clock for USART2
                RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

                // Errata workaround
                RCC->APB1ENR;

                // Enable transmitter and receiver, peripheral enabled in stop mode
                USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

                // Clock enabled in stop mode, disable overrun detection, one bit sampling method
                USART2->CR3 = USART_CR3_UCESM | USART_CR3_OVRDIS | USART_CR3_ONEBIT;

                // Configure baudrate
                USART2->BRR = _twr_uart_brr_t[baudrate];

                NVIC_EnableIRQ(USART2_IRQn);

                _twr_uart[channel].usart = USART2;
            }

            break;
        }
        case TWR_UART_UART2:
        {
            twr_gpio_init(TWR_GPIO_P11); // TXD2
            twr_gpio_init(TWR_GPIO_P10); // RXD2

            // Enable pull-up on RXD2 pin
            twr_gpio_set_pull(TWR_GPIO_P10, TWR_GPIO_PULL_UP);

            // Select AF6 alternate function for TXD0 and RXD0 pins
            twr_gpio_set_mode(TWR_GPIO_P11, TWR_GPIO_MODE_ALTERNATE_4);
            twr_gpio_set_mode(TWR_GPIO_P10, TWR_GPIO_MODE_ALTERNATE_4);

            // Enable clock for USART1
            RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

            // Errata workaround
            RCC->APB2ENR;

            // Enable transmitter and receiver, peripheral enabled in stop mode
            USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UESM;

            // Clock enabled in stop mode, disable overrun detection, one bit sampling method
            USART1->CR3 = USART_CR3_UCESM | USART_CR3_OVRDIS | USART_CR3_ONEBIT;

            // Configure baudrate
            USART1->BRR = _twr_uart_brr_t[baudrate];

            NVIC_EnableIRQ(USART1_IRQn);

            _twr_uart[channel].usart = USART1;

            break;
        }
        default:
        {
            return;
        }
    }

    // Clear ISR bits
    _twr_uart[channel].usart->ICR |= USART_ICR_CMCF | USART_ICR_IDLECF | USART_ICR_FECF;

    // Stop bits
    _twr_uart[channel].usart->CR2 &= ~USART_CR2_STOP_Msk;
    _twr_uart[channel].usart->CR2 |= ((uint32_t) setting & 0x03) << USART_CR2_STOP_Pos;

    // Parity
    _twr_uart[channel].usart->CR1 &= ~(USART_CR1_PCE_Msk | USART_CR1_PS_Msk);
    _twr_uart[channel].usart->CR1 |= (((uint32_t) setting >> 2) & 0x03) << USART_CR1_PS_Pos;

    // Word length
    _twr_uart[channel].usart->CR1 &= ~(USART_CR1_M1_Msk | USART_CR1_M0_Msk);

    uint32_t word_length = setting >> 4;

    if ((setting & 0x0c) != 0)
    {
        word_length++;
    }

    if (word_length == 0x07)
    {
        word_length = 0x10;

        _twr_uart[channel].usart->CR1 |= 1 << USART_CR1_M1_Pos;
    }
    else if (word_length == 0x09)
    {
        _twr_uart[channel].usart->CR1 |= 1 << USART_CR1_M0_Pos;
    }

    // Enable UART
    _twr_uart[channel].usart->CR1 |= USART_CR1_UE;

    _twr_uart[channel].initialized = true;
}


void twr_uart_deinit(twr_uart_channel_t channel)
{
    twr_uart_async_read_cancel(channel);

    // Disable UART
    _twr_uart[channel].usart->CR1 &= ~USART_CR1_UE_Msk;

    if (_twr_uart[channel].async_write_task_id != 0) {
        twr_scheduler_unregister(_twr_uart[channel].async_write_task_id);
    }

    switch(channel)
    {
        case TWR_UART_UART0:
        {
            NVIC_DisableIRQ(USART4_5_IRQn);

            // Disable clock for USART4
            RCC->APB1ENR &= ~RCC_APB1ENR_USART4EN;

            // Disable pull-up on RXD0 pin
            twr_gpio_set_pull(TWR_GPIO_P1, TWR_GPIO_PULL_NONE);

            // Configure TXD0 and RXD0 pins as Analog
            twr_gpio_set_mode(TWR_GPIO_P0, TWR_GPIO_MODE_ANALOG);
            twr_gpio_set_mode(TWR_GPIO_P1, TWR_GPIO_MODE_ANALOG);
            break;
        }
        case TWR_UART_UART1:
        {
            if (_twr_uart[channel].usart == LPUART1)
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

            // Disable pull-up on RXD1 pin
            twr_gpio_set_pull(TWR_GPIO_P3, TWR_GPIO_PULL_NONE);

            // Configure TXD1 and RXD1 pins as Analog
            twr_gpio_set_mode(TWR_GPIO_P2, TWR_GPIO_MODE_ANALOG);
            twr_gpio_set_mode(TWR_GPIO_P3, TWR_GPIO_MODE_ANALOG);
            break;
        }
        case TWR_UART_UART2:
        {
            NVIC_DisableIRQ(USART1_IRQn);

            // Disable clock for USART1
            RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN_Msk;

            // Disable pull-up on RXD2 pin
            twr_gpio_set_pull(TWR_GPIO_P10, TWR_GPIO_PULL_NONE);

            // Configure TXD2 and RXD2 pins as Analog
            twr_gpio_set_mode(TWR_GPIO_P11, TWR_GPIO_MODE_ANALOG);
            twr_gpio_set_mode(TWR_GPIO_P10, TWR_GPIO_MODE_ANALOG);
            break;
        }
        default:
        {
            return;
        }
    }

    _twr_uart[channel].initialized = false;
}

size_t twr_uart_write(twr_uart_channel_t channel, const void *buffer, size_t length)
{
    if (!_twr_uart[channel].initialized || _twr_uart[channel].async_write_in_progress)
    {
        return 0;
    }

    USART_TypeDef *usart = _twr_uart[channel].usart;

    size_t bytes_written = 0;

    if (_twr_uart[channel].usart == LPUART1)
    {
        twr_system_hsi16_enable();
    } else {
        twr_system_pll_enable();
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

    if (_twr_uart[channel].usart == LPUART1)
    {
        twr_system_hsi16_disable();
    }
    else
    {
        twr_system_pll_disable();
    }

    return bytes_written;
}

size_t twr_uart_read(twr_uart_channel_t channel, void *buffer, size_t length, twr_tick_t timeout)
{
    if (!_twr_uart[channel].initialized)
    {
        return 0;
    }

    USART_TypeDef *usart = _twr_uart[channel].usart;

    size_t bytes_read = 0;

    if (_twr_uart[channel].usart != LPUART1)
    {
        twr_system_pll_enable();
    }

    twr_tick_t tick_timeout = timeout == TWR_TICK_INFINITY ? TWR_TICK_INFINITY : twr_tick_get() + timeout;

    while (bytes_read != length)
    {
        // If timeout condition is met...
        if (twr_tick_get() >= tick_timeout)
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

    if (_twr_uart[channel].usart != LPUART1)
    {
        twr_system_pll_disable();
    }

    return bytes_read;
}

void twr_uart_set_event_handler(twr_uart_channel_t channel, void (*event_handler)(twr_uart_channel_t, twr_uart_event_t, void *), void *event_param)
{
    _twr_uart[channel].event_handler = event_handler;
    _twr_uart[channel].event_param = event_param;
}

void twr_uart_set_async_fifo(twr_uart_channel_t channel, twr_fifo_t *write_fifo, twr_fifo_t *read_fifo)
{
    _twr_uart[channel].write_fifo = write_fifo;
    _twr_uart[channel].read_fifo = read_fifo;
}

size_t twr_uart_async_write(twr_uart_channel_t channel, const void *buffer, size_t length)
{
    if (!_twr_uart[channel].initialized || _twr_uart[channel].write_fifo == NULL)
    {
        return 0;
    }

    size_t bytes_written = 0;

    for (size_t i = 0; i < length; i += 16)
    {
        bytes_written += twr_fifo_write(_twr_uart[channel].write_fifo, (uint8_t *)buffer + i, length - i  > 16 ? 16 : length - i);
    }

    if (bytes_written != 0)
    {
        if (!_twr_uart[channel].async_write_in_progress)
        {
            _twr_uart[channel].async_write_task_id = twr_scheduler_register(_twr_uart_async_write_task, (void *) channel, TWR_TICK_INFINITY);

            if (_twr_uart[channel].usart == LPUART1)
            {
                twr_system_hsi16_enable();
            }
            else
            {
                twr_system_pll_enable();
            }
        }
        else
        {
            twr_scheduler_plan_absolute(_twr_uart[channel].async_write_task_id, TWR_TICK_INFINITY);
        }

        twr_irq_disable();

        // Enable transmit interrupt
        _twr_uart[channel].usart->CR1 |= USART_CR1_TXEIE;

        twr_irq_enable();

        _twr_uart[channel].async_write_in_progress = true;
    }

    return bytes_written;
}

bool twr_uart_async_read_start(twr_uart_channel_t channel, twr_tick_t timeout)
{
    if (!_twr_uart[channel].initialized || _twr_uart[channel].read_fifo == NULL || _twr_uart[channel].async_read_in_progress)
    {
        return false;
    }

    _twr_uart[channel].async_timeout = timeout;

    _twr_uart[channel].async_read_task_id = twr_scheduler_register(_twr_uart_async_read_task, (void *) channel, _twr_uart[channel].async_timeout);

    if (channel == TWR_UART_UART2)
    {
        twr_dma_channel_config_t config = {
                .request = TWR_DMA_REQUEST_3,
                .direction = TWR_DMA_DIRECTION_TO_RAM,
                .data_size_memory = TWR_DMA_SIZE_1,
                .data_size_peripheral = TWR_DMA_SIZE_1,
                .length = _twr_uart[channel].read_fifo->size,
                .mode = TWR_DMA_MODE_CIRCULAR,
                .address_memory = _twr_uart[channel].read_fifo->buffer,
                .address_peripheral = (void *) &_twr_uart[channel].usart->RDR,
                .priority = TWR_DMA_PRIORITY_HIGH
        };

        twr_dma_init();

        twr_dma_channel_config(TWR_DMA_CHANNEL_3, &config);

        _twr_uart_2_dma.read_task_id = twr_scheduler_register(_twr_uart_2_dma_read_task, (void *) channel, 0);

        twr_irq_disable();
        // Enable receive DMA interrupt
        _twr_uart[channel].usart->CR3 |=  USART_CR3_DMAR;
        twr_irq_enable();

        twr_dma_channel_run(TWR_DMA_CHANNEL_3);
    }
    else
    {
        twr_irq_disable();
        // Enable receive interrupt
        _twr_uart[channel].usart->CR1 |= USART_CR1_RXNEIE;
        twr_irq_enable();
    }

    if (_twr_uart[channel].usart != LPUART1)
    {
        twr_system_pll_enable();
    }

    _twr_uart[channel].async_read_in_progress = true;

    return true;
}

bool twr_uart_async_read_cancel(twr_uart_channel_t channel)
{
    if (!_twr_uart[channel].initialized || !_twr_uart[channel].async_read_in_progress)
    {
        return false;
    }

    _twr_uart[channel].async_read_in_progress = false;

    if (channel == TWR_UART_UART2)
    {
        twr_dma_channel_stop(TWR_DMA_CHANNEL_3);

        twr_scheduler_unregister(_twr_uart_2_dma.read_task_id);

        twr_irq_disable();
        // Disable receive DMA interrupt
        _twr_uart[channel].usart->CR3 &= ~USART_CR3_DMAR_Msk;
        twr_irq_enable();
    }
    else
    {
        twr_irq_disable();

        // Disable receive interrupt
        _twr_uart[channel].usart->CR1 &= ~USART_CR1_RXNEIE_Msk;

        twr_irq_enable();
    }

    if (_twr_uart[channel].usart != LPUART1)
    {
        twr_system_pll_disable();
    }

    twr_scheduler_unregister(_twr_uart[channel].async_read_task_id);

    return false;
}

size_t twr_uart_async_read(twr_uart_channel_t channel, void *buffer, size_t length)
{
    if (!_twr_uart[channel].initialized || !_twr_uart[channel].async_read_in_progress)
    {
        return 0;
    }

    size_t bytes_read = twr_fifo_read(_twr_uart[channel].read_fifo, buffer, length);

    return bytes_read;
}

static void _twr_uart_async_write_task(void *param)
{
    twr_uart_channel_t channel = (twr_uart_channel_t) param;
    twr_uart_t *uart = &_twr_uart[channel];

    uart->async_write_in_progress = false;

    twr_scheduler_unregister(uart->async_write_task_id);

    if (uart->usart == LPUART1)
    {
        twr_system_hsi16_disable();
    }
    else
    {
        twr_system_pll_disable();
    }

    if (uart->event_handler != NULL)
    {
        uart->event_handler(channel, TWR_UART_EVENT_ASYNC_WRITE_DONE, uart->event_param);
    }
}

static void _twr_uart_async_read_task(void *param)
{
    twr_uart_channel_t channel = (twr_uart_channel_t) param;
    twr_uart_t *uart = &_twr_uart[channel];

    twr_scheduler_plan_current_relative(uart->async_timeout);

    if (uart->event_handler != NULL)
    {
        if (twr_fifo_is_empty(uart->read_fifo))
        {
            uart->event_handler(channel, TWR_UART_EVENT_ASYNC_READ_TIMEOUT, uart->event_param);
        }
        else
        {
            uart->event_handler(channel, TWR_UART_EVENT_ASYNC_READ_DATA, uart->event_param);
        }
    }
}

static void _twr_uart_2_dma_read_task(void *param)
{
    (void) param;

    size_t length = twr_dma_channel_get_length(TWR_DMA_CHANNEL_3);

    twr_uart_t *uart = &_twr_uart[TWR_UART_UART2];

    if (_twr_uart_2_dma.length != length)
    {
        uart->read_fifo->head = uart->read_fifo->size - length;

        _twr_uart_2_dma.length = length;

        twr_scheduler_plan_now(uart->async_read_task_id);
    }

    twr_scheduler_plan_current_now();
}

static void _twr_uart_irq_handler(twr_uart_channel_t channel)
{
    USART_TypeDef *usart = _twr_uart[channel].usart;

    if ((usart->CR1 & USART_CR1_RXNEIE) != 0 && (usart->ISR & USART_ISR_RXNE) != 0)
    {
        uint8_t character;

        // Read receive data register
        character = usart->RDR;

        twr_fifo_irq_write(_twr_uart[channel].read_fifo, &character, 1);

        twr_scheduler_plan_now(_twr_uart[channel].async_read_task_id);
    }

    // If it is transmit interrupt...
    if ((usart->CR1 & USART_CR1_TXEIE) != 0 && (usart->ISR & USART_ISR_TXE) != 0)
    {
        uint8_t character;

        // If there are still data in FIFO...
        if (twr_fifo_irq_read(_twr_uart[channel].write_fifo, &character, 1) != 0)
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

        twr_scheduler_plan_now(_twr_uart[channel].async_write_task_id);
    }
}

void AES_RNG_LPUART1_IRQHandler(void)
{
    _twr_uart_irq_handler(TWR_UART_UART1);
}

void USART1_IRQHandler(void)
{
    _twr_uart_irq_handler(TWR_UART_UART2);
}

void USART2_IRQHandler(void)
{
    _twr_uart_irq_handler(TWR_UART_UART1);
}

void USART4_5_IRQHandler(void)
{
    _twr_uart_irq_handler(TWR_UART_UART0);
}
