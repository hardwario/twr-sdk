#include "bc_dac.h"
#include "stm32l0xx.h"

#define BC_DAC_PERIPHERAL_CLOCK_ENABLE() (RCC->APB1ENR |= RCC_APB1ENR_DACEN)
#define BC_DAC_PERIPHERAL_CLOCK_DISABLE() (RCC->APB1ENR &= ~RCC_APB1ENR_DACEN)
#define BC_DAC_GPIO_CLOCK_ENABLE(_PIN_) (RCC->IOPENR |= BC_PINS_GET_RCC_IOPENR_MASK(_PIN_))
#define BC_DAC_CHANNEL0_SET(_CODE_) (DAC1->DHR12L1 = _CODE_)
#define BC_DAC_CHANNEL1_SET(_CODE_) (DAC1->DHR12L2 = _CODE_)

#define BC_DAC_VOLTAGE_TO_CODE_CONSTANT 0.0000502626747

// TODO ... velikosti channel cnt
bc_dac_format_t bc_dac_setup[2];

void bc_dac_init(bc_dac_channel_t channel, bc_dac_format_t format)
{
    /* store channel input format */
    bc_dac_setup[channel] = format;

    /* Enable peripheral code */
    BC_DAC_PERIPHERAL_CLOCK_ENABLE();

    /* Software trigger, disable output buffer, enable DACs */
    DAC1->CR = DAC_CR_BOFF1 | DAC_CR_BOFF2 | DAC_CR_TSEL1 | DAC_CR_TSEL2
            | DAC_CR_EN1 | DAC_CR_EN2;
}

void bc_dac_set_format(bc_dac_channel_t channel, bc_dac_format_t format)
{
    /* store channel input format */
    bc_dac_setup[channel] = format;
}

bc_dac_format_t bc_dac_get_format(bc_dac_channel_t channel)
{
    return bc_dac_setup[channel];
}

// TODO .. pro4 se p5ed8v8 pointer<, hodnota bude rzchej39 o se t74e stacku i pr8ce s hodnotou
void bc_dac_set_output(bc_dac_channel_t channel, const void *output)
{
    uint16_t raw;

    // TODO ... jednoduch7 p5evod, mo6n8 bude lep39 pou69t float a switch je na nic...
    switch (bc_dac_setup[channel])
    {
    case BC_DAC_FORMAT_8_BIT:
        raw = (*(uint16_t *) output << 8);
        break;
    case BC_DAC_FORMAT_16_BIT:
        raw = *(uint16_t *) output;
        break;
    case BC_DAC_FORMAT_24_BIT:
        raw = ((*(uint32_t *) output >> 8) & 0xffff);
        break;
    case BC_DAC_FORMAT_32_BIT:
        raw = ((*(uint32_t *) output >> 16) & 0xffff);
        break;
    default:
        // TODO ... tadz bz to asi m2lo vr8ti chzbu
        break;
    }

    switch (channel)
    {
    case BC_DAC_DAC0:
        BC_DAC_CHANNEL0_SET(raw);
        break;
    case BC_DAC_DAC1:
        BC_DAC_CHANNEL1_SET(raw);
        break;
    default:
        // TODO ... to sam7 tadz
        break;
    }
}
