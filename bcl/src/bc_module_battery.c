#include <bc_module_battery.h>
#include <bc_gpio.h>

static struct
{
    uint16_t adc_result;

} _bc_module_battery;

void bc_module_battery_init(bc_module_battery_format_t format)
{
    memset(&_bc_module_battery, 0, sizeof(_bc_module_battery));

    bc_gpio_init(BC_GPIO_P1);
    bc_gpio_set_output(BC_GPIO_P1, false);
    bc_gpio_set_mode(BC_GPIO_P1, BC_GPIO_MODE_OUTPUT);

    // TODO Implement
    (void) format;
}

void bc_module_battery_set_event_handler(void (*event_handler)(bc_module_battery_event_t, void *), void *event_param)
{
    // TODO Implement
    (void) event_handler;
    (void) event_param;
}

void bc_module_battery_set_update_interval(bc_tick_t interval)
{
    // TODO Implement
    (void) interval;
}

void bc_module_battery_measure(void)
{
    // TODO Implement
}

bool bc_module_battery_get_voltage_volt(float *voltage)
{
    // TODO Implement
    (void) voltage;
    return false;
}
