#include <bc_uart.h>
#include <bc_scheduler.h>
#include <bc_irq.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

static struct
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

} _bc_uart1 = { .initialized = false };

static void _bc_uart1_init(bc_uart_config_t config);

static size_t _bc_uart1_write(const void *buffer, size_t length);

static size_t _bc_uart1_read(void *buffer, size_t length, bc_tick_t timeout);

static void _bc_uart1_set_event_handler(void (*event_handler)(bc_uart_channel_t, bc_uart_event_t, void *), void *event_param);

static void _bc_uart1_set_async_fifo(bc_fifo_t *write_fifo, bc_fifo_t *read_fifo);

static size_t _bc_uart1_async_write(const void *buffer, size_t length);

static void _bc_uart1_async_write_task(void *param);

static bool _bc_uart1_async_read_start(bc_tick_t timeout);

static bool _bc_uart1_async_read_cancel(void);

static size_t _bc_uart1_async_read(void *buffer, size_t length);

static void _bc_uart1_async_read_task(void *param);

static void _bc_uart1_irq_handler(void);

void bc_uart_init(bc_uart_channel_t channel, bc_uart_config_t config)
{
    if (channel == BC_UART_UART1)
    {
        _bc_uart1_init(config);
    }
}

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length)
{
    if (channel == BC_UART_UART1)
    {
        return _bc_uart1_write(buffer, length);
    }

    return 0;
}

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout)
{
    if (channel == BC_UART_UART1)
    {
        return _bc_uart1_read(buffer, length, timeout);
    }

    return 0;
}

void bc_uart_set_event_handler(bc_uart_channel_t channel, void (*event_handler)(bc_uart_channel_t, bc_uart_event_t, void *), void *event_param)
{
    if (channel == BC_UART_UART1)
    {
        _bc_uart1_set_event_handler(event_handler, event_param);
    }
}

void bc_uart_set_async_fifo(bc_uart_channel_t channel, bc_fifo_t *write_fifo, bc_fifo_t *read_fifo)
{
    if (channel == BC_UART_UART1)
    {
        _bc_uart1_set_async_fifo(write_fifo, read_fifo);
    }
}

size_t bc_uart_async_write(bc_uart_channel_t channel, const void *buffer, size_t length)
{
    if (channel == BC_UART_UART1)
    {
        return _bc_uart1_async_write(buffer, length);
    }

    return 0;
}

bool bc_uart_async_read_start(bc_uart_channel_t channel, bc_tick_t timeout)
{
    if (channel == BC_UART_UART1)
    {
        return _bc_uart1_async_read_start(timeout);
    }

    return false;
}

bool bc_uart_async_read_cancel(bc_uart_channel_t channel)
{
    if (channel == BC_UART_UART1)
    {
        return _bc_uart1_async_read_cancel();
    }

    return false;
}

size_t bc_uart_async_read(bc_uart_channel_t channel, void *buffer, size_t length)
{
    if (channel == BC_UART_UART1)
    {
        return _bc_uart1_async_read(buffer, length);
    }

    return 0;
}

static void _bc_uart1_init(bc_uart_config_t config)
{
    (void) config;

    memset(&_bc_uart1, 0, sizeof(_bc_uart1));

    // Enable GPIOA clock
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Errata workaround
    RCC->IOPENR;

    // Enable pull-up on RXD1 pin
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;

    // Configure TXD1 and RXD1 pins as alternate function
    GPIOA->MODER &= ~(1 << GPIO_MODER_MODE3_Pos | 1 << GPIO_MODER_MODE2_Pos);

    // Select AF6 alternate function for TXD1 and RXD1 pins
    GPIOA->AFR[0] |= 6 << GPIO_AFRL_AFRL3_Pos | 6 << GPIO_AFRL_AFRL2_Pos;

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

    // Enable LPUART1
    LPUART1->CR1 |= USART_CR1_UE;

    NVIC_EnableIRQ(LPUART1_IRQn);

    _bc_uart1.initialized = true;
}

static size_t _bc_uart1_write(const void *buffer, size_t length)
{
    if (!_bc_uart1.initialized || _bc_uart1.async_write_in_progress)
    {
        return 0;
    }

    size_t bytes_written = 0;

    // TODO Not needed anymore?
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

    // TODO Not needed anymore?
    bc_module_core_pll_disable();

    return bytes_written;
}

static size_t _bc_uart1_read(void *buffer, size_t length, bc_tick_t timeout)
{
    if (!_bc_uart1.initialized)
    {
        return 0;
    }

    size_t bytes_read = 0;

    // TODO Not needed anymore?
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

    // TODO Not needed anymore?
    bc_module_core_pll_disable();

    return bytes_read;
}

static void _bc_uart1_set_event_handler(void (*event_handler)(bc_uart_channel_t, bc_uart_event_t, void *), void *event_param)
{
    _bc_uart1.event_handler = event_handler;
    _bc_uart1.event_param = event_param;
}

static void _bc_uart1_set_async_fifo(bc_fifo_t *write_fifo, bc_fifo_t *read_fifo)
{
    _bc_uart1.write_fifo = write_fifo;
    _bc_uart1.read_fifo = read_fifo;
}

static size_t _bc_uart1_async_write(const void *buffer, size_t length)
{
    if (!_bc_uart1.initialized || _bc_uart1.write_fifo == NULL)
    {
        return 0;
    }

    size_t bytes_written = bc_fifo_write(_bc_uart1.write_fifo, buffer, length);

    if (bytes_written != 0)
    {
        if (!_bc_uart1.async_write_in_progress)
        {
            _bc_uart1.async_write_task_id = bc_scheduler_register(_bc_uart1_async_write_task, NULL, BC_TICK_INFINITY);

            // TODO Better replace with disable sleep?
            bc_module_core_pll_enable();
        }
        else
        {
            bc_scheduler_plan_absolute(_bc_uart1.async_write_task_id, BC_TICK_INFINITY);
        }

        bc_irq_disable();

        // Enable transmit interrupt
        LPUART1->CR1 |= USART_CR1_TXEIE;

        bc_irq_enable();

        _bc_uart1.async_write_in_progress = true;
    }

    return bytes_written;
}

static void _bc_uart1_async_write_task(void *param)
{
    (void) param;

    _bc_uart1.async_write_in_progress = false;

    bc_scheduler_unregister(_bc_uart1.async_write_task_id);

    bc_module_core_pll_disable();

    if (_bc_uart1.event_handler != NULL)
    {
        _bc_uart1.event_handler(BC_UART_UART1, BC_UART_EVENT_ASYNC_WRITE_DONE, _bc_uart1.event_param);
    }
}

static bool _bc_uart1_async_read_start(bc_tick_t timeout)
{
    if (!_bc_uart1.initialized || _bc_uart1.read_fifo == NULL || _bc_uart1.async_read_in_progress)
    {
        return false;
    }

    _bc_uart1.async_timeout = timeout;

    _bc_uart1.async_read_task_id = bc_scheduler_register(_bc_uart1_async_read_task, NULL, _bc_uart1.async_timeout);

    bc_irq_disable();

    // Enable receive interrupt
    LPUART1->CR1 |= USART_CR1_RXNEIE;

    bc_irq_enable();

    _bc_uart1.async_read_in_progress = true;

    return true;
}

static bool _bc_uart1_async_read_cancel(void)
{
    if (!_bc_uart1.initialized || !_bc_uart1.async_read_in_progress)
    {
        return false;
    }

    _bc_uart1.async_read_in_progress = false;

    bc_irq_disable();

    // Disable receive interrupt
    LPUART1->CR1 &= ~USART_CR1_RXNEIE;

    bc_irq_enable();

    bc_scheduler_unregister(_bc_uart1.async_read_task_id);

    return false;
}

static size_t _bc_uart1_async_read(void *buffer, size_t length)
{
    if (!_bc_uart1.initialized || !_bc_uart1.async_read_in_progress)
    {
        return 0;
    }

    size_t bytes_read = bc_fifo_read(_bc_uart1.read_fifo, buffer, length);

    return bytes_read;
}

static void _bc_uart1_async_read_task(void *param)
{
    (void) param;

    bc_scheduler_plan_current_relative(_bc_uart1.async_timeout);

    if (_bc_uart1.event_handler != NULL)
    {
        // TODO Create API from the following construct in bc_fifo

        bc_irq_disable();

        if (_bc_uart1.read_fifo->tail != _bc_uart1.read_fifo->head)
        {
            bc_irq_enable();

            _bc_uart1.event_handler(BC_UART_UART1, BC_UART_EVENT_ASYNC_READ_DATA, _bc_uart1.event_param);
        }
        else
        {
            bc_irq_enable();

            _bc_uart1.event_handler(BC_UART_UART1, BC_UART_EVENT_ASYNC_READ_TIMEOUT, _bc_uart1.event_param);
        }
    }
}

static void _bc_uart1_irq_handler(void)
{
    // If it is transmit interrupt...
    if ((LPUART1->CR1 & USART_CR1_TXEIE) != 0 && (LPUART1->ISR & USART_ISR_TXE) != 0)
    {
        uint8_t character;

        // If there are still data in FIFO...
        if (bc_fifo_irq_read(_bc_uart1.write_fifo, &character, 1) != 0)
        {
            // Load transmit data register
            LPUART1->TDR = character;
        }
        else
        {
            // Disable transmit interrupt
            LPUART1->CR1 &= ~USART_CR1_TXEIE;

            // Enable transmission complete interrupt
            LPUART1->CR1 |= USART_CR1_TCIE;
        }
    }

    // If it is transmit interrupt...
    if ((LPUART1->CR1 & USART_CR1_TCIE) != 0 && (LPUART1->ISR & USART_ISR_TC) != 0)
    {
        // Disable transmission complete interrupt
        LPUART1->CR1 &= ~USART_CR1_TCIE;

        bc_scheduler_plan_now(_bc_uart1.async_write_task_id);
    }

    if ((LPUART1->CR1 & USART_CR1_RXNEIE) != 0 && (LPUART1->ISR & USART_ISR_RXNE) != 0)
    {
        uint8_t character;

        // Read receive data register
        character = LPUART1->RDR;

        bc_fifo_irq_write(_bc_uart1.read_fifo, &character, 1);

        bc_scheduler_plan_now(_bc_uart1.async_read_task_id);
    }
}

void AES_RNG_LPUART1_IRQHandler(void)
{
    _bc_uart1_irq_handler();
}
