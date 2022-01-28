#include <twr_adc.h>
#include <twr_scheduler.h>
#include <twr_irq.h>
#include <stm32l083xx.h>
#include <twr_sleep.h>

#include <twr_system.h>

#define VREFINT_CAL_ADDR 0x1ff80078

#define TWR_ADC_CHANNEL_INTERNAL_REFERENCE 7
#define TWR_ADC_CHANNEL_NONE ((twr_adc_channel_t) (-1))
#define TWR_ADC_CHANNEL_COUNT ((twr_adc_channel_t) 8)

typedef enum
{
    TWR_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN,
    TWR_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END,
    TWR_ADC_STATE_MEASURE_INPUT

} twr_adc_state_t;

typedef struct
{
    void (*event_handler)(twr_adc_channel_t, twr_adc_event_t, void *);
    void *event_param;
    bool pending;
    twr_adc_resolution_t resolution;
    twr_adc_oversampling_t oversampling;
    uint16_t value;
    uint32_t chselr;

} twr_adc_channel_config_t;

static struct
{
    bool initialized;
    twr_adc_channel_t channel_in_progress;
    uint16_t vrefint;
    float real_vdda_voltage;
    twr_adc_state_t state;
    twr_scheduler_task_id_t task_id;
    twr_adc_channel_config_t channel_table[8];
}
_twr_adc =
{
    .initialized = false,
    .channel_in_progress = TWR_ADC_CHANNEL_NONE,
    .channel_table =
    {
        [TWR_ADC_CHANNEL_A0].chselr = ADC_CHSELR_CHSEL0,
        [TWR_ADC_CHANNEL_A1].chselr = ADC_CHSELR_CHSEL1,
        [TWR_ADC_CHANNEL_A2].chselr = ADC_CHSELR_CHSEL2,
        [TWR_ADC_CHANNEL_A3].chselr = ADC_CHSELR_CHSEL3,
        [TWR_ADC_CHANNEL_A4].chselr = ADC_CHSELR_CHSEL4,
        [TWR_ADC_CHANNEL_A5].chselr = ADC_CHSELR_CHSEL5,
        [TWR_ADC_CHANNEL_A6].chselr = ADC_CHSELR_CHSEL6,
        [TWR_ADC_CHANNEL_INTERNAL_REFERENCE] = { NULL, NULL, false, TWR_ADC_RESOLUTION_12_BIT, TWR_ADC_OVERSAMPLING_256, 0, ADC_CHSELR_CHSEL17 }
    }
};

static void _twr_adc_task(void *param);

static inline bool _twr_adc_get_pending(twr_adc_channel_t *next ,twr_adc_channel_t start);

void twr_adc_init()
{
    if (_twr_adc.initialized != true)
    {
        // Enable ADC clock
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

        // Errata workaround
        RCC->APB2ENR;

        // Set auto-off mode, left align
        ADC1->CFGR1 |= ADC_CFGR1_AUTOFF;

        // Set PCLK as a clock source
        ADC1->CFGR2 = ADC_CFGR2_CKMODE_0 | ADC_CFGR2_CKMODE_1;

        // Sampling time selection (12.5 cycles)
        ADC1->SMPR |= ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0;

        // Enable ADC voltage regulator
        ADC1->CR |= ADC_CR_ADVREGEN;

        // Load Vrefint constant from ROM
        _twr_adc.vrefint = (*(uint16_t *) VREFINT_CAL_ADDR);// << 4;

        NVIC_EnableIRQ(ADC1_COMP_IRQn);

        _twr_adc.initialized = true;

        _twr_adc.task_id = twr_scheduler_register(_twr_adc_task, NULL, TWR_TICK_INFINITY);

        twr_adc_calibration();
    }
}

void twr_adc_oversampling_set(twr_adc_channel_t channel, twr_adc_oversampling_t oversampling)
{
    _twr_adc.channel_table[channel].oversampling = oversampling;
}

void twr_adc_resolution_set(twr_adc_channel_t channel, twr_adc_resolution_t resolution)
{
    _twr_adc.channel_table[channel].resolution = resolution;
}

static void _twr_adc_configure_resolution(twr_adc_resolution_t resolution)
{
    ADC1->CFGR1 &= ~ADC_CFGR1_RES_Msk;
    ADC1->CFGR1 |= resolution & ADC_CFGR1_RES_Msk;
}

static void _twr_adc_configure_oversampling(twr_adc_oversampling_t oversampling)
{
    // Clear oversampling enable, oversampling register and oversampling shift register
    ADC1->CFGR2 &= ~(ADC_CFGR2_OVSE_Msk | ADC_CFGR2_OVSR_Msk | ADC_CFGR2_OVSS_Msk);

    static const uint16_t oversampling_register_lut[9] =
    {
        0, // no oversampling
        ADC_CFGR2_OVSE                                                              | ADC_CFGR2_OVSS_0, // 2x
        ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_0                                           | ADC_CFGR2_OVSS_1, // 4x
        ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_1                                           | ADC_CFGR2_OVSS_1 | ADC_CFGR2_OVSS_0, // 8x
        ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0                        | ADC_CFGR2_OVSS_2, // 16x
        ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_2                                           | ADC_CFGR2_OVSS_2 | ADC_CFGR2_OVSS_0, // 32x
        ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_0                        | ADC_CFGR2_OVSS_2 | ADC_CFGR2_OVSS_1, // 64x
        ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1                        | ADC_CFGR2_OVSS_2 | ADC_CFGR2_OVSS_1 | ADC_CFGR2_OVSS_0, // 128x
        ADC_CFGR2_OVSE | ADC_CFGR2_OVSR_2 | ADC_CFGR2_OVSR_1 | ADC_CFGR2_OVSR_0     | ADC_CFGR2_OVSS_3, // 256x
    };

    ADC1->CFGR2 |= oversampling_register_lut[oversampling];
}

static uint16_t _twr_adc_get_measured_value(twr_adc_channel_t channel)
{
    uint16_t value = ADC1->DR;

    switch (_twr_adc.channel_table[channel].resolution)
    {
        case TWR_ADC_RESOLUTION_6_BIT:
        {
            value <<= 10;
            break;
        }
        case TWR_ADC_RESOLUTION_8_BIT:
        {
            value <<= 8;
            break;
        }
        case TWR_ADC_RESOLUTION_10_BIT:
        {
            value <<= 6;
            break;
        }
        case TWR_ADC_RESOLUTION_12_BIT:
        {
            value <<= 4;
            break;
        }
        default:
        {
            break;
        }
    }

    return value;
}

bool twr_adc_is_ready()
{
    return _twr_adc.channel_in_progress == TWR_ADC_CHANNEL_NONE;
}

bool twr_adc_get_value(twr_adc_channel_t channel, uint16_t *result)
{
    // If ongoing conversion...
    if (_twr_adc.channel_in_progress != TWR_ADC_CHANNEL_NONE)
    {
        return false;
    }

    // Set ADC channel
    ADC1->CHSELR = _twr_adc.channel_table[channel].chselr;

    // Disable all ADC interrupts
    ADC1->IER = 0;

    // Clear EOS flag (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS;

    // Clear ADRDY (it is cleared by software writing 1 to it)
    ADC1->ISR |= ADC_ISR_ADRDY;

    _twr_adc_configure_oversampling(_twr_adc.channel_table[channel].oversampling);
    _twr_adc_configure_resolution(_twr_adc.channel_table[channel].resolution);

    // Start the AD measurement
    ADC1->CR |= ADC_CR_ADSTART;

    // wait for end of measurement
    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    if (result != NULL)
    {
        *result = _twr_adc_get_measured_value(channel);
    }

    return true;
}

bool twr_adc_set_event_handler(twr_adc_channel_t channel, void (*event_handler)(twr_adc_channel_t, twr_adc_event_t, void *), void *event_param)
{
    // Check ongoing on edited channel
    if (_twr_adc.channel_in_progress == channel)
    {
        return false;
    }

    twr_adc_channel_config_t *adc = &_twr_adc.channel_table[channel];

    adc->event_handler = event_handler;
    adc->event_param = event_param;

    return true;
}

bool twr_adc_async_measure(twr_adc_channel_t channel)
{
    // If another conversion is ongoing...
    if (_twr_adc.channel_in_progress != TWR_ADC_CHANNEL_NONE)
    {
        _twr_adc.channel_table[channel].pending = true;

        return true;
    }

    _twr_adc.channel_in_progress = channel;
    _twr_adc.channel_table[channel].pending = false;

    // Skip cal and measure VREF + channel
    //------------------------------------
    _twr_adc.state = TWR_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END;

    // Enable internal reference to ADC peripheral
    ADC->CCR |= ADC_CCR_VREFEN;

    _twr_adc_configure_oversampling(_twr_adc.channel_table[TWR_ADC_CHANNEL_INTERNAL_REFERENCE].oversampling);
    _twr_adc_configure_resolution(_twr_adc.channel_table[TWR_ADC_CHANNEL_INTERNAL_REFERENCE].resolution);

    // Set ADC channel
    ADC1->CHSELR = _twr_adc.channel_table[TWR_ADC_CHANNEL_INTERNAL_REFERENCE].chselr;

    // Clear end of calibration interrupt
    ADC1->ISR = ADC_ISR_EOCAL;

    // Enable end of conversion interrupt
    ADC1->IER = ADC_IER_EOCIE;

    // Begin internal reference reading
    ADC1->CR |= ADC_CR_ADSTART;

    twr_sleep_disable(); // enable in _twr_adc_task

    return true;
}

bool twr_adc_async_get_value(twr_adc_channel_t channel, uint16_t *result)
{
    *result = _twr_adc.channel_table[channel].value;
    return true;
}

bool twr_adc_async_get_voltage(twr_adc_channel_t channel, float *result)
{
    *result = (_twr_adc.channel_table[channel].value * _twr_adc.real_vdda_voltage) / 65536.f;
    return true;
}

bool twr_adc_get_vdda_voltage(float *vdda_voltage)
{
    if (_twr_adc.real_vdda_voltage == 0.f)
    {
        return false;
    }
    else
    {
        *vdda_voltage = _twr_adc.real_vdda_voltage;

        return true;
    }
}

void ADC1_COMP_IRQHandler(void)
{
    // Get real VDDA and begin analog channel measurement
    if (_twr_adc.state == TWR_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END)
    {
        // Compute actual VDDA
        _twr_adc.real_vdda_voltage = 3.f * ((float) _twr_adc.vrefint / (float) ADC1->DR);

        _twr_adc_configure_oversampling(_twr_adc.channel_table[_twr_adc.channel_in_progress].oversampling);
        _twr_adc_configure_resolution(_twr_adc.channel_table[_twr_adc.channel_in_progress].resolution);

        // Set ADC channel
        ADC1->CHSELR = _twr_adc.channel_table[_twr_adc.channel_in_progress].chselr;

        _twr_adc.state = TWR_ADC_STATE_MEASURE_INPUT;

        // Clear end of conversion interrupt
        ADC1->ISR = ADC_ISR_EOC;

        // Begin adc input channel measurement
        ADC1->CR |= ADC_CR_ADSTART;
    }

    // Measurement is done, plan calling callback
    else if (_twr_adc.state == TWR_ADC_STATE_MEASURE_INPUT)
    {
        // Disable internal reference
        ADC->CCR &= ~ADC_CCR_VREFEN;

        _twr_adc.channel_table[_twr_adc.channel_in_progress].value = _twr_adc_get_measured_value(_twr_adc.channel_in_progress);

        // Plan ADC task
        twr_scheduler_plan_now(_twr_adc.task_id);

        // Clear all interrupts
        ADC1->ISR = 0xffff;

        // Disable all ADC interrupts
        ADC1->IER = 0;

    }
}

bool twr_adc_calibration(void)
{
    if (_twr_adc.channel_in_progress != TWR_ADC_CHANNEL_NONE)
    {
        return false;
    }

    if (ADC1->CR & ADC_CR_ADEN)
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
    ADC1->CHSELR = _twr_adc.channel_table[TWR_ADC_CHANNEL_INTERNAL_REFERENCE].chselr;

    // Clear EOS flag (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS;

    // Perform measurement on internal reference
    ADC1->CR |= ADC_CR_ADSTART;

    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    // Compute actual VDDA
    _twr_adc.real_vdda_voltage = 3.f * ((float) _twr_adc.vrefint / (float) ADC1->DR);

    // Disable internal reference
    ADC->CCR &= ~ADC_CCR_VREFEN;

    return true;
}

static void _twr_adc_task(void *param)
{
    (void) param;

    twr_sleep_enable();

    twr_adc_channel_config_t *adc = &_twr_adc.channel_table[_twr_adc.channel_in_progress];
    twr_adc_channel_t pending_result_channel;
    twr_adc_channel_t next;

    // Update pending channel result
    pending_result_channel = _twr_adc.channel_in_progress;

    // Release ADC for further conversion
    _twr_adc.channel_in_progress = TWR_ADC_CHANNEL_NONE;

    // Disable interrupts
    twr_irq_disable();

    // Get pending
    if (_twr_adc_get_pending(&next, pending_result_channel) == true)
    {
        twr_adc_async_measure(next);
    }

    // Enable interrupts
    twr_irq_enable();

    // Perform event call-back
    if (adc->event_handler != NULL)
    {
        adc->event_handler(pending_result_channel, TWR_ADC_EVENT_DONE, adc->event_param);
    }
}

static inline bool _twr_adc_get_pending(twr_adc_channel_t *next ,twr_adc_channel_t start)
{
    for (int i = start + 1; i != start; i++)
    {
        if (i == TWR_ADC_CHANNEL_COUNT)
        {
            if (start == TWR_ADC_CHANNEL_A0)
            {
                break;
            }
            else
            {
                i = TWR_ADC_CHANNEL_A0;
            }
        }

        if (_twr_adc.channel_table[i].pending == true)
        {
            *next = i;

            return true;
        }
    }

    return false;
}
