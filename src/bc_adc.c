#include <bc_common.h>
#include <bc_adc.h>
#include <stm32l083xx.h>

typedef struct {
    bc_adc_reference_t reference;
    bc_adc_format_t format;
    uint32_t chsel;
} bc_adc_config_t;

static bc_adc_config_t _bc_adc_config_table[6] = {
    [0].chsel = ADC_CHSELR_CHSEL0,
    [1].chsel = ADC_CHSELR_CHSEL1,
    [2].chsel = ADC_CHSELR_CHSEL2,
    [3].chsel = ADC_CHSELR_CHSEL3,
    [4].chsel = ADC_CHSELR_CHSEL4,
    [5].chsel = ADC_CHSELR_CHSEL5
};

void bc_adc_init(bc_adc_channel_t channel, bc_adc_reference_t reference, bc_adc_format_t format)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Enable ADC peripheral clock

    // Clear the ADRDY bit
    ADC1->ISR |= ADC_ISR_ADRDY;

    ADC1->CR &= (uint32_t) (~ADC_CR_ADEN); // Clear ADEN
    ADC1->CR |= ADC_CR_ADCAL; // Start calibration

    // Wait untill calibration is done
    while ((ADC1->ISR & ADC_ISR_EOCAL) == 0) /* (4) */
    {
        /* For robust implementation, add here time-out management */
    }

    // Clear EOCAL
    ADC1->ISR |= ADC_ISR_EOCAL;

    // Enable the ADC
    ADC1->CR |= ADC_CR_ADEN;

    // Select the auto off mode
    ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;

    // Wait until ADC ready
    if ((ADC1->CFGR1 & ADC_CFGR1_AUTOFF) == 0)
    {
        while ((ADC1->ISR & ADC_ISR_ADRDY) == 0)
        {
            /* For robust implementation, add here time-out management */
        }
    }

    ADC1->CFGR2 = (ADC1->CFGR2 & (~ADC_CFGR2_CKMODE)) | // Asynchronous clock mode
                  (ADC_CFGR2_OVSE |   // Oversampler Enable
                   ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0); // Oversampling ratio (16x)

    // Store desired reference
    bc_adc_set_reference(channel, reference);

    // Store desired format
    bc_adc_set_format(channel, format);

    // TODO ... initialize gpio
}

void bc_adc_set_reference(bc_adc_channel_t channel, bc_adc_reference_t reference)
{
    // Store desired reference
    _bc_adc_config_table[channel].reference = reference;
}

bc_adc_reference_t bc_adc_get_reference(bc_adc_channel_t channel)
{
    // Return reference
    return _bc_adc_config_table[channel].reference;
}

void bc_adc_set_format(bc_adc_channel_t channel, bc_adc_format_t format)
{
    // Store desired format
    _bc_adc_config_table[channel].format = format;
}

bc_adc_format_t bc_adc_get_format(bc_adc_channel_t channel)
{
    // Return format
    return _bc_adc_config_table[channel].format;
}

void bc_adc_measure(bc_adc_channel_t channel, void *result)
{
    // Set adc channel
    ADC1->CHSELR = _bc_adc_config_table[channel].chsel; // Select CHSEL17 for VRefInt
    
    ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2; // Select a sampling mode
    // ADC->CCR |= ADC_CCR_VREFEN; // Wake-up the VREFINT (only for Temp sensor and VRefInt)

    // Clear EOC, EOS and OVR flags
    ADC1->ISR = (ADC_ISR_EOS | ADC_ISR_OVR);

    // Performs the AD conversion
    ADC1->CR |= ADC_CR_ADSTART;

    // wait end of sequence
    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
    /* For robust implementation, add here time-out management */
    }

    // TODO ... take care of reference ...

    // TODO ... take care of format ...

    // readout data in format
}
