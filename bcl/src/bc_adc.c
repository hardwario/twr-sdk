#include <stm32l083xx.h>
#include <bc_adc.h>
#include <bc_scheduler.h>
#include <bc_irq.h>

#define VREFINT_CAL_ADDR 0x1FF80078

#define BC_ADC_CHANNEL_INTERNAL_REFERENCE 6
#define BC_ADC_CHANNEL_NONE ((bc_adc_channel_t)7)

typedef enum
{
    BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN,
    BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_FINISH,
    BC_ADC_STATE_MEASURE_INPUT

} bc_adc_state_t;

typedef struct
{
    bc_adc_format_t format;
    void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *);
    void *event_param;
    uint32_t chselr;

} bc_adc_channel_config_t;

static struct
{
    bool initialized;
    bc_adc_channel_t channel_in_progress;
    uint16_t vrefint;
    float real_vdda;
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
        [BC_ADC_CHANNEL_INTERNAL_REFERENCE] = {BC_ADC_FORMAT_16_BIT, NULL, NULL, ADC_CHSELR_CHSEL17 }
    }
};

static inline void _bc_adc_calibration(void);

static void _bc_adc_task(void *param);

void bc_adc_init(bc_adc_channel_t channel, bc_adc_format_t format)
{
    if (_bc_adc.initialized != true)
    {
        // Enable ADC clock
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

        // Errata workaround
        RCC->APB2ENR;

        // Set auto-off mode, left align (TODO Deeper investigation of the align)
        ADC1->CFGR1 |= ADC_CFGR1_AUTOFF | ADC_CFGR1_ALIGN;

        // Enable Over-sampler with ratio (256x) and set PCLK/2 as a clock source
        ADC1->CFGR2 = ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0 | ADC_CFGR2_OVSS_2 | ADC_CFGR2_CKMODE_0;

        // Sampling time selection (160.5 cycles)
        ADC1->SMPR |= ADC_SMPR_SMP_2 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0;

        // Enable ADC voltage regulator
        ADC1->CR |= ADC_CR_ADVREGEN;

        // Load Vrefint constant from ROM
        _bc_adc.vrefint = (*(uint16_t *) VREFINT_CAL_ADDR) << 4;

        NVIC_EnableIRQ(ADC1_COMP_IRQn);

        _bc_adc.initialized = true;
    }

    bc_adc_set_format(channel, format);

    _bc_adc.task_id = bc_scheduler_register(_bc_adc_task, NULL, BC_TICK_INFINITY);
}

void bc_adc_set_format(bc_adc_channel_t channel, bc_adc_format_t format)
{
    _bc_adc.channel_table[channel].format = format;
}

bc_adc_format_t bc_adc_get_format(bc_adc_channel_t channel)
{
    return _bc_adc.channel_table[channel].format;
}

bool bc_adc_read(bc_adc_channel_t channel, void *result)
{
    // If ongoing conversion...
    if (_bc_adc.channel_in_progress != BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    _bc_adc_calibration();

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

    bc_adc_get_result(channel, result);

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
    // If ongoing conversion ...
    if (_bc_adc.channel_in_progress != BC_ADC_CHANNEL_NONE)
    {
        return false;
    }

    _bc_adc.channel_in_progress = channel;

    // Update internal state
    _bc_adc.state = BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN;

    bc_irq_disable();

    // Clear end of calibration flag
    ADC1->ISR = ADC_ISR_EOCAL;

    // Enable end of calibration interrupt
    ADC1->IER = ADC_IER_EOCALIE;

    bc_irq_enable();

    // Begin offset calibration
    ADC1->CR |= ADC_CR_ADCAL;

    return true;
}

void bc_adc_get_result(bc_adc_channel_t channel, void *result)
{
    uint32_t data = ADC1->DR;

    data *= _bc_adc.real_vdda / 3.3f;

    switch (_bc_adc.channel_table[channel].format)
    {
    case BC_ADC_FORMAT_8_BIT:
        *(uint8_t *) result = data >> 8;
        break;
    case BC_ADC_FORMAT_16_BIT:
        *(uint16_t *) result = data;
        break;
    case BC_ADC_FORMAT_24_BIT:
        memcpy((uint8_t *) result + 1, &data, 3);
        break;
    case BC_ADC_FORMAT_32_BIT:
        *(uint32_t *) result = data << 16;
        break;
    case BC_ADC_FORMAT_FLOAT:
        *(float *) result = data * (3.3f / 65536.f);
        break;
    default:
        return;
        break;
    }
}

bool bc_adc_get_vdda(float *vdda)
{
    if(_bc_adc.real_vdda ==  0.0f)
    {
        return false;
    }
    else
    {
        *vdda = _bc_adc.real_vdda;

        return true;
    }
}

void ADC1_COMP_IRQHandler(void)
{
    // ADC offset calibrated !!

    // Read internal reference channel
    if (_bc_adc.state == BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN)
    {
        // Enable internal reference to ADC peripheral
        ADC->CCR |= ADC_CCR_VREFEN;

        // Set ADC channel
        ADC1->CHSELR = _bc_adc.channel_table[BC_ADC_CHANNEL_INTERNAL_REFERENCE].chselr;

        // Update internal state
        _bc_adc.state = BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_FINISH;

        // Clear end of sequence interrupt
        ADC1->ISR = ADC_ISR_EOS;

        // Enable end of sequence interrupt
        ADC1->IER = ADC_IER_EOSIE;

        // Begin internal reference reading
        ADC1->CR |= ADC_CR_ADSTART;
    }

    // Get real VDDA and begin analog channel measurement
    else if (_bc_adc.state == BC_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_FINISH)
    {
        // Disable internal reference
        ADC->CCR &= ~ADC_CCR_VREFEN;

        // Compute actual VDDA
        _bc_adc.real_vdda = 3. * ((float) _bc_adc.vrefint / (float) ADC1->DR);

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

        // Disable all ADC interrupts
        ADC1->IER = 0;
    }
}

static inline void _bc_adc_calibration(void)
{
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
    _bc_adc.real_vdda = 3. * ((float) _bc_adc.vrefint / (float) ADC1->DR);

    // Disable internal reference
    ADC->CCR &= ~ADC_CCR_VREFEN;
}

static void _bc_adc_task(void *param)
{
    (void) param;

    bc_adc_channel_config_t *adc = &_bc_adc.channel_table[_bc_adc.channel_in_progress];
    bc_adc_channel_t pending_result_channel;

    // Update pending channel result
    pending_result_channel = _bc_adc.channel_in_progress;

    // Release ADC for further conversion
    _bc_adc.channel_in_progress = BC_ADC_CHANNEL_NONE;

    // Perform event call-back
    adc->event_handler(pending_result_channel, BC_ADC_EVENT_DONE, adc->event_param);
}
