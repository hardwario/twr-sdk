#include "bc_dac.h"
#include "stm32l0xx.h"

// Approximate voltage to code constant
#define BC_DAC_VOLTAGE_TO_CODE_CONSTANT 19961;

static bc_dac_format_t _bc_dac_setup[2];

void bc_dac_init(bc_dac_channel_t channel, bc_dac_format_t format)
{
    // Store channel input format
    _bc_dac_setup[channel] = format;

    // Enable peripheral code
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;

    // Software trigger, disable output buffer, enable DACs
    DAC1->CR = DAC_CR_BOFF1 | DAC_CR_BOFF2 | DAC_CR_TSEL1 | DAC_CR_TSEL2 | DAC_CR_EN1 | DAC_CR_EN2;
}

void bc_dac_set_format(bc_dac_channel_t channel, bc_dac_format_t format)
{
    // Store channel input format
    _bc_dac_setup[channel] = format;
}

bc_dac_format_t bc_dac_get_format(bc_dac_channel_t channel)
{
    return _bc_dac_setup[channel];
}

void bc_dac_set_output_raw(bc_dac_channel_t channel, const void *output)
{
    uint16_t raw;

    // Handle input format
    switch (_bc_dac_setup[channel])
    {
        case BC_DAC_FORMAT_8_BIT:
        {
            raw = *(uint16_t *) output << 8;
            break;
        }
        case BC_DAC_FORMAT_16_BIT:
        {
            raw = *(uint16_t *) output;
            break;
        }
        case BC_DAC_FORMAT_24_BIT:
        {
            raw = *(uint32_t *) output >> 8;
            break;
        }
        case BC_DAC_FORMAT_32_BIT:
        {
            raw = *(uint32_t *) output >> 16;
            break;
        }
        default:
        {
            return;
        }
    }

    // Write raw on channel
    switch (channel)
    {
        case BC_DAC_CHANNEL_DAC0:
        {
            DAC1->DHR12L1 = raw;
            break;
        }
        case BC_DAC_CHANNEL_DAC1:
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

void bc_dac_set_output_voltage(bc_dac_channel_t channel, float voltage)
{
    // Convert voltage to DAC code
    uint16_t raw = voltage * BC_DAC_VOLTAGE_TO_CODE_CONSTANT;

    // Write raw on channel
    switch (channel)
    {
        case BC_DAC_CHANNEL_DAC0:
        {
            DAC1->DHR12L1 = raw;
            break;
        }
        case BC_DAC_CHANNEL_DAC1:
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
