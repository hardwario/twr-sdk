#include <twr_dac.h>
#include <twr_scheduler.h>
#include <twr_system.h>
#include <stm32l0xx.h>

// Approximate voltage to code constant
#define _TWR_DAC_VOLTAGE_TO_CODE_CONSTANT 19961

typedef struct
{
    bool is_initialized;
    bool is_in_progress;
    void (*event_handler)(twr_dac_channel_t, twr_dac_event_t, void *);
    void *event_param;

    struct
    {
        uint8_t *u8;
        uint16_t *u16;

    } dac_register;

    twr_dma_channel_t dma_channel;
    twr_dma_channel_config_t dma_config;

    TIM_TypeDef *tim;
    twr_dac_sample_rate_t sample_rate;

} twr_dac_channel_setup_t;

static struct
{
    twr_dac_channel_setup_t channel[2];

} _twr_dac;

static const twr_dac_channel_setup_t _twr_dac_channel_0_setup_default;

static const twr_dac_channel_setup_t _twr_dac_channel_1_setup_default;

static void _twr_dac_dma_handler(twr_dma_channel_t channel, twr_dma_event_t event, void *event_param);

void twr_dac_init(twr_dac_channel_t channel)
{
    if (_twr_dac.channel[channel].is_initialized)
    {
        return;
    }

    RCC->APB1ENR |= RCC_APB1ENR_DACEN;

    // Errata workaround
    RCC->AHBENR;

    if (channel == TWR_DAC_DAC0)
    {
        _twr_dac.channel[TWR_DAC_DAC0] = _twr_dac_channel_0_setup_default;

        // Enable DAC channel 0
        DAC->CR |= DAC_CR_EN1;
    }
    else if (channel == TWR_DAC_DAC1)
    {
        _twr_dac.channel[TWR_DAC_DAC1] = _twr_dac_channel_1_setup_default;

        // Enable DAC channel 1
        DAC->CR |= DAC_CR_EN2;
    }

    _twr_dac.channel[channel].is_initialized = true;
}

void twr_dac_deinit(twr_dac_channel_t channel)
{
    if (!_twr_dac.channel[channel].is_initialized)
    {
        return;
    }

    if (_twr_dac.channel[channel].is_in_progress)
    {
        twr_dac_async_stop(channel);
    }

    if (channel == TWR_DAC_DAC0)
    {
        // Disable DAC channel 0
        DAC->CR &= ~DAC_CR_EN1_Msk;
    }
    else if (channel == TWR_DAC_DAC1)
    {
        // Disable DAC channel 1
        DAC->CR &= ~DAC_CR_EN2_Msk;
    }

    _twr_dac.channel[channel].is_initialized = false;

    if (!_twr_dac.channel[TWR_DAC_DAC0].is_initialized && !_twr_dac.channel[TWR_DAC_DAC1].is_initialized)
    {
        RCC->APB1ENR &= ~RCC_APB1ENR_DACEN;
    }
}

void twr_dac_set_output(twr_dac_channel_t channel, const void *raw, twr_dac_format_t format)
{
    if (_twr_dac.channel[channel].is_in_progress)
    {
        return;
    }

    uint16_t *dac_value = _twr_dac.channel[channel].dac_register.u16;

    switch (format)
    {
        case TWR_DAC_FORMAT_8_BIT:
        {
            *_twr_dac.channel[channel].dac_register.u8 = *(uint8_t *) raw;
            break;
        }
        case TWR_DAC_FORMAT_16_BIT:
        {
            *dac_value = *(uint16_t *) raw;
            break;
        }
        case TWR_DAC_FORMAT_VOLTAGE:
        {
            *dac_value = *(float *) raw * _TWR_DAC_VOLTAGE_TO_CODE_CONSTANT;
            break;
        }
        default:
        {
            return;
        }
    }
}

void twr_dac_set_event_handler(twr_dac_channel_t channel, void (*event_handler)(twr_dac_channel_t, twr_dac_event_t, void *), void *event_param)
{
    _twr_dac.channel[channel].event_handler = event_handler;
    _twr_dac.channel[channel].event_param = event_param;
}

bool twr_dac_async_config(twr_dac_channel_t channel, twr_dac_config_t *config)
{
    if (_twr_dac.channel[channel].is_in_progress)
    {
        return false;
    }

    twr_dma_channel_config_t *dac_dma_config = &_twr_dac.channel[channel].dma_config;

    // Set peripheral address according to data size
    if (config->data_size == TWR_DAC_DATA_SIZE_8)
    {
        dac_dma_config->address_peripheral = _twr_dac.channel[channel].dac_register.u8;
    }
    else if (config->data_size == TWR_DAC_DATA_SIZE_16)
    {
        dac_dma_config->address_peripheral = _twr_dac.channel[channel].dac_register.u16;
    }

    // Set data-size
    if (config->data_size == TWR_DAC_DATA_SIZE_8)
    {
        dac_dma_config->data_size_memory = TWR_DMA_SIZE_1;
    }
    else if (config->data_size == TWR_DAC_DATA_SIZE_16)
    {
        dac_dma_config->data_size_memory = TWR_DMA_SIZE_2;
    }

    // Set DMA channel mode
    if (config->mode == TWR_DAC_MODE_SINGLE)
    {
        dac_dma_config->mode = TWR_DMA_MODE_STANDARD;
    }
    else if (config->mode == TWR_DAC_MODE_CIRCULAR)
    {
        dac_dma_config->mode = TWR_DMA_MODE_CIRCULAR;
    }

    dac_dma_config->length = config->length;

    dac_dma_config->address_memory = config->buffer;

    _twr_dac.channel[channel].sample_rate = config->sample_rate;

    return true;
}

bool twr_dac_async_run(twr_dac_channel_t channel)
{
    twr_dac_channel_setup_t *dac_channel_setup = &_twr_dac.channel[channel];

    if (dac_channel_setup->is_in_progress)
    {
        return false;
    }

    twr_system_pll_enable();

    if (channel == TWR_DAC_DAC0)
    {
        DAC->CR &= ~(DAC_CR_TSEL1_Msk);

        // DMA transfer with timer 6 TRGO event as a trigger
        DAC->CR |= DAC_CR_DMAEN1 | DAC_CR_TEN1;

        // Enable time-base timer clock
        RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    }
    else if (channel == TWR_DAC_DAC1)
    {
        DAC->CR &= ~(DAC_CR_TSEL2_Msk);

        // DMA transfer with timer 7 TRGO event as a trigger
        DAC->CR |= DAC_CR_DMAEN2 | DAC_CR_TEN2 | DAC_CR_TSEL2_0 | DAC_CR_TSEL2_2 | DAC_CR_WAVE1_0;

        // Enable time-base timer clock
        RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    }

    // Errata workaround
    RCC->AHBENR;

    TIM_TypeDef *tim = _twr_dac.channel[channel].tim;

    // Set prescaler - Tout 0.5us
    tim->PSC = 16 - 1;

    // Configure auto-reload register according to desired sample rate (125us or 62.5us)
    if (dac_channel_setup->sample_rate == TWR_DAC_SAMPLE_RATE_8K)
    {
        tim->ARR = 250 - 1;
    }
    else if (dac_channel_setup->sample_rate == TWR_DAC_SAMPLE_RATE_16K)
    {
        tim->ARR = 125 - 1;
    }

    // Enable update event generation
    tim->EGR = TIM_EGR_UG;

    // Set the update event as a trigger output (TRGO)
    tim->CR2 = TIM_CR2_MMS_1;

    twr_dma_init();

    // Update DMA channel with image of DAC DMA channel
    twr_dma_channel_config(dac_channel_setup->dma_channel, &dac_channel_setup->dma_config);

    twr_dma_set_event_handler(dac_channel_setup->dma_channel, _twr_dac_dma_handler, (void *)channel);

    twr_dma_channel_run(dac_channel_setup->dma_channel);

    // Start timer
    tim->CR1 |= TIM_CR1_CEN;

    dac_channel_setup->is_in_progress = true;

    return true;
}

void twr_dac_async_stop(twr_dac_channel_t channel)
{
    twr_dac_channel_setup_t *dac_channel_setup = &_twr_dac.channel[channel];

    if (!dac_channel_setup->is_in_progress)
    {
        return;
    }

    // Stop timer
    dac_channel_setup->tim->CR1 &= ~TIM_CR1_CEN;

    twr_dma_channel_stop(dac_channel_setup->dma_channel);

    if (channel == TWR_DAC_DAC0)
    {
        DAC->CR &= ~(DAC_CR_DMAEN1_Msk | DAC_CR_TEN1_Msk | DAC_CR_TSEL1_Msk);

        // Disable time-base timer clock
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM6EN;
    }
    else if (channel == TWR_DAC_DAC1)
    {
        DAC->CR &= ~(DAC_CR_DMAEN2_Msk | DAC_CR_TEN2_Msk | DAC_CR_TSEL2_Msk);

        // Disable time-base timer clock
        RCC->APB1ENR &= ~RCC_APB1ENR_TIM7EN;
    }

    dac_channel_setup->is_in_progress = false;

    twr_system_pll_disable();
}

static void _twr_dac_dma_handler(twr_dma_channel_t channel, twr_dma_event_t event, void *event_param)
{
    (void) channel;

    twr_dac_channel_t dac_channel = (twr_dac_channel_t) event_param;
    twr_dac_channel_setup_t *dac_channel_setup = &_twr_dac.channel[dac_channel];

    if (event == TWR_DMA_EVENT_HALF_DONE)
    {
        if (dac_channel_setup->event_handler != NULL)
        {
            dac_channel_setup->event_handler(dac_channel, TWR_DAC_EVENT_HALF_DONE, dac_channel_setup->event_param);
        }
    }
    else if (event == TWR_DMA_EVENT_DONE)
    {
        if (dac_channel_setup->event_handler != NULL)
        {
            dac_channel_setup->event_handler(dac_channel, TWR_DAC_EVENT_DONE, dac_channel_setup->event_param);
        }

        if (dac_channel_setup->dma_config.mode != TWR_DMA_MODE_CIRCULAR)
        {
            twr_dac_async_stop(dac_channel);
        }
    }
    else if (event == TWR_DMA_EVENT_ERROR)
    {
        // TODO Do something
    }
}

static const twr_dac_channel_setup_t _twr_dac_channel_0_setup_default =
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

    .dma_channel = TWR_DMA_CHANNEL_2,

    .dma_config =
    {
        .request = TWR_DMA_REQUEST_9,
        .direction = TWR_DMA_DIRECTION_TO_PERIPHERAL,
        .data_size_peripheral = TWR_DMA_SIZE_2,
        .priority = TWR_DMA_PRIORITY_LOW
    },
    .tim = TIM6,
    .sample_rate = TWR_DAC_SAMPLE_RATE_8K
};

static const twr_dac_channel_setup_t _twr_dac_channel_1_setup_default =
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

    .dma_channel = TWR_DMA_CHANNEL_4,
    {
        .request = TWR_DMA_REQUEST_15,
        .direction = TWR_DMA_DIRECTION_TO_PERIPHERAL,
        .data_size_peripheral = TWR_DMA_SIZE_2,
        .priority = TWR_DMA_PRIORITY_LOW
    },
    .tim = TIM7,
    .sample_rate = TWR_DAC_SAMPLE_RATE_8K
};
