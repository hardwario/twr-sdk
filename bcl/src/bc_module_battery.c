#include <stm32l0xx.h>
#include <bc_module_battery.h>
#include <bc_gpio.h>
#include <bc_adc.h>
#include <bc_scheduler.h>
#include <bc_timer.h>

#define _BC_MODULE_BATTERY_CELL_VOLTAGE 1.5f

#define _BC_MODULE_BATTERY_STANDATD_DEFAULT_LEVEL_LOW        (1.2 * 4)
#define _BC_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL   (1.0 * 4)

#define _BC_MODULE_BATTERY_MINI_DEFAULT_LEVEL_LOW        (1.2 * 2)
#define _BC_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL   (1.0 * 2)

#define _BC_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)      ((100. * ((__VOLTAGE__) - _BC_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL)) / ((_BC_MODULE_BATTERY_CELL_VOLTAGE * 2) - _BC_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL))
#define _BC_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)  ((100. * ((__VOLTAGE__) - _BC_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL)) / ((_BC_MODULE_BATTERY_CELL_VOLTAGE * 4) - _BC_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL))

#define _BC_MODULE_BATTERY_MINI_CALIBRATION(__VOLTAGE__) ((__VOLTAGE__) * 1.095f + 0.0069f)
#define _BC_MODULE_BATTERY_STANDARD_CALIBRATION(__VOLTAGE__) ((__VOLTAGE__) * 1.1068f + 0.0212f)

#define _BC_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE(__RESULT__)       ((__RESULT__) * (1 / (5.0 / (5.0 + 10.0))))
#define _BC_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(__RESULT__)   ((__RESULT__) * (1 / 0.13))

typedef enum
{
        BC_MODULE_STATE_DETECT_PRESENT = 0,
        BC_MODULE_STATE_DETECT_FORMAT = 1,
        BC_MODULE_STATE_MEASURE = 2,
        BC_MODULE_STATE_READ = 3,
        BC_MODULE_STATE_UPDATE = 4

} _bc_module_battery_state_t;

static struct
{
    float voltage;
    float valid_min;
    float valid_max;
    bc_module_battery_format_t format;
    void (*event_handler)(bc_module_battery_event_t, void *);
    void *event_param;
    bool measurement_active;
    float level_low_threshold;
    float level_critical_threshold;
    bc_tick_t update_interval;
    bc_tick_t next_update_start;
    bc_scheduler_task_id_t task_id;
    float adc_value;
    _bc_module_battery_state_t state;

} _bc_module_battery;

static bool _bc_module_battery_present_test(void);
static void _bc_module_battery_task(void *param);
static void _bc_module_battery_adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *param);
static void _bc_module_battery_measurement(int state);

void bc_module_battery_init(void)
{
    memset(&_bc_module_battery, 0, sizeof(_bc_module_battery));

    _bc_module_battery.voltage = NAN;
    _bc_module_battery.adc_value = NAN;
    _bc_module_battery.update_interval = BC_TICK_INFINITY;
    _bc_module_battery.task_id = bc_scheduler_register(_bc_module_battery_task, NULL, BC_TICK_INFINITY);

    bc_gpio_init(BC_GPIO_P0);
    bc_gpio_init(BC_GPIO_P1);

    bc_timer_init();
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
        if (!_bc_module_battery.measurement_active)
        {
            bc_scheduler_plan_absolute(_bc_module_battery.task_id, BC_TICK_INFINITY);
        }
    }
    else
    {
        bc_module_battery_measure();
    }
}

void bc_module_battery_set_threshold_levels(float level_low_threshold, float level_critical_threshold)
{
    _bc_module_battery.level_low_threshold = level_low_threshold;
    _bc_module_battery.level_critical_threshold = level_critical_threshold;
}

bc_module_battery_format_t bc_module_battery_get_format()
{
    return _bc_module_battery.format;
}

bool bc_module_battery_measure(void)
{
    if (_bc_module_battery.measurement_active)
    {
        return false;
    }

    _bc_module_battery.measurement_active = true;

    bc_scheduler_plan_now(_bc_module_battery.task_id);

    return true;
}

bool bc_module_battery_get_voltage(float *voltage)
{
    *voltage = _bc_module_battery.voltage;

    return !isnan(_bc_module_battery.voltage);
}

bool bc_module_battery_get_charge_level(int *percentage)
{
    float voltage;

    if (bc_module_battery_get_voltage(&voltage))
    {
        // Calculate the percentage of charge
        if (_bc_module_battery.format == BC_MODULE_BATTERY_FORMAT_MINI)
        {
            *percentage = _BC_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
        }
        else
        {
            *percentage = _BC_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
        }

        if (*percentage > 100)
        {
            *percentage = 100;
        }
        else if (*percentage < 0)
        {
            *percentage = 0;
        }

        return true;
    }

    return false;
}

bool bc_module_battery_is_present(void)
{
    if (_bc_module_battery.state != BC_MODULE_STATE_DETECT_PRESENT)
    {
        return true;
    }

    return _bc_module_battery_present_test();
}

static bool _bc_module_battery_present_test(void)
{
    bc_system_pll_enable();

    bc_gpio_set_mode(BC_GPIO_P1, BC_GPIO_MODE_OUTPUT);

    bc_gpio_set_output(BC_GPIO_P1, 0);

    bc_gpio_set_mode(BC_GPIO_P0, BC_GPIO_MODE_OUTPUT);

    bc_gpio_set_output(BC_GPIO_P0, 1);

    bc_gpio_set_pull(BC_GPIO_P0, BC_GPIO_PULL_DOWN);

    bc_gpio_set_mode(BC_GPIO_P0, BC_GPIO_MODE_INPUT);

    __NOP();

    int value = bc_gpio_get_input(BC_GPIO_P0);

    bc_system_pll_disable();

    return value != 0;
}

static void _bc_module_battery_task(void *param)
{
    (void) param;

start:

    switch (_bc_module_battery.state)
    {

        case BC_MODULE_STATE_DETECT_PRESENT:
        {
            if (_bc_module_battery.update_interval == BC_TICK_INFINITY)
            {
                _bc_module_battery.next_update_start = BC_TICK_INFINITY;
            }
            else
            {
                _bc_module_battery.next_update_start = bc_tick_get() + _bc_module_battery.update_interval;
            }

            _bc_module_battery.format = BC_MODULE_BATTERY_FORMAT_UNKNOWN;

            if (!_bc_module_battery_present_test())
            {
                bc_scheduler_plan_current_absolute(_bc_module_battery.next_update_start);

                if (_bc_module_battery.next_update_start == BC_TICK_INFINITY)
                {
                    _bc_module_battery.measurement_active = false;
                }

                if (_bc_module_battery.event_handler != NULL)
                {
                    _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_ERROR, _bc_module_battery.event_param);
                }

                return;
            }

            _bc_module_battery_measurement(ENABLE);

            bc_adc_init();
            bc_adc_oversampling_set(BC_ADC_CHANNEL_A0, BC_ADC_OVERSAMPLING_256);
            bc_adc_set_event_handler(BC_ADC_CHANNEL_A0, _bc_module_battery_adc_event_handler, NULL);

            bc_adc_async_measure(BC_ADC_CHANNEL_A0);

            _bc_module_battery.state = BC_MODULE_STATE_DETECT_FORMAT;

            break;
        }
        case BC_MODULE_STATE_DETECT_FORMAT:
        {
            float voltage = _BC_MODULE_BATTERY_STANDARD_CALIBRATION(_BC_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(_bc_module_battery.adc_value));

            if ((voltage > 3.8) && (voltage < 7.0))
            {
                _bc_module_battery.format = BC_MODULE_BATTERY_FORMAT_STANDARD;
                _bc_module_battery.level_low_threshold = _BC_MODULE_BATTERY_STANDATD_DEFAULT_LEVEL_LOW;
                _bc_module_battery.level_critical_threshold = _BC_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL;
                _bc_module_battery.valid_min = 3.8;
                _bc_module_battery.valid_max = 7.0;
            }
            else
            {
                _bc_module_battery.format = BC_MODULE_BATTERY_FORMAT_MINI;
                _bc_module_battery.level_low_threshold = _BC_MODULE_BATTERY_MINI_DEFAULT_LEVEL_LOW;
                _bc_module_battery.level_critical_threshold = _BC_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL;
                _bc_module_battery.valid_min = 1.8;
                _bc_module_battery.valid_max = 3.8;
            }

            _bc_module_battery.state = BC_MODULE_STATE_MEASURE;

            if (_bc_module_battery.measurement_active)
            {
                bc_scheduler_plan_current_now();
            }
            else
            {
                bc_scheduler_plan_current_absolute(_bc_module_battery.next_update_start);
            }

            break;
        }
        case BC_MODULE_STATE_MEASURE:
        {
            if (_bc_module_battery.update_interval == BC_TICK_INFINITY)
            {
                _bc_module_battery.next_update_start = BC_TICK_INFINITY;
            }
            else
            {
                _bc_module_battery.next_update_start = bc_tick_get() + _bc_module_battery.update_interval;
            }

            _bc_module_battery_measurement(ENABLE);

            bc_adc_set_event_handler(BC_ADC_CHANNEL_A0, _bc_module_battery_adc_event_handler, NULL);

            bc_adc_async_measure(BC_ADC_CHANNEL_A0);

            _bc_module_battery.state = BC_MODULE_STATE_READ;

            break;
        }
        case BC_MODULE_STATE_READ:
        {
            if (_bc_module_battery.format == BC_MODULE_BATTERY_FORMAT_MINI)
            {
                _bc_module_battery.voltage = _BC_MODULE_BATTERY_MINI_CALIBRATION(_BC_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE(_bc_module_battery.adc_value));
            }
            else
            {
                _bc_module_battery.voltage = _BC_MODULE_BATTERY_STANDARD_CALIBRATION(_BC_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(_bc_module_battery.adc_value));
            }

            _bc_module_battery.measurement_active = false;

            if ((_bc_module_battery.voltage < _bc_module_battery.valid_min) || (_bc_module_battery.voltage > _bc_module_battery.valid_max))
            {
                _bc_module_battery.voltage = NAN;

                _bc_module_battery.state = BC_MODULE_STATE_DETECT_PRESENT;

                bc_scheduler_plan_current_absolute(_bc_module_battery.next_update_start);

                if (_bc_module_battery.event_handler != NULL)
                {
                    _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_ERROR, _bc_module_battery.event_param);
                }

                return;
            }

            _bc_module_battery.state = BC_MODULE_STATE_UPDATE;

            goto start;
        }
        case BC_MODULE_STATE_UPDATE:
        {
            if (_bc_module_battery.event_handler != NULL)
            {
                // Notify event based on calculated percentage
                if (_bc_module_battery.voltage <= _bc_module_battery.level_critical_threshold)
                {
                    _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_LEVEL_CRITICAL, _bc_module_battery.event_param);
                }
                else if (_bc_module_battery.voltage <= _bc_module_battery.level_low_threshold)
                {
                    _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_LEVEL_LOW, _bc_module_battery.event_param);
                }

                _bc_module_battery.event_handler(BC_MODULE_BATTERY_EVENT_UPDATE, _bc_module_battery.event_param);
            }

            _bc_module_battery.state = BC_MODULE_STATE_MEASURE;

            bc_scheduler_plan_current_absolute(_bc_module_battery.next_update_start);

            break;
        }
        default:
        {
            return;
        }
    }
}

static void _bc_module_battery_adc_event_handler(bc_adc_channel_t channel, bc_adc_event_t event, void *param)
{
    (void) channel;
    (void) param;

    if (event == BC_ADC_EVENT_DONE)
    {

        if (!bc_adc_async_get_voltage(BC_ADC_CHANNEL_A0, &_bc_module_battery.adc_value))
        {
            _bc_module_battery.adc_value = NAN;
        }

        _bc_module_battery_measurement(DISABLE);

        bc_scheduler_plan_now(_bc_module_battery.task_id);
    }
}

static void _bc_module_battery_measurement(int state)
{
    if (_bc_module_battery.format == BC_MODULE_BATTERY_FORMAT_MINI)
    {
        bc_gpio_set_mode(BC_GPIO_P1, state == ENABLE ? BC_GPIO_MODE_OUTPUT : BC_GPIO_MODE_ANALOG);
    }
    else
    {
        bc_gpio_set_output(BC_GPIO_P1, state);
    }

    if (state == ENABLE)
    {
        bc_timer_start();

        bc_timer_delay(100);

        bc_timer_stop();
    }
}

