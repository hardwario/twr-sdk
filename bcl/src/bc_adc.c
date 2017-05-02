#include <bc_common.h>
#include <bc_adc.h>
#include <bc_scheduler.h>
#include <bc_module_core.h>
#include <stm32l083xx.h>

// TODO Ok?
#define BC_ADC_CHANNEL_NONE (BC_ADC_CHANNEL_A5 + 1)

typedef struct
{
    bc_adc_reference_t reference;
    bc_adc_format_t format;
    void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *);
    void *event_param;
    uint32_t chselr;
} _bc_adc_config_t;

static bc_adc_channel_t _bc_adc_in_progress = BC_ADC_CHANNEL_NONE;
static bool _bc_adc_initialized;
static _bc_adc_config_t _bc_adc_config_table[6] =
{
    [0].chselr = ADC_CHSELR_CHSEL0,
    [1].chselr = ADC_CHSELR_CHSEL1,
    [2].chselr = ADC_CHSELR_CHSEL2,
    [3].chselr = ADC_CHSELR_CHSEL3,
    [4].chselr = ADC_CHSELR_CHSEL4,
    [5].chselr = ADC_CHSELR_CHSEL5
};

void _bc_adc_task();
bc_scheduler_task_id_t _bc_adc_task_id;

void bc_adc_init(bc_adc_channel_t channel, bc_adc_reference_t reference, bc_adc_format_t format)
{
    if (_bc_adc_initialized != true)
    {
        _bc_adc_initialized = true;

        // Enable ADC clock
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

        // TODO Errata

        // Disable ADC peripheral
        ADC1->CR &= ~ADC_CR_ADEN;

        // Set auto-off mode, left align
        ADC1->CFGR1 |= ADC_CFGR1_AUTOFF | ADC_CFGR1_ALIGN;

        // Enable Over-sampler with over-sampling ratio (16x) and set PCLK/2 as a clock source
        ADC1->CFGR2 = ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0 | ADC_CFGR2_CKMODE_0;

        // Sampling time selection (12.5 cycles)
        ADC1->SMPR |= ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0;

        // TODO ... enable conversion time (sample count + sampling time) corections

        // Perform ADC calibration
        ADC1->CR |= ADC_CR_ADCAL;
        while ((ADC1->ISR & ADC_ISR_EOCAL) == 0)
        {
            continue;
        }

        // Clear EOCAL flag
        ADC1->ISR |= ADC_ISR_EOCAL;

        // Enable the ADC
        ADC1->CR |= ADC_CR_ADEN;
    }

    bc_adc_set_reference(channel, reference);

    bc_adc_set_format(channel, format);

    _bc_adc_task_id = bc_scheduler_register(_bc_adc_task, NULL, BC_TICK_INFINITY);
}

void bc_adc_set_reference(bc_adc_channel_t channel, bc_adc_reference_t reference)
{
    _bc_adc_config_table[channel].reference = reference;
}

bc_adc_reference_t bc_adc_get_reference(bc_adc_channel_t channel)
{
    return _bc_adc_config_table[channel].reference;
}

void bc_adc_set_format(bc_adc_channel_t channel, bc_adc_format_t format)
{
    _bc_adc_config_table[channel].format = format;
}

bc_adc_format_t bc_adc_get_format(bc_adc_channel_t channel)
{
    return _bc_adc_config_table[channel].format;
}

bool bc_adc_measure(bc_adc_channel_t channel, void *result)
{
    // If ongoing conversion ...
    if (_bc_adc_in_progress != BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    // Set ADC channel
    ADC1->CHSELR = _bc_adc_config_table[channel].chselr;

    // Clear EOS, EOC, OVR and EOSMP flags
    ADC1->ISR = ADC_ISR_EOS | ADC_ISR_EOC | ADC_ISR_OVR | ADC_ISR_EOSMP;

    // Performs the AD conversion
    ADC1->CR |= ADC_CR_ADSTART;

    // wait for end of sequence
    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    bc_adc_get_result(channel, result);

    return true;
}

bool bc_adc_async_set_event_handler(bc_adc_channel_t channel, void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *), void *event_param)
{
    // Check ongoing on edited channel
    if (_bc_adc_in_progress == channel)
    {
        return false;
    }

    _bc_adc_config_t *adc = &_bc_adc_config_table[channel];

    adc->event_handler = event_handler;
    adc->event_param = event_param;

    return true;
}

bool bc_adc_async_measure(bc_adc_channel_t channel)
{
    // If ongoing conversion ...
    if (_bc_adc_in_progress != BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    _bc_adc_in_progress = channel;

    // Set ADC channel
    ADC1->CHSELR = _bc_adc_config_table[channel].chselr;

    // Clear EOS, EOC, OVR and EOSMP flags
    ADC1->ISR = ADC_ISR_EOS | ADC_ISR_EOC | ADC_ISR_OVR | ADC_ISR_EOSMP;

    // Enable "End Of Sequence" interrupt
    ADC1->IER = ADC_IER_EOSIE;

    NVIC_EnableIRQ(ADC1_COMP_IRQn);

    // Start AD conversion
    ADC1->CR |= ADC_CR_ADSTART;

    return true;
}

void bc_adc_get_result(bc_adc_channel_t channel, void *result)
{
    uint32_t data;

    data = ADC1->DR;

    // TODO ... take care of reference ...
    // TODO Maybe add voltage (auto-calibration)

    switch (_bc_adc_config_table[channel].format)
    {
    case BC_ADC_FORMAT_8_BIT:
        *(uint8_t *) result = data >> 8;
        break;
    case BC_ADC_FORMAT_16_BIT:
        *(uint16_t *) result = data;
        break;
    case BC_ADC_FORMAT_24_BIT:
        memcpy((uint8_t *) result + 1, &data, 2);
        break;
    case BC_ADC_FORMAT_32_BIT:
        *(uint32_t *) result = data << 16;
        break;
        /*
         case BC_ADC_FORMAT_FLOAT_BIT:
         1. measure Vrefint
         2. *(float *)result = (Vrefint(voltage) / Vrefint(code)) * data;
         break;
         */
    default:
        return;
        break;
    }
}

void ADC1_COMP_IRQHandler()
{
    // Plan ADC task
    bc_scheduler_plan_now(_bc_adc_task_id);

    // Clear flags
    ADC1->ISR |= (ADC_ISR_EOS | ADC_ISR_EOC | ADC_ISR_EOSMP);

    NVIC_DisableIRQ(ADC1_COMP_IRQn);
}

void _bc_adc_task()
{
    _bc_adc_config_t *adc = &_bc_adc_config_table[_bc_adc_in_progress];
    bc_adc_channel_t pending;

    // Release used channel for another possible conversion
    pending = _bc_adc_in_progress;
    _bc_adc_in_progress = BC_ADC_CHANNEL_NONE;

    // Perform event call-back
    adc->event_handler(pending, BC_ADC_EVENT_DONE, adc->event_param);
}
