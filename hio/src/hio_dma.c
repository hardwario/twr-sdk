#include <hio_dma.h>
#include <hio_irq.h>
#include <hio_scheduler.h>
#include <hio_fifo.h>
#include <stm32l0xx.h>

#define _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(__CHANNEL) \
    if ((DMA1->ISR & DMA_ISR_GIF##__CHANNEL) != 0) \
    { \
        if ((DMA1->ISR & DMA_ISR_TEIF##__CHANNEL) != 0) \
        { \
            _hio_dma_irq_handler(HIO_DMA_CHANNEL_##__CHANNEL, HIO_DMA_EVENT_ERROR); \
        } \
        else if ((DMA1->ISR & DMA_ISR_HTIF##__CHANNEL) != 0) \
        { \
            _hio_dma_irq_handler(HIO_DMA_CHANNEL_##__CHANNEL, HIO_DMA_EVENT_HALF_DONE); \
            DMA1->IFCR |= DMA_IFCR_CHTIF##__CHANNEL; \
        } \
        else if ((DMA1->ISR & DMA_ISR_TCIF##__CHANNEL) != 0) \
        { \
            _hio_dma_irq_handler(HIO_DMA_CHANNEL_##__CHANNEL, HIO_DMA_EVENT_DONE); \
            DMA1->IFCR |= DMA_IFCR_CTCIF##__CHANNEL; \
        } \
    }

typedef struct
{
    uint8_t channel : 4;
    uint8_t event : 4;

} hio_dma_pending_event_t;

static hio_dma_pending_event_t _hio_dma_pending_event_buffer[2 * 7 * sizeof(hio_dma_pending_event_t)];

static struct
{
    bool is_initialized;

    struct
    {
        DMA_Channel_TypeDef *instance;
        void (*event_handler)(hio_dma_channel_t, hio_dma_event_t, void *);
        void *event_param;

    } channel[7];

    hio_fifo_t fifo_pending;
    hio_scheduler_task_id_t task_id;

} _hio_dma;

static void _hio_dma_task(void *param);

static void _hio_dma_irq_handler(hio_dma_channel_t channel, hio_dma_event_t event);

void hio_dma_init(void)
{
    if (_hio_dma.is_initialized)
    {
        return;
    }

    // Initialize channel instances
    _hio_dma.channel[HIO_DMA_CHANNEL_1].instance = DMA1_Channel1;
    _hio_dma.channel[HIO_DMA_CHANNEL_2].instance = DMA1_Channel2;
    _hio_dma.channel[HIO_DMA_CHANNEL_3].instance = DMA1_Channel3;
    _hio_dma.channel[HIO_DMA_CHANNEL_4].instance = DMA1_Channel4;
    _hio_dma.channel[HIO_DMA_CHANNEL_5].instance = DMA1_Channel5;
    _hio_dma.channel[HIO_DMA_CHANNEL_6].instance = DMA1_Channel6;
    _hio_dma.channel[HIO_DMA_CHANNEL_7].instance = DMA1_Channel7;

    hio_fifo_init(&_hio_dma.fifo_pending, _hio_dma_pending_event_buffer, sizeof(_hio_dma_pending_event_buffer));

    _hio_dma.task_id = hio_scheduler_register(_hio_dma_task, NULL, HIO_TICK_INFINITY);

    // Enable DMA1
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    // Errata workaround
    RCC->AHBENR;

    // Enable DMA 1 channel 1 interrupt
    NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    // Enable DMA 1 channel 2 and 3 interrupts
    NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    // Enable DMA 1 channel 4, 5, 6 and 7 interrupts
    NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 2);
    NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
}

void hio_dma_channel_config(hio_dma_channel_t channel, hio_dma_channel_config_t *config)
{
    DMA_Channel_TypeDef *dma_channel = _hio_dma.channel[channel].instance;

    uint32_t dma_cselr_pos = channel * 4;

    hio_irq_disable();

    // Set DMA direction
    if (config->direction == HIO_DMA_DIRECTION_TO_PERIPHERAL)
    {
        dma_channel->CCR |= DMA_CCR_DIR;
    }
    else
    {
        dma_channel->CCR &= ~DMA_CCR_DIR;
    }

    // Set memory data size
    dma_channel->CCR &= ~DMA_CCR_MSIZE_Msk;

    if (config->data_size_memory == HIO_DMA_SIZE_2)
    {
        dma_channel->CCR |= DMA_CCR_MSIZE_0;
    }
    else if (config->data_size_memory == HIO_DMA_SIZE_4)
    {
        dma_channel->CCR |= DMA_CCR_MSIZE_1;
    }

    // Set peripheral data size
    dma_channel->CCR &= ~DMA_CCR_PSIZE_Msk;

    if (config->data_size_peripheral == HIO_DMA_SIZE_2)
    {
        dma_channel->CCR |= DMA_CCR_PSIZE_0;
    }
    else if (config->data_size_peripheral == HIO_DMA_SIZE_4)
    {
        dma_channel->CCR |= DMA_CCR_PSIZE_1;
    }

    // Set DMA mode
    if (config->mode == HIO_DMA_MODE_STANDARD)
    {
        dma_channel->CCR &= ~DMA_CCR_CIRC;
    }
    else if (config->mode == HIO_DMA_MODE_CIRCULAR)
    {
        dma_channel->CCR |= DMA_CCR_CIRC;
    }

    // Set memory incrementation
    dma_channel->CCR |= DMA_CCR_MINC;

    // Set DMA channel priority
    dma_channel->CCR &= ~DMA_CCR_PL_Msk;
    dma_channel->CCR |= config->priority << DMA_CCR_PL_Pos;

    // Configure request selection for DMA1 Channel
    DMA1_CSELR->CSELR &= ~(0xf << dma_cselr_pos);
    DMA1_CSELR->CSELR |= config->request << dma_cselr_pos;

    // Configure DMA channel data length
    dma_channel->CNDTR = config->length;

    // Configure DMA channel source address
    dma_channel->CPAR = (uint32_t) config->address_peripheral;

    // Configure DMA channel destination address
    dma_channel->CMAR = (uint32_t) config->address_memory;

    // Enable the transfer complete, half-complete and error interrupts
    dma_channel->CCR |= DMA_CCR_TCIE | DMA_CCR_HTIE | DMA_CCR_TEIE;

    hio_irq_enable();
}

void hio_dma_set_event_handler(hio_dma_channel_t channel, void (*event_handler)(hio_dma_channel_t, hio_dma_event_t, void *), void *event_param)
{
    _hio_dma.channel[channel].event_handler = event_handler;
    _hio_dma.channel[channel].event_param = event_param;
}

void hio_dma_channel_run(hio_dma_channel_t channel)
{
    _hio_dma.channel[channel].instance->CCR |= DMA_CCR_EN;
}

void hio_dma_channel_stop(hio_dma_channel_t channel)
{
    _hio_dma.channel[channel].instance->CCR &= ~DMA_CCR_EN;
}

size_t hio_dma_channel_get_length(hio_dma_channel_t channel)
{
    return (size_t) _hio_dma.channel[channel].instance->CNDTR;
}

void _hio_dma_task(void *param)
{
    (void) param;

    hio_dma_pending_event_t pending_event;

    while (hio_fifo_read(&_hio_dma.fifo_pending, &pending_event, sizeof(hio_dma_pending_event_t)) != 0)
    {
        if (_hio_dma.channel[pending_event.channel].event_handler != NULL)
        {
            _hio_dma.channel[pending_event.channel].event_handler(pending_event.channel, pending_event.event, _hio_dma.channel[pending_event.channel].event_param);
        }
    }
}

void _hio_dma_irq_handler(hio_dma_channel_t channel, hio_dma_event_t event)
{
    if (event == HIO_DMA_EVENT_DONE && !(_hio_dma.channel[channel].instance->CCR & DMA_CCR_CIRC))
    {
        hio_dma_channel_stop(channel);
    }

    hio_dma_pending_event_t pending_event = { channel, event };

    hio_fifo_irq_write(&_hio_dma.fifo_pending, &pending_event, sizeof(hio_dma_pending_event_t));

    hio_scheduler_plan_now(_hio_dma.task_id);
}

void DMA1_Channel1_IRQHandler(void)
{
    _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(1);
}

void DMA1_Channel2_3_IRQHandler(void)
{
    _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(2);

    _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(3);
}

void DMA1_Channel4_5_6_7_IRQHandler(void)
{
    _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(4);

    _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(5);

    _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(6);

    _HIO_DMA_CHECK_IRQ_OF_CHANNEL_(7);
}
