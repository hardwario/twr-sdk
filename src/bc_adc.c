#include <bc_common.h>
#include <bc_adc.h>
#include <stm32l083xx.h>

typedef struct
{
    bc_adc_reference_t reference;
    bc_adc_format_t format;
    uint32_t chselr;
} bc_adc_config_t;

static bc_adc_config_t _bc_adc_config_table[6] =
{
    [0].chselr = ADC_CHSELR_CHSEL0,
    [1].chselr = ADC_CHSELR_CHSEL1,
    [2].chselr = ADC_CHSELR_CHSEL2,
    [3].chselr = ADC_CHSELR_CHSEL3,
    [4].chselr = ADC_CHSELR_CHSEL4,
    [5].chselr = ADC_CHSELR_CHSEL5
};

void bc_adc_init(bc_adc_channel_t channel, bc_adc_reference_t reference, bc_adc_format_t format)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; // Enable ADC peripheral clock

    // ADC calibration
    ADC1->CR &= (uint32_t) (~ADC_CR_ADEN);  // Clear ADEN
    ADC1->CR |= ADC_CR_ADCAL;               // Start calibration
    while ((ADC1->ISR & ADC_ISR_EOCAL) == 0)
    {
        continue;                           // Wait until calibration is done
    }
    ADC1->ISR |= ADC_ISR_EOCAL;             // Clear EOCAL

    // Enable and configure ADC
    ADC1->ISR |= ADC_ISR_ADRDY;                         // Clear the ADRDY bit
    ADC1->CR |= ADC_CR_ADEN;                            // Enable the ADC
    ADC1->CFGR1 |= ADC_CFGR1_AUTOFF |                   // Select the auto off mode
                   ADC_CFGR1_ALIGN;                     // Left alignment
    ADC1->CFGR2 = (ADC1->CFGR2 & (~ADC_CFGR2_CKMODE)) | // Asynchronous clock mode
    (ADC_CFGR2_OVSE |                                   // Over-sampler Enable
    ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0);               // Over-sampling ratio (16x)
    ADC1->SMPR |= ADC_SMPR_SMP_0 | ADC_SMPR_SMP_1;      // Sampling time selection (12.5 cycles)

    // Store desired reference
    bc_adc_set_reference(channel, reference);

    // Store desired format
    bc_adc_set_format(channel, format);
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
    uint32_t data;

    // Set ADC channel
    ADC1->CHSELR = _bc_adc_config_table[channel].chselr; // Select CHSEL17 for VRefInt

    // Clear EOC, EOS and OVR flags
    ADC1->ISR = (ADC_ISR_EOS | ADC_ISR_OVR);

    // Performs the AD conversion
    ADC1->CR |= ADC_CR_ADSTART;

    // wait for end of sequence
    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    data = ADC1->DR;

    // TODO ... take care of reference ...

    switch (_bc_adc_config_table[channel].format)
    {
    case BC_ADC_FORMAT_8_BIT:
        *(uint8_t *)result = data >> 8;
        break;
    case BC_ADC_FORMAT_16_BIT:
        *(uint16_t *)result = data;
        break;
    case BC_ADC_FORMAT_24_BIT:
        memcpy((uint8_t *)result + 1, &data, 2);
        break;
    case BC_ADC_FORMAT_32_BIT:
        *(uint32_t *)result = data << 16;
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
