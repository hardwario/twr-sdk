#include <bc_common.h>
#include <bc_adc.h>
#include <stm32l083xx.h>

typedef struct {
    bc_adc_reference_t reference;
    bc_adc_format_t format;
} bc_adc_config_t;

static bc_adc_config_t _bc_adc_config_table[6];

void bc_adc_init(bc_adc_channel_t channel, bc_adc_reference_t reference, bc_adc_format_t format)
{
    ADC1->ISR |= ADC_ISR_ADRDY; // Clear the ADRDY bit
    ADC1->CR |= ADC_CR_ADEN; // Enable the ADC
    if ((ADC1->CFGR1 & ADC_CFGR1_AUTOFF) == 0)
    {
        // Wait until ADC ready
        while ((ADC1->ISR & ADC_ISR_ADRDY) == 0)
        {
            /* For robust implementation, add here time-out management */
        }
    }

    ADC1->CFGR2 = (ADC1->CFGR2 & (~ADC_CFGR2_CKMODE)) | // Asynchronous clock mode)
            (ADC_CFGR2_OVSE |   // Oversampler Enable
             ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0); // Oversampling ratio

    // TODO ... initialize gpio
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

void bc_adc_measure(bc_adc_channel_t channel, void *result)
{
    ADC1->CFGR1 |= ADC_CFGR1_AUTOFF; // Select the auto off mode
    ADC1->CHSELR = ADC_CHSELR_CHSEL0; // Select CHSEL17 for VRefInt
    ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_2; // Select a sampling mode of 111 i.e. 239.5 ADC clk to be greater than 17.1us
    // ADC->CCR |= ADC_CCR_VREFEN; // Wake-up the VREFINT (only for Temp sensor and VRefInt)

    /* Performs the AD conversion */
    ADC1->CR |= ADC_CR_ADSTART; /* start the ADC conversion */
    while ((ADC1->ISR & ADC_ISR_EOC) == 0) /* wait end of conversion */
    {
    /* For robust implementation, add here time-out management */
    }

    // TODO ... set reference ...

    // TODO ... set format ...
}
