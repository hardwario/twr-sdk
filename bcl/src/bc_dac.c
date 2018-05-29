#include <bc_dac.h>
#include <bc_scheduler.h>
#include <bc_system.h>
#include <stm32l0xx.h>

// Approximate voltage to code constant
#define _BC_DAC_VOLTAGE_TO_CODE_CONSTANT 19961

typedef struct
{
    bool is_initialized;
    bool is_in_progress;
    void (*event_handler)(bc_dac_channel_t, bc_dac_event_t, void *);
    void *event_param;

    struct
    {
        uint8_t *u8;
        uint16_t *u16;

    } dac_register;

    bc_dma_channel_t dma_channel;
    bc_dma_channel_config_t dma_config;

    TIM_TypeDef *tim;
    bc_dac_sample_rate_t sample_rate;

} bc_dac_channel_setup_t;

static struct
{
    bc_dac_channel_setup_t channel[2];

} _bc_dac;

static const bc_dac_channel_setup_t _bc_dac_channel_0_setup_default;

static const bc_dac_channel_setup_t _bc_dac_channel_1_setup_default;

static void _bc_dac_dma_handler(bc_dma_channel_t channel, bc_dma_event_t event, void *event_param);

void bc_dac_init(bc_dac_channel_t channel)
{
    if (_bc_dac.channel[channel].is_initialized)
    {
        return;
    }

    RCC->APB1ENR |= RCC_APB1ENR_DACEN;

    // Errata workaround
    RCC->AHBENR;

    if (channel == BC_DAC_DAC0)
    {
        _bc_dac.channel[BC_DAC_DAC0] = _bc_dac_channel_0_setup_default;

        // Enable DAC channel 0
        DAC->CR |= DAC_CR_EN1;
    }
    else if (channel == BC_DAC_DAC1)
    {
        _bc_dac.channel[BC_DAC_DAC1] = _bc_dac_channel_1_setup_default;

        // Enable DAC channel 1
        DAC->CR |= DAC_CR_EN2;
    }

    _bc_dac.channel[channel].is_initialized = true;
}

void bc_dac_deinit(bc_dac_channel_t channel)
{
    if (!_bc_dac.channel[channel].is_initialized)
    {
        return;
    }

    if (_bc_dac.channel[channel].is_in_progress)
    {
        bc_dac_async_stop(channel);
    }

    if (channel == BC_DAC_DAC0)
    {
        // Disable DAC channel 0
        DAC->CR &= ~DAC_CR_EN1_Msk;
    }
    else if (channel == BC_DAC_DAC1)
    {
        // Disable DAC channel 1
        DAC->CR &= ~DAC_CR_EN2_Msk;
    }

    _bc_dac.channel[channel].is_initialized = false;

    if (!_bc_dac.channel[BC_DAC_DAC0].is_initialized && !_bc_dac.channel[BC_DAC_DAC1].is_initialized)
    {
        RCC->APB1ENR &= ~RCC_APB1ENR_DACEN;
    }
}

void bc_dac_set_output(bc_dac_channel_t channel, const void *raw, bc_dac_format_t format)
{
    if (_bc_dac.channel[channel].is_in_progress)
    {
        return;
    }

    uint16_t *dac_value = _bc_dac.channel[channel].dac_register.u16;

    switch (format)
    {
        case BC_DAC_FORMAT_8_BIT:
        {
            *_bc_dac.channel[channel].dac_register.u8 = *(uint8_t *) raw;
            break;
        }
        case BC_DAC_FORMAT_16_BIT:
        {
            *dac_value = *(uint16_t *) raw;
            break;
        }
        case BC_DAC_FORMAT_VOLTAGE:
        {
            *dac_value = *(float *) raw * _BC_DAC_VOLTAGE_TO_CODE_CONSTANT;
            break;
        }
        default:
        {
            return;
        }
    }
}

void bc_dac_set_event_handler(bc_dac_channel_t channel, void (*event_handler)(bc_dac_channel_t, bc_dac_event_t, void *), void *event_param)
{
    _bc_dac.channel[channel].event_handler = event_handler;
    _bc_dac.channel[channel].event_param = event_param;
}

bool bc_dac_async_config(bc_dac_channel_t channel, bc_dac_config_t *config)
{
    if (_bc_dac.channel[channel].is_in_progress)
    {
        return false;
    }

    bc_dma_channel_config_t *dac_dma_config = &_bc_dac.channel[channel].dma_config;

    // Set peripheral address according to data size
    if (config->data_size == BC_DAC_DATA_SIZE_8)
    {
        dac_dma_config->address_peripheral = _bc_dac.channel[channel].dac_register.u8;
    }
    else if (config->data_size == BC_DAC_DATA_SIZE_16)
    {
        dac_dma_config->address_peripheral = _bc_dac.channel[channel].dac_register.u16;
    }

    // Set data-size
    if (config->data_size == BC_DAC_DATA_SIZE_8)
    {
        dac_dma_config->data_size_memory = BC_DMA_SIZE_1;
    }
    else if (config->data_size == BC_DAC_DATA_SIZE_16)
    {
        dac_dma_config->data_size_memory = BC_DMA_SIZE_2;
    }

    // Set DMA channel mode
    if (config->mode == BC_DAC_MODE_SINGLE)
    {
        dac_dma_config->mode = BC_DMA_MODE_STANDARD;
    }
    else if (config->mode == BC_DAC_MODE_CIRCULAR)
    {
        dac_dma_config->mode = BC_DMA_MODE_CIRCULAR;
    }

    dac_dma_config->length = config->length;

    dac_dma_config->address_memory = config->buffer;

    _bc_dac.channel[channel].sample_rate = config->sample_rate;

    return true;
}

bool bc_dac_async_run(bc_dac_channel_t channel)
{
    bc_dac_channel_setup_t *dac_channel_setup = &_bc_dac.channel[channel];

    if (dac_channel_setup->is_in_progress)
    {
        return false;
    }

    bc_system_pll_enable();

    if (channel == BC_DAC_DAC0)
    {
        DAC->CR &= ~(DAC_CR_TSEL1_Msk);

        // DMA transfer with timer 6 TRGO event as a trigger
        DAC->CR |= DAC_CR_DMAEN1 | DAC_CR_TEN1;

        // Enable time-base timer clock
        RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    }
    else if (channel == BC_DAC_DAC1)
    {
        DAC->CR &= ~(DAC_CR_TSEL2_Msk);

        // DMA transfer with timer 7 TRGO event as a trigger
        DAC->CR |= DAC_CR_DMAEN2 | DAC_CR_TEN2 | DAC_CR_TSEL2_0 | DAC_CR_TSEL2_2 | DAC_CR_WAVE1_0;

        // Enable time-base timer clock
        RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    }

    // Errata workaround
    RCC->AHBENR;

    TIM_TypeDef *tim = _bc_dac.channel[channel].tim;

    // Set prescaler - Tout 0.5us
    tim->PSC = 16 - 1;

    // Configure auto-reload register according to desired sample rate (125us or 62.5us)
    if (dac_channel_setup->sample_rate == BC_DAC_SAMPLE_RATE_8K)
    {
        tim->ARR = 250 - 1;
    }
    else if (dac_channel_setup->sample_rate == BC_DAC_SAMPLE_RATE_16K)
    {
        tim->ARR = 125 - 1;
    }

    // Enable update event generation
    tim->EGR = TIM_EGR_UG;

    // Set the update event as a trigger output (TRGO)
    tim->CR2 = TIM_CR2_MMS_1;

    bc_dma_init();

    // Update DMA channel with image of DAC DMA channel
    bc_dma_channel_config(dac_channel_setup->dma_channel, &dac_channel_setup->dma_config);

    bc_dma_set_event_handler(dac_channel_setup->dma_channel, _bc_dac_dma_handler, (void *)channel);

    bc_dma_channel_run(dac_channel_setup->dma_channel);

    // Start timer
    tim->CR1 |= TIM_CR1_CEN;

    dac_channel_setup->is_in_progress = true;

    return true;
}

void bc_dac_async_stop(bc_dac_channel_t channel)
{
    bc_dac_channel_setup_t *dac_channel_setup = &_bc_dac.channel[channel];

    if (!dac_channel_setup->is_in_progress)
    {
        return;
    }

    // Stop timer
    dac_channel_setup->tim->CR1 &= ~TIM_CR1_CEN;

    bc_dma_channel_stop(dac_channel_setup->dma_channel);

    if (channel == BC_DAC_DAC0)
    {
        DAC->CR &= ~(DAC_CR_DMAEN1_Msk | DAC_CR_TEN1_Msk | DAC_CR_TSEL1_Msk);

        // Disable time-base timer clock
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM6EN;
    }
    else if (channel == BC_DAC_DAC1)
    {
        DAC->CR &= ~(DAC_CR_DMAEN2_Msk | DAC_CR_TEN2_Msk | DAC_CR_TSEL2_Msk);

        // Disable time-base timer clock
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM7EN;
    }

    dac_channel_setup->is_in_progress = false;

    bc_system_pll_disable();
}

static void _bc_dac_dma_handler(bc_dma_channel_t channel, bc_dma_event_t event, void *event_param)
{
    (void) channel;

    bc_dac_channel_t dac_channel = (bc_dac_channel_t) event_param;
    bc_dac_channel_setup_t *dac_channel_setup = &_bc_dac.channel[dac_channel];

    if (event == BC_DMA_EVENT_HALF_DONE)
    {
        if (dac_channel_setup->event_handler != NULL)
        {
            dac_channel_setup->event_handler(dac_channel, BC_DAC_EVENT_HALF_DONE, dac_channel_setup->event_param);
        }
    }
    else if (event == BC_DMA_EVENT_DONE)
    {
        if (dac_channel_setup->event_handler != NULL)
        {
            dac_channel_setup->event_handler(dac_channel, BC_DAC_EVENT_DONE, dac_channel_setup->event_param);
        }

        if (dac_channel_setup->dma_config.mode != BC_DMA_MODE_CIRCULAR)
        {
            bc_dac_async_stop(dac_channel);
        }
    }
    else if (event == BC_DMA_EVENT_ERROR)
    {
        // TODO Do something
    }
}

static const bc_dac_channel_setup_t _bc_dac_channel_0_setup_default =
{
    .is_initialized = false,
    .is_in_progress = false,
    .event_handler = NULL,
    .event_param = NULL,

    .dac_register =
    {
        .u8 = (void *)&DAC->DHR8R1,
        .u16 = (void *)&DAC->DHR12L1
    },

    .dma_channel = BC_DMA_CHANNEL_2,

    .dma_config =
    {
        .request = BC_DMA_REQUEST_9,
        .direction = BC_DMA_DIRECTION_TO_PERIPHERAL,
        .data_size_peripheral = BC_DMA_SIZE_2,
        .priority = BC_DMA_PRIORITY_LOW
    },
    .tim = TIM6,
    .sample_rate = BC_DAC_SAMPLE_RATE_8K
};

static const bc_dac_channel_setup_t _bc_dac_channel_1_setup_default =
{
    .is_initialized = false,
    .is_in_progress = false,
    .event_handler = NULL,
    .event_param = NULL,

    .dac_register =
    {
        .u8 = (void *)&DAC->DHR8R2,
        .u16 = (void *)&DAC->DHR12L2
    },

    .dma_channel = BC_DMA_CHANNEL_4,
    {
        .request = BC_DMA_REQUEST_15,
        .direction = BC_DMA_DIRECTION_TO_PERIPHERAL,
        .data_size_peripheral = BC_DMA_SIZE_2,
        .priority = BC_DMA_PRIORITY_LOW
    },
    .tim = TIM7,
    .sample_rate = BC_DAC_SAMPLE_RATE_8K
};
