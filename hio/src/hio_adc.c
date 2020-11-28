#include <hio_adc.h>
#include <hio_scheduler.h>
#include <hio_irq.h>
#include <stm32l083xx.h>

#include <hio_system.h>

#define VREFINT_CAL_ADDR 0x1ff80078

#define HIO_ADC_CHANNEL_INTERNAL_REFERENCE 7
#define HIO_ADC_CHANNEL_NONE ((hio_adc_channel_t) (-1))
#define HIO_ADC_CHANNEL_COUNT ((hio_adc_channel_t) 8)

typedef enum
{
    HIO_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_BEGIN,
    HIO_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END,
    HIO_ADC_STATE_MEASURE_INPUT

} hio_adc_state_t;

typedef struct
{
    void (*event_handler)(hio_adc_channel_t, hio_adc_event_t, void *);
    void *event_param;
    bool pending;
    hio_adc_resolution_t resolution;
    hio_adc_oversampling_t oversampling;
    uint16_t value;
    uint32_t chselr;

} hio_adc_channel_config_t;

static struct
{
    bool initialized;
    hio_adc_channel_t channel_in_progress;
    uint16_t vrefint;
    float real_vdda_voltage;
    hio_adc_state_t state;
    hio_scheduler_task_id_t task_id;
    hio_adc_channel_config_t channel_table[8];
}
_hio_adc =
{
    .initialized = false,
    .channel_in_progress = HIO_ADC_CHANNEL_NONE,
    .channel_table =
    {
        [HIO_ADC_CHANNEL_A0].chselr = ADC_CHSELR_CHSEL0,
        [HIO_ADC_CHANNEL_A1].chselr = ADC_CHSELR_CHSEL1,
        [HIO_ADC_CHANNEL_A2].chselr = ADC_CHSELR_CHSEL2,
        [HIO_ADC_CHANNEL_A3].chselr = ADC_CHSELR_CHSEL3,
        [HIO_ADC_CHANNEL_A4].chselr = ADC_CHSELR_CHSEL4,
        [HIO_ADC_CHANNEL_A5].chselr = ADC_CHSELR_CHSEL5,
        [HIO_ADC_CHANNEL_A6].chselr = ADC_CHSELR_CHSEL6,
        [HIO_ADC_CHANNEL_INTERNAL_REFERENCE] = { NULL, NULL, false, HIO_ADC_RESOLUTION_12_BIT, HIO_ADC_OVERSAMPLING_256, 0, ADC_CHSELR_CHSEL17 }
    }
};

static void _hio_adc_task(void *param);

static inline bool _hio_adc_get_pending(hio_adc_channel_t *next ,hio_adc_channel_t start);

void hio_adc_init()
{
    if (_hio_adc.initialized != true)
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
        _hio_adc.vrefint = (*(uint16_t *) VREFINT_CAL_ADDR);// << 4;

        NVIC_EnableIRQ(ADC1_COMP_IRQn);

        _hio_adc.initialized = true;

        _hio_adc.task_id = hio_scheduler_register(_hio_adc_task, NULL, HIO_TICK_INFINITY);

        hio_adc_calibration();
    }
}

void hio_adc_oversampling_set(hio_adc_channel_t channel, hio_adc_oversampling_t oversampling)
{
    _hio_adc.channel_table[channel].oversampling = oversampling;
}

void hio_adc_resolution_set(hio_adc_channel_t channel, hio_adc_resolution_t resolution)
{
    _hio_adc.channel_table[channel].resolution = resolution;
}

static void _hio_adc_configure_resolution(hio_adc_resolution_t resolution)
{
    ADC1->CFGR1 &= ~ADC_CFGR1_RES_Msk;
    ADC1->CFGR1 |= resolution & ADC_CFGR1_RES_Msk;
}

static void _hio_adc_configure_oversampling(hio_adc_oversampling_t oversampling)
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

static uint16_t _hio_adc_get_measured_value(hio_adc_channel_t channel)
{
    uint16_t value = ADC1->DR;

    switch (_hio_adc.channel_table[channel].resolution)
    {
        case HIO_ADC_RESOLUTION_6_BIT:
        {
            value <<= 10;
            break;
        }
        case HIO_ADC_RESOLUTION_8_BIT:
        {
            value <<= 8;
            break;
        }
        case HIO_ADC_RESOLUTION_10_BIT:
        {
            value <<= 6;
            break;
        }
        case HIO_ADC_RESOLUTION_12_BIT:
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

bool hio_adc_is_ready()
{
    return _hio_adc.channel_in_progress == HIO_ADC_CHANNEL_NONE;
}

bool hio_adc_get_value(hio_adc_channel_t channel, uint16_t *result)
{
    // If ongoing conversion...
    if (_hio_adc.channel_in_progress != HIO_ADC_CHANNEL_NONE)
    {
        return false;
    }

    // Set ADC channel
    ADC1->CHSELR = _hio_adc.channel_table[channel].chselr;

    // Disable all ADC interrupts
    ADC1->IER = 0;

    // Clear EOS flag (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS;

    // Clear ADRDY (it is cleared by software writing 1 to it)
    ADC1->ISR |= ADC_ISR_ADRDY;

    _hio_adc_configure_oversampling(_hio_adc.channel_table[channel].oversampling);
    _hio_adc_configure_resolution(_hio_adc.channel_table[channel].resolution);

    // Start the AD measurement
    ADC1->CR |= ADC_CR_ADSTART;

    // wait for end of measurement
    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    if (result != NULL)
    {
        *result = _hio_adc_get_measured_value(channel);
    }

    return true;
}

bool hio_adc_set_event_handler(hio_adc_channel_t channel, void (*event_handler)(hio_adc_channel_t, hio_adc_event_t, void *), void *event_param)
{
    // Check ongoing on edited channel
    if (_hio_adc.channel_in_progress == channel)
    {
        return false;
    }

    hio_adc_channel_config_t *adc = &_hio_adc.channel_table[channel];

    adc->event_handler = event_handler;
    adc->event_param = event_param;

    return true;
}

bool hio_adc_async_measure(hio_adc_channel_t channel)
{
    // If another conversion is ongoing...
    if (_hio_adc.channel_in_progress != HIO_ADC_CHANNEL_NONE)
    {
        _hio_adc.channel_table[channel].pending = true;

        return true;
    }

    _hio_adc.channel_in_progress = channel;
    _hio_adc.channel_table[channel].pending = false;

    // Skip cal and measure VREF + channel
    //------------------------------------
    _hio_adc.state = HIO_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END;

    // Enable internal reference to ADC peripheral
    ADC->CCR |= ADC_CCR_VREFEN;

    _hio_adc_configure_oversampling(_hio_adc.channel_table[HIO_ADC_CHANNEL_INTERNAL_REFERENCE].oversampling);
    _hio_adc_configure_resolution(_hio_adc.channel_table[HIO_ADC_CHANNEL_INTERNAL_REFERENCE].resolution);

    // Set ADC channel
    ADC1->CHSELR = _hio_adc.channel_table[HIO_ADC_CHANNEL_INTERNAL_REFERENCE].chselr;

    // Clear end of calibration interrupt
    ADC1->ISR = ADC_ISR_EOCAL;

    // Enable end of conversion interrupt
    ADC1->IER = ADC_IER_EOCIE;

    // Begin internal reference reading
    ADC1->CR |= ADC_CR_ADSTART;

    hio_scheduler_disable_sleep();

    return true;
}

bool hio_adc_async_get_value(hio_adc_channel_t channel, uint16_t *result)
{
    *result = _hio_adc.channel_table[channel].value;
    return true;
}

bool hio_adc_async_get_voltage(hio_adc_channel_t channel, float *result)
{
    *result = (_hio_adc.channel_table[channel].value * _hio_adc.real_vdda_voltage) / 65536.f;
    return true;
}

bool hio_adc_get_vdda_voltage(float *vdda_voltage)
{
    if (_hio_adc.real_vdda_voltage == 0.f)
    {
        return false;
    }
    else
    {
        *vdda_voltage = _hio_adc.real_vdda_voltage;

        return true;
    }
}

void ADC1_COMP_IRQHandler(void)
{
    // Get real VDDA and begin analog channel measurement
    if (_hio_adc.state == HIO_ADC_STATE_CALIBRATION_BY_INTERNAL_REFERENCE_END)
    {
        // Compute actual VDDA
        _hio_adc.real_vdda_voltage = 3.f * ((float) _hio_adc.vrefint / (float) ADC1->DR);

        _hio_adc_configure_oversampling(_hio_adc.channel_table[_hio_adc.channel_in_progress].oversampling);
        _hio_adc_configure_resolution(_hio_adc.channel_table[_hio_adc.channel_in_progress].resolution);

        // Set ADC channel
        ADC1->CHSELR = _hio_adc.channel_table[_hio_adc.channel_in_progress].chselr;

        _hio_adc.state = HIO_ADC_STATE_MEASURE_INPUT;

        // Clear end of conversion interrupt
        ADC1->ISR = ADC_ISR_EOC;

        // Begin adc input channel measurement
        ADC1->CR |= ADC_CR_ADSTART;
    }

    // Measurement is done, plan calling callback
    else if (_hio_adc.state == HIO_ADC_STATE_MEASURE_INPUT)
    {
        // Disable internal reference
        ADC->CCR &= ~ADC_CCR_VREFEN;

        _hio_adc.channel_table[_hio_adc.channel_in_progress].value = _hio_adc_get_measured_value(_hio_adc.channel_in_progress);

        // Plan ADC task
        hio_scheduler_plan_now(_hio_adc.task_id);

        // Clear all interrupts
        ADC1->ISR = 0xffff;

        // Disable all ADC interrupts
        ADC1->IER = 0;

        hio_scheduler_enable_sleep();
    }
}

bool hio_adc_calibration(void)
{
    if (_hio_adc.channel_in_progress != HIO_ADC_CHANNEL_NONE)
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
    ADC1->CHSELR = _hio_adc.channel_table[HIO_ADC_CHANNEL_INTERNAL_REFERENCE].chselr;

    // Clear EOS flag (it is cleared by software writing 1 to it)
    ADC1->ISR = ADC_ISR_EOS;

    // Perform measurement on internal reference
    ADC1->CR |= ADC_CR_ADSTART;

    while ((ADC1->ISR & ADC_ISR_EOS) == 0)
    {
        continue;
    }

    // Compute actual VDDA
    _hio_adc.real_vdda_voltage = 3.f * ((float) _hio_adc.vrefint / (float) ADC1->DR);

    // Disable internal reference
    ADC->CCR &= ~ADC_CCR_VREFEN;

    return true;
}

static void _hio_adc_task(void *param)
{
    (void) param;

    hio_adc_channel_config_t *adc = &_hio_adc.channel_table[_hio_adc.channel_in_progress];
    hio_adc_channel_t pending_result_channel;
    hio_adc_channel_t next;

    // Update pending channel result
    pending_result_channel = _hio_adc.channel_in_progress;

    // Release ADC for further conversion
    _hio_adc.channel_in_progress = HIO_ADC_CHANNEL_NONE;

    // Disable interrupts
    hio_irq_disable();

    // Get pending
    if (_hio_adc_get_pending(&next, pending_result_channel) == true)
    {
        hio_adc_async_measure(next);
    }

    // Enable interrupts
    hio_irq_enable();

    // Perform event call-back
    if (adc->event_handler != NULL)
    {
        adc->event_handler(pending_result_channel, HIO_ADC_EVENT_DONE, adc->event_param);
    }
}

static inline bool _hio_adc_get_pending(hio_adc_channel_t *next ,hio_adc_channel_t start)
{
    for (int i = start + 1; i != start; i++)
    {
        if (i == HIO_ADC_CHANNEL_COUNT)
        {
            if (start == HIO_ADC_CHANNEL_A0)
            {
                break;
            }
            else
            {
                i = HIO_ADC_CHANNEL_A0;
            }
        }

        if (_hio_adc.channel_table[i].pending == true)
        {
            *next = i;

            return true;
        }
    }

    return false;
}
