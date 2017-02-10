#include <bc_dac.h>
#include <stm32l0xx.h>

// Approximate voltage to code constant
#define _BC_DAC_VOLTAGE_TO_CODE_CONSTANT 19961

static bc_dac_format_t _bc_dac_format[2];

void bc_dac_init(bc_dac_channel_t channel, bc_dac_format_t format)
{
    // Store input format of raw value
    _bc_dac_format[channel] = format;

    // Enable DAC clock
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;

    // Software trigger, disable output buffer, enable DACs
    DAC1->CR |= DAC_CR_TSEL2_2 | DAC_CR_TSEL2_1 | DAC_CR_TSEL2_0 | DAC_CR_BOFF2 | DAC_CR_EN2;
    DAC1->CR |= DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0 | DAC_CR_BOFF1 | DAC_CR_EN1;
}

void bc_dac_set_format(bc_dac_channel_t channel, bc_dac_format_t format)
{
    // Store channel input format
    _bc_dac_format[channel] = format;
}

bc_dac_format_t bc_dac_get_format(bc_dac_channel_t channel)
{
    return _bc_dac_format[channel];
}

void bc_dac_set_output_raw(bc_dac_channel_t channel, const void *raw)
{
    uint16_t dac_value;

    // Convert input value in configured format to raw value
    switch (_bc_dac_format[channel])
    {
        case BC_DAC_FORMAT_8_BIT:
        {
            dac_value = *(uint16_t *) raw << 8;
            break;
        }
        case BC_DAC_FORMAT_16_BIT:
        {
            dac_value = *(uint16_t *) raw;
            break;
        }
        case BC_DAC_FORMAT_24_BIT:
        {
            dac_value = *(uint32_t *) raw >> 8;
            break;
        }
        case BC_DAC_FORMAT_32_BIT:
        {
            dac_value = *(uint32_t *) raw >> 16;
            break;
        }
        default:
        {
            return;
        }
    }

    // Write raw value to DAC channel
    switch (channel)
    {
        case BC_DAC_DAC0:
        {
            DAC1->DHR12L1 = dac_value;
            break;
        }
        case BC_DAC_DAC1:
        {
            DAC1->DHR12L2 = dac_value;
            break;
        }
        default:
        {
            break;
        }
    }
}

void bc_dac_set_output_voltage(bc_dac_channel_t channel, float voltage)
{
    // Convert desired voltage to raw DAC code
    uint16_t raw = voltage * _BC_DAC_VOLTAGE_TO_CODE_CONSTANT;

    // Write raw value to DAC channel
    switch (channel)
    {
        case BC_DAC_DAC0:
        {
            DAC1->DHR12L1 = raw;
            break;
        }
        case BC_DAC_DAC1:
        {
            DAC1->DHR12L2 = raw;
            break;
        }
        default:
        {
            break;
        }
    }
}
