#include <bc_adc.h>
#include <bc_scheduler.h>
#include <bc_common.h>
#include <bc_irq.h>
#include <bc_module_core.h>
#include <stm32l083xx.h>

#define _BC_ADC_CHANNEL_NONE (BC_ADC_CHANNEL_A5 + 1)

typedef struct
{
    bc_adc_reference_t reference;
    bc_adc_format_t format;
    void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *);
    void *event_param;
    uint32_t chselr;
} bc_adc_config_t;

static bool _bc_adc_initialized;
static bc_adc_channel_t _bc_adc_channel_in_progress = _BC_ADC_CHANNEL_NONE;
static bc_adc_config_t _bc_adc_channel_table[BC_ADC_CHANNEL_A5 + 1] =
{
    [BC_ADC_CHANNEL_A0].chselr = ADC_CHSELR_CHSEL0,
    [BC_ADC_CHANNEL_A1].chselr = ADC_CHSELR_CHSEL1,
    [BC_ADC_CHANNEL_A2].chselr = ADC_CHSELR_CHSEL2,
    [BC_ADC_CHANNEL_A3].chselr = ADC_CHSELR_CHSEL3,
    [BC_ADC_CHANNEL_A4].chselr = ADC_CHSELR_CHSEL4,
    [BC_ADC_CHANNEL_A5].chselr = ADC_CHSELR_CHSEL5
};

static void _bc_adc_task();
bc_scheduler_task_id_t _bc_adc_task_id;

void bc_adc_init(bc_adc_channel_t channel, bc_adc_reference_t reference, bc_adc_format_t format)
{
    if (_bc_adc_initialized != true)
    {
        _bc_adc_initialized = true;

        // Enable ADC clock
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

        // Errata workaround
        RCC->APB2ENR;

        // Disable ADC peripheral
        ADC1->CR &= ~ADC_CR_ADEN;

        // Set auto-off mode, left align
        ADC1->CFGR1 |= ADC_CFGR1_AUTOFF | ADC_CFGR1_ALIGN;

        // Enable Over-sampler with over-sampling ratio (16x) and set PCLK/2 as a clock source
        ADC1->CFGR2 = ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0 | ADC_CFGR2_CKMODE_0;

        // Sampling time selection (12.5 cycles)
        ADC1->SMPR |= ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0;

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

        NVIC_EnableIRQ(ADC1_COMP_IRQn);
    }

    bc_adc_set_reference(channel, reference);

    bc_adc_set_format(channel, format);

    _bc_adc_task_id = bc_scheduler_register(_bc_adc_task, NULL, BC_TICK_INFINITY);
}

void bc_adc_set_reference(bc_adc_channel_t channel, bc_adc_reference_t reference)
{
    _bc_adc_channel_table[channel].reference = reference;
}

bc_adc_reference_t bc_adc_get_reference(bc_adc_channel_t channel)
{
    return _bc_adc_channel_table[channel].reference;
}

void bc_adc_set_format(bc_adc_channel_t channel, bc_adc_format_t format)
{
    _bc_adc_channel_table[channel].format = format;
}

bc_adc_format_t bc_adc_get_format(bc_adc_channel_t channel)
{
    return _bc_adc_channel_table[channel].format;
}

bool bc_adc_read(bc_adc_channel_t channel, void *result)
{
    // If ongoing conversion ...
    if (_bc_adc_channel_in_progress != _BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    // Set ADC channel
    ADC1->CHSELR = _bc_adc_channel_table[channel].chselr;

    // Clear EOS, EOC, OVR and EOSMP flags (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS | ADC_ISR_EOC | ADC_ISR_OVR | ADC_ISR_EOSMP;

    // Start the AD measurement
    ADC1->CR |= ADC_CR_ADSTART;

    // wait for end of measurement
    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    bc_adc_get_result(channel, result);

    return true;
}

bool bc_adc_set_event_handler(bc_adc_channel_t channel, void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *), void *event_param)
{
    // Check ongoing on edited channel
    if (_bc_adc_channel_in_progress == channel)
    {
        return false;
    }

    bc_adc_config_t *adc = &_bc_adc_channel_table[channel];

    adc->event_handler = event_handler;
    adc->event_param = event_param;

    return true;
}

bool bc_adc_async_read(bc_adc_channel_t channel)
{
    // If ongoing conversion ...
    if (_bc_adc_channel_in_progress != _BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    _bc_adc_channel_in_progress = channel;

    // Set ADC channel
    ADC1->CHSELR = _bc_adc_channel_table[channel].chselr;

    // Clear EOS, EOC, OVR and EOSMP flags
    ADC1->ISR = ADC_ISR_EOS | ADC_ISR_EOC | ADC_ISR_OVR | ADC_ISR_EOSMP;

    bc_irq_disable();

    // Enable "End Of Sequence" interrupt
    ADC1->IER = ADC_IER_EOSIE;

    bc_irq_enable();

    // Start AD conversion
    ADC1->CR |= ADC_CR_ADSTART;

    return true;
}

void bc_adc_get_result(bc_adc_channel_t channel, void *result)
{
    uint32_t data = ADC1->DR;

    // TODO ... take care of reference ...

    switch (_bc_adc_channel_table[channel].format)
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
    case BC_ADC_FORMAT_FLOAT:
        // TODO Currently only approximate
        *(float *) result = (3.3 / 65536) * data;
        break;
    default:
        return;
        break;
    }
}

void ADC1_COMP_IRQHandler()
{
    // Plan ADC task
    bc_scheduler_plan_now(_bc_adc_task_id);

    // Disable "End Of Sequence" interrupt
    ADC1->IER = 0;

    // Clear EOS, EOC, OVR and EOSMP flags (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS | ADC_ISR_EOC | ADC_ISR_OVR | ADC_ISR_EOSMP;
}

static void _bc_adc_task()
{
    bc_adc_config_t *adc = &_bc_adc_channel_table[_bc_adc_channel_in_progress];
    bc_adc_channel_t pending_channel_result;

    // Update pending channel result
    pending_channel_result = _bc_adc_channel_in_progress;

    // Release ADC for further conversion
    _bc_adc_channel_in_progress = _BC_ADC_CHANNEL_NONE;

    // Perform event call-back
    adc->event_handler(pending_channel_result, BC_ADC_EVENT_DONE, adc->event_param);
}
