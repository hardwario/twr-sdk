#include <stm32l0xx.h>
#include <bc_module_battery.h>
#include <bc_gpio.h>
#include <bc_adc.h>
#include <bc_scheduler.h>

#define _BC_MODULE_BATTERY_CELL_VOLTAGE 1.6f

#define _BC_MODULE_BATTERY_DEFAULT_LEVEL_LOW        1.2
#define _BC_MODULE_BATTERY_DEFAULT_LEVEL_CRITICAL   1.0

#define _BC_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)      ((100. * __VOLTAGE__) / _BC_MODULE_BATTERY_CELL_VOLTAGE)
#define _BC_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)  ((100. * __VOLTAGE__) / _BC_MODULE_BATTERY_CELL_VOLTAGE)

#define _BC_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE_ON_BATTERY(__RESULT__)       (((__RESULT__) * (1 / 0.33)) / 2.)
#define _BC_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE_ON_BATTERY(__RESULT__)   (((__RESULT__) * (1 / 0.13)) / 4.)

static struct
{
    float voltage_on_battery;
    bc_module_battery_format_t format;
    float level_low_threshold;
    float level_critical_threshold;
    void (*event_handler)(bc_module_battery_event_t, void *);
    void *event_param;
    bool valid;
    bool measurement_active;
    bc_tick_t update_interval;
    bc_scheduler_task_id_t task_id;
} _bc_module_battery;

static void _bc_module_battery_task();
static void _bc_module_battery_adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *param);
static void _bc_module_battery_measurement(int state);
static void _bc_module_battery_update_voltage_on_battery(void);

void bc_module_battery_init(bc_module_battery_format_t format)
{
    memset(&_bc_module_battery, 0, sizeof(_bc_module_battery));

    _bc_module_battery.task_id = bc_scheduler_register(_bc_module_battery_task, NULL, BC_TICK_INFINITY);
    _bc_module_battery.format = format;
    _bc_module_battery.level_low_threshold = _BC_MODULE_BATTERY_DEFAULT_LEVEL_LOW;
    _bc_module_battery.level_critical_threshold = _BC_MODULE_BATTERY_DEFAULT_LEVEL_CRITICAL;

    bc_gpio_init(BC_GPIO_P1);
    _bc_module_battery_measurement(DISABLE);
    bc_gpio_set_mode(BC_GPIO_P1, BC_GPIO_MODE_OUTPUT);

    // Initialize ADC channel
    bc_adc_init(BC_ADC_CHANNEL_A0, BC_ADC_FORMAT_FLOAT);
    bc_adc_set_event_handler(BC_ADC_CHANNEL_A0, _bc_module_battery_adc_event_handler, NULL);
}

void bc_module_battery_set_event_handler(void (*event_handler)(bc_module_battery_event_t, void *), void *event_param)
{
    _bc_module_battery.event_handler = event_handler;
    _bc_module_battery.event_param = event_param;
}

void bc_module_battery_set_update_interval(bc_tick_t interval)
{
    _bc_module_battery.update_interval = interval;

    if (_bc_module_battery.update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(_bc_module_battery.task_id, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(_bc_module_battery.task_id, _bc_module_battery.update_interval);

        bc_module_battery_measure();
    }
}

void bc_module_battery_set_threshold_levels(float level_low_threshold, float level_critical_threshold)
{
    _bc_module_battery.level_low_threshold = level_low_threshold;
    _bc_module_battery.level_critical_threshold = level_critical_threshold;
}

bool bc_module_battery_measure(void)
{
    if (_bc_module_battery.measurement_active)
    {
        return false;
    }

    _bc_module_battery.measurement_active = true;

    _bc_module_battery_measurement(ENABLE);

    bc_adc_async_read(BC_ADC_CHANNEL_A0);

    return true;
}

bool bc_module_battery_update_voltage_on_battery(float *voltage)
{
    if(_bc_module_battery.valid == true)
    {
        *voltage = _bc_module_battery.voltage_on_battery;

        return true;
    }

    return false;
}

bool bc_module_battery_get_charge_level(int *percentage)
{
    if (_bc_module_battery.valid == true)
    {
        float voltage;

        bc_module_battery_update_voltage_on_battery(&voltage);

        // Calculate the percentage of charge
        if (_bc_module_battery.format == BC_MODULE_BATTERY_FORMAT_MINI)
        {
            *percentage = _BC_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
        }
        else
        {
            *percentage = _BC_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
        }

        if (*percentage >= 100)
        {
            *percentage = 100;
        }

        return true;
    }

    return false;
}

static void _bc_module_battery_task(void *param)
{
    (void) param;

    bc_scheduler_plan_current_relative(_bc_module_battery.update_interval);

    _bc_module_battery_measurement(ENABLE);

    // Lock measurement
    _bc_module_battery.measurement_active = true;

    bc_adc_async_read(BC_ADC_CHANNEL_A0);
}

static void _bc_module_battery_adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *param)
{
    (void) channel;
    (void) param;

    if (event == BC_ADC_EVENT_DONE)
    {
        _bc_module_battery_update_voltage_on_battery();

        // Security measures ...
        _bc_module_battery.valid = true;

        _bc_module_battery_measurement(DISABLE);

        if (_bc_module_battery.event_handler != NULL)
        {
            // Notify event based on calculated percentage
            if (_bc_module_battery.voltage_on_battery <= _bc_module_battery.level_critical_threshold)
            {
                _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_LEVEL_CRITICAL, _bc_module_battery.event_param);
            }
            else if (_bc_module_battery.voltage_on_battery <= _bc_module_battery.level_low_threshold)
            {
                _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_LEVEL_LOW, _bc_module_battery.event_param);
            }
            else
            {
                _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_UPDATE, _bc_module_battery.event_param);
            }
        }
    }

    // Unlock measurement
    _bc_module_battery.measurement_active = false;
}

static void _bc_module_battery_measurement(int state)
{
    if (_bc_module_battery.format == BC_MODULE_BATTERY_FORMAT_MINI)
    {
        bc_gpio_set_output(BC_GPIO_P1, !state);
    }
    else
    {
        bc_gpio_set_output(BC_GPIO_P1, state);
    }
}

static void _bc_module_battery_update_voltage_on_battery(void)
{
    float v;

    bc_adc_get_result(BC_ADC_CHANNEL_A0, &v);

    if (_bc_module_battery.format == BC_MODULE_BATTERY_FORMAT_MINI)
    {
        _bc_module_battery.voltage_on_battery = _BC_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE_ON_BATTERY(v);
    }
    else
    {
        _bc_module_battery.voltage_on_battery = _BC_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE_ON_BATTERY(v);
    }
}
