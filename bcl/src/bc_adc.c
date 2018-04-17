#include <bc_adc.h>
#include <bc_scheduler.h>
#include <bc_irq.h>
#include <stm32l083xx.h>

#define VREFINT_CAL_ADDR 0x1ff80078

#define BC_ADC_CHANNEL_INTERNAL_REFERENCE 6
#define BC_ADC_CHANNEL_NONE ((bc_adc_channel_t) (-1))
#define BC_ADC_CHANNEL_COUNT ((bc_adc_channel_t) 7)

typedef enum
{
    BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN,
    BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END,
    BC_ADC_STATE_MEASURE_INPUT

} bc_adc_state_t;

typedef struct
{
    bc_adc_format_t format;
    void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *);
    void *event_param;
    bool pending;
    uint32_t chselr;

} bc_adc_channel_config_t;

static struct
{
    bool initialized;
    bc_adc_channel_t channel_in_progress;
    uint16_t vrefint;
    float real_vdda_voltage;
    bc_adc_state_t state;
    bc_scheduler_task_id_t task_id;
    bc_adc_channel_config_t channel_table[7];
}
_bc_adc =
{
    .initialized = false,
    .channel_in_progress = BC_ADC_CHANNEL_NONE,
    .channel_table =
    {
        [BC_ADC_CHANNEL_A0].chselr = ADC_CHSELR_CHSEL0,
        [BC_ADC_CHANNEL_A1].chselr = ADC_CHSELR_CHSEL1,
        [BC_ADC_CHANNEL_A2].chselr = ADC_CHSELR_CHSEL2,
        [BC_ADC_CHANNEL_A3].chselr = ADC_CHSELR_CHSEL3,
        [BC_ADC_CHANNEL_A4].chselr = ADC_CHSELR_CHSEL4,
        [BC_ADC_CHANNEL_A5].chselr = ADC_CHSELR_CHSEL5,
        [BC_ADC_CHANNEL_INTERNAL_REFERENCE] = { BC_ADC_FORMAT_16_BIT, NULL, NULL, false, ADC_CHSELR_CHSEL17 }
    }
};

static void _bc_adc_task(void *param);

static inline bool _bc_adc_get_pending(bc_adc_channel_t *next ,bc_adc_channel_t start);

void bc_adc_init(bc_adc_channel_t channel, bc_adc_format_t format)
{
    if (_bc_adc.initialized != true)
    {
        // Enable ADC clock
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

        // Errata workaround
        RCC->APB2ENR;

        // Set auto-off mode, left align
        ADC1->CFGR1 |= ADC_CFGR1_AUTOFF | ADC_CFGR1_ALIGN;

        // Enable Over-sampler with ratio (16x) and set PCLK/2 as a clock source
        ADC1->CFGR2 = ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0 | ADC_CFGR2_CKMODE_0;

        // Sampling time selection (12.5 cycles)
        ADC1->SMPR |= ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0;

        // Enable ADC voltage regulator
        ADC1->CR |= ADC_CR_ADVREGEN;

        // Load Vrefint constant from ROM
        _bc_adc.vrefint = (*(uint16_t *) VREFINT_CAL_ADDR) << 4;

        NVIC_EnableIRQ(ADC1_COMP_IRQn);

        _bc_adc.initialized = true;

        _bc_adc.task_id = bc_scheduler_register(_bc_adc_task, NULL, BC_TICK_INFINITY);
    }

    bc_adc_set_format(channel, format);
}

void bc_adc_set_format(bc_adc_channel_t channel, bc_adc_format_t format)
{
    _bc_adc.channel_table[channel].format = format;
}

bc_adc_format_t bc_adc_get_format(bc_adc_channel_t channel)
{
    return _bc_adc.channel_table[channel].format;
}

bool bc_adc_is_ready(bc_adc_channel_t channel)
{
    (void) channel;

    return _bc_adc.channel_in_progress == BC_ADC_CHANNEL_NONE;
}

bool bc_adc_read(bc_adc_channel_t channel, void *result)
{
    // If ongoing conversion...
    if (_bc_adc.channel_in_progress != BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    // Set ADC channel
    ADC1->CHSELR = _bc_adc.channel_table[channel].chselr;

    // Disable all ADC interrupts
    ADC1->IER = 0;

    // Clear EOS flag (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS;

    // Start the AD measurement
    ADC1->CR |= ADC_CR_ADSTART;

    // wait for end of measurement
    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    if (result != NULL)
    {
        bc_adc_get_result(channel, result);
    }

    return true;
}

bool bc_adc_set_event_handler(bc_adc_channel_t channel, void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *), void *event_param)
{
    // Check ongoing on edited channel
    if (_bc_adc.channel_in_progress == channel)
    {
        return false;
    }

    bc_adc_channel_config_t *adc = &_bc_adc.channel_table[channel];

    adc->event_handler = event_handler;
    adc->event_param = event_param;

    return true;
}

bool bc_adc_async_read(bc_adc_channel_t channel)
{
    // If another conversion is ongoing...
    if (_bc_adc.channel_in_progress != BC_ADC_CHANNEL_NONE)
    {
        _bc_adc.channel_table[channel].pending = true;

        return true;
    }

    _bc_adc.channel_in_progress = channel;
    _bc_adc.channel_table[channel].pending = false;

    // Update internal state
    _bc_adc.state = BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN;

    // Disable interrupts
    bc_irq_disable();

    // Clear end of calibration flag
    ADC1->ISR = ADC_ISR_EOCAL;

    // Enable end of calibration interrupt
    ADC1->IER = ADC_IER_EOCALIE;

    // Begin offset calibration
    ADC1->CR |= ADC_CR_ADCAL;

    // Enable interrupts
    bc_irq_enable();

    return true;
}

bool bc_adc_get_result(bc_adc_channel_t channel, void *result)
{
    uint32_t data = ADC1->DR;

    switch (_bc_adc.channel_table[channel].format)
    {
        case BC_ADC_FORMAT_8_BIT:
        {
            *(uint8_t *) result = data >> 8;
            break;
        }
        case BC_ADC_FORMAT_16_BIT:
        {
            *(uint16_t *) result = data;
            break;
        }
        case BC_ADC_FORMAT_24_BIT:
        {
            memcpy((uint8_t *) result + 1, &data, 3);
            break;
        }
        case BC_ADC_FORMAT_32_BIT:
        {
            *(uint32_t *) result = data << 16;
            break;
        }
        case BC_ADC_FORMAT_FLOAT:
        {
            data *= _bc_adc.real_vdda_voltage / 3.3f;
            *(float *) result = data * (3.3f / 65536.f);
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool bc_adc_get_vdda_voltage(float *vdda_voltage)
{
    if (_bc_adc.real_vdda_voltage == 0.f)
    {
        return false;
    }
    else
    {
        *vdda_voltage = _bc_adc.real_vdda_voltage;

        return true;
    }
}

void ADC1_COMP_IRQHandler(void)
{
    // TODO ADC offset calibrated !!

    // Read internal reference channel
    if (_bc_adc.state == BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN)
    {
        // Enable internal reference to ADC peripheral
        ADC->CCR |= ADC_CCR_VREFEN;

        // Set ADC channel
        ADC1->CHSELR = _bc_adc.channel_table[BC_ADC_CHANNEL_INTERNAL_REFERENCE].chselr;

        // Update internal state
        _bc_adc.state = BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END;

        // Clear end of sequence interrupt
        ADC1->ISR = ADC_ISR_EOS;

        // Enable end of sequence interrupt
        ADC1->IER = ADC_IER_EOSIE;

        // Begin internal reference reading
        ADC1->CR |= ADC_CR_ADSTART;
    }

    // Get real VDDA and begin analog channel measurement
    else if (_bc_adc.state == BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END)
    {
        // Disable internal reference
        ADC->CCR &= ~ADC_CCR_VREFEN;

        // Compute actual VDDA
        _bc_adc.real_vdda_voltage = 3.f * ((float) _bc_adc.vrefint / (float) ADC1->DR);

        // Set ADC channel
        ADC1->CHSELR = _bc_adc.channel_table[_bc_adc.channel_in_progress].chselr;

        _bc_adc.state = BC_ADC_STATE_MEASURE_INPUT;

        // Clear end of sequence interrupt
        ADC1->ISR = ADC_ISR_EOS;

        // Begin internal reference measurement
        ADC1->CR |= ADC_CR_ADSTART;
    }

    // Measurement is done, plan calling callback
    else if (_bc_adc.state == BC_ADC_STATE_MEASURE_INPUT)
    {
        // Plan ADC task
        bc_scheduler_plan_now(_bc_adc.task_id);

        // Clear all interrupts
        ADC1->ISR = 0xffff;

        // Disable all ADC interrupts
        ADC1->IER = 0;
    }
}

bool bc_adc_calibration(void)
{
    if (_bc_adc.channel_in_progress != BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    // Perform ADC calibration
    ADC1->CR |= ADC_CR_ADCAL;
    while ((ADC1->ISR & ADC_ISR_EOCAL) == 0)
    {
        continue;
    }

    // Enable internal reference
    ADC->CCR |= ADC_CCR_VREFEN;

    // Set ADC channel
    ADC1->CHSELR = _bc_adc.channel_table[BC_ADC_CHANNEL_INTERNAL_REFERENCE].chselr;

    // Clear EOS flag (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS;

    // Perform measurement on internal reference
    ADC1->CR |= ADC_CR_ADSTART;

    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    // Compute actual VDDA
    _bc_adc.real_vdda_voltage = 3.f * ((float) _bc_adc.vrefint / (float) ADC1->DR);

    // Disable internal reference
    ADC->CCR &= ~ADC_CCR_VREFEN;

    return true;
}

static void _bc_adc_task(void *param)
{
    (void) param;

    bc_adc_channel_config_t *adc = &_bc_adc.channel_table[_bc_adc.channel_in_progress];
    bc_adc_channel_t pending_result_channel;
    bc_adc_channel_t next;

    // Update pending channel result
    pending_result_channel = _bc_adc.channel_in_progress;

    // Release ADC for further conversion
    _bc_adc.channel_in_progress = BC_ADC_CHANNEL_NONE;

    // Disable interrupts
    bc_irq_disable();

    // Get pending
    if (_bc_adc_get_pending(&next, pending_result_channel) == true)
    {
        bc_adc_async_read(next);
    }

    // Enable interrupts
    bc_irq_enable();

    // Perform event call-back
    if (adc->event_handler != NULL)
    {
        adc->event_handler(pending_result_channel, BC_ADC_EVENT_DONE, adc->event_param);
    }
}

static inline bool _bc_adc_get_pending(bc_adc_channel_t *next ,bc_adc_channel_t start)
{
    for (int i = start + 1; i != start; i++)
    {
        if (i == BC_ADC_CHANNEL_COUNT)
        {
            if (start == BC_ADC_CHANNEL_A0)
            {
                break;
            }
            else
            {
                i = BC_ADC_CHANNEL_A0;
            }
        }

        if (_bc_adc.channel_table[i].pending == true)
        {
            *next = i;

            return true;
        }
    }

    return false;
}
