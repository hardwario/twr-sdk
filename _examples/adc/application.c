#include <application.h>

static void _adc_event_handler(twr_adc_channel_t channel, twr_adc_event_t event, void *param)
{
    (void) channel;
    (void) param;

    if (event == TWR_ADC_EVENT_DONE)
    {
        uint16_t adc;
        twr_adc_async_get_value(TWR_ADC_CHANNEL_A2, &adc);
        twr_log_debug("%d", adc);

        float voltage;
        twr_adc_async_get_voltage(TWR_ADC_CHANNEL_A2, &voltage);
        twr_log_debug("%f", voltage);
    }
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_OFF);

    twr_adc_init();
    twr_adc_set_event_handler(TWR_ADC_CHANNEL_A2, _adc_event_handler, NULL);
    twr_adc_resolution_set(TWR_ADC_CHANNEL_A2, TWR_ADC_RESOLUTION_12_BIT);
    twr_adc_oversampling_set(TWR_ADC_CHANNEL_A2, TWR_ADC_OVERSAMPLING_256);
}

void application_task()
{
    twr_adc_async_measure(TWR_ADC_CHANNEL_A2);

    twr_scheduler_plan_current_relative(200);
}
