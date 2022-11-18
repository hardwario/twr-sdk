#include <application.h>

twr_button_t button;

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_module_battery_measure();

        float voltage = 0.0;
        twr_module_battery_get_voltage(&voltage);

        int chargePercentage = -1;
        twr_module_battery_get_charge_level(&chargePercentage);

        twr_log_debug("Voltage %.3f", voltage);
        twr_log_debug("Charge: %d", chargePercentage);
    }
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_module_battery_init();
}
