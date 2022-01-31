#include <stm32l0xx.h>
#include <twr_module_battery.h>
#include <twr_gpio.h>
#include <twr_adc.h>
#include <twr_scheduler.h>
#include <twr_timer.h>

#define _TWR_MODULE_BATTERY_CELL_VOLTAGE 1.5f

#define _TWR_MODULE_BATTERY_STANDATD_DEFAULT_LEVEL_LOW        (1.2 * 4)
#define _TWR_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL   (1.0 * 4)

#define _TWR_MODULE_BATTERY_MINI_DEFAULT_LEVEL_LOW        (1.2 * 2)
#define _TWR_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL   (1.0 * 2)

#define _TWR_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)      ((100. * ((__VOLTAGE__) - _TWR_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL)) / ((_TWR_MODULE_BATTERY_CELL_VOLTAGE * 2) - _TWR_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL))
#define _TWR_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)  ((100. * ((__VOLTAGE__) - _TWR_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL)) / ((_TWR_MODULE_BATTERY_CELL_VOLTAGE * 4) - _TWR_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL))

#define _TWR_MODULE_BATTERY_MINI_CALIBRATION(__VOLTAGE__) ((__VOLTAGE__) * 1.095f + 0.0069f)
#define _TWR_MODULE_BATTERY_STANDARD_CALIBRATION(__VOLTAGE__) ((__VOLTAGE__) * 1.1068f + 0.0212f)

#define _TWR_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE(__RESULT__)       ((__RESULT__) * (1 / (5.0 / (5.0 + 10.0))))
#define _TWR_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(__RESULT__)   ((__RESULT__) * (1 / 0.13))

typedef enum
{
        TWR_MODULE_STATE_DETECT_PRESENT = 0,
        TWR_MODULE_STATE_DETECT_FORMAT = 1,
        TWR_MODULE_STATE_MEASURE = 2,
        TWR_MODULE_STATE_READ = 3,
        TWR_MODULE_STATE_UPDATE = 4

} _twr_module_battery_state_t;

static struct
{
    float voltage;
    float valid_min;
    float valid_max;
    twr_module_battery_format_t format;
    void (*event_handler)(twr_module_battery_event_t, void *);
    void *event_param;
    bool measurement_active;
    float level_low_threshold;
    float level_critical_threshold;
    twr_tick_t update_interval;
    twr_tick_t next_update_start;
    twr_scheduler_task_id_t task_id;
    float adc_value;
    _twr_module_battery_state_t state;

} _twr_module_battery;

static bool _twr_module_battery_present_test(void);
static void _twr_module_battery_task(void *param);
static void _twr_module_battery_adc_event_handler(twr_adc_channel_t channel, twr_adc_event_t event, void *param);
static void _twr_module_battery_measurement(int state);

void twr_module_battery_init(void)
{
    memset(&_twr_module_battery, 0, sizeof(_twr_module_battery));

    _twr_module_battery.voltage = NAN;
    _twr_module_battery.adc_value = NAN;
    _twr_module_battery.update_interval = TWR_TICK_INFINITY;
    _twr_module_battery.task_id = twr_scheduler_register(_twr_module_battery_task, NULL, TWR_TICK_INFINITY);

    twr_gpio_init(TWR_GPIO_P0);
    twr_gpio_init(TWR_GPIO_P1);

    twr_timer_init();
}

void twr_module_battery_set_event_handler(void (*event_handler)(twr_module_battery_event_t, void *), void *event_param)
{
    _twr_module_battery.event_handler = event_handler;
    _twr_module_battery.event_param = event_param;
}

void twr_module_battery_set_update_interval(twr_tick_t interval)
{
    _twr_module_battery.update_interval = interval;

    if (_twr_module_battery.update_interval == TWR_TICK_INFINITY)
    {
        if (!_twr_module_battery.measurement_active)
        {
            twr_scheduler_plan_absolute(_twr_module_battery.task_id, TWR_TICK_INFINITY);
        }
    }
    else
    {
        twr_module_battery_measure();
    }
}

void twr_module_battery_set_threshold_levels(float level_low_threshold, float level_critical_threshold)
{
    _twr_module_battery.level_low_threshold = level_low_threshold;
    _twr_module_battery.level_critical_threshold = level_critical_threshold;
}

twr_module_battery_format_t twr_module_battery_get_format()
{
    return _twr_module_battery.format;
}

bool twr_module_battery_measure(void)
{
    if ((_twr_module_battery.measurement_active) ||
        (_twr_module_battery.state == TWR_MODULE_STATE_DETECT_FORMAT) ||
        (_twr_module_battery.state == TWR_MODULE_STATE_READ))
    {
        return false;
    }

    _twr_module_battery.measurement_active = true;

    twr_scheduler_plan_now(_twr_module_battery.task_id);

    return true;
}

bool twr_module_battery_get_voltage(float *voltage)
{
    *voltage = _twr_module_battery.voltage;

    return !isnan(_twr_module_battery.voltage);
}

bool twr_module_battery_get_charge_level(int *percentage)
{
    float voltage;

    if (twr_module_battery_get_voltage(&voltage))
    {
        // Calculate the percentage of charge
        if (_twr_module_battery.format == TWR_MODULE_BATTERY_FORMAT_MINI)
        {
            *percentage = _TWR_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
        }
        else
        {
            *percentage = _TWR_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
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

bool twr_module_battery_is_present(void)
{
    if (_twr_module_battery.state != TWR_MODULE_STATE_DETECT_PRESENT)
    {
        return true;
    }

    return _twr_module_battery_present_test();
}

static bool _twr_module_battery_present_test(void)
{
    twr_system_pll_enable();

    twr_gpio_set_mode(TWR_GPIO_P1, TWR_GPIO_MODE_OUTPUT);

    twr_gpio_set_output(TWR_GPIO_P1, 0);

    twr_gpio_set_mode(TWR_GPIO_P0, TWR_GPIO_MODE_OUTPUT);

    twr_gpio_set_output(TWR_GPIO_P0, 1);

    twr_gpio_set_pull(TWR_GPIO_P0, TWR_GPIO_PULL_DOWN);

    twr_gpio_set_mode(TWR_GPIO_P0, TWR_GPIO_MODE_INPUT);

    __NOP();

    int value = twr_gpio_get_input(TWR_GPIO_P0);

    twr_system_pll_disable();

    return value != 0;
}

static void _twr_module_battery_task(void *param)
{
    (void) param;

start:

    switch (_twr_module_battery.state)
    {

        case TWR_MODULE_STATE_DETECT_PRESENT:
        {
            if (_twr_module_battery.update_interval == TWR_TICK_INFINITY)
            {
                _twr_module_battery.next_update_start = TWR_TICK_INFINITY;
            }
            else
            {
                _twr_module_battery.next_update_start = twr_tick_get() + _twr_module_battery.update_interval;
            }

            _twr_module_battery.format = TWR_MODULE_BATTERY_FORMAT_UNKNOWN;

            if (!_twr_module_battery_present_test())
            {
                twr_scheduler_plan_current_absolute(_twr_module_battery.next_update_start);

                if (_twr_module_battery.next_update_start == TWR_TICK_INFINITY)
                {
                    _twr_module_battery.measurement_active = false;
                }

                if (_twr_module_battery.event_handler != NULL)
                {
                    _twr_module_battery.event_handler(TWR_MODULE_BATTERY_EVENT_ERROR, _twr_module_battery.event_param);
                }

                return;
            }

            _twr_module_battery_measurement(ENABLE);

            twr_adc_init();
            twr_adc_oversampling_set(TWR_ADC_CHANNEL_A0, TWR_ADC_OVERSAMPLING_256);
            twr_adc_set_event_handler(TWR_ADC_CHANNEL_A0, _twr_module_battery_adc_event_handler, NULL);

            twr_adc_async_measure(TWR_ADC_CHANNEL_A0);

            _twr_module_battery.state = TWR_MODULE_STATE_DETECT_FORMAT;

            break;
        }
        case TWR_MODULE_STATE_DETECT_FORMAT:
        {
            float voltage = _TWR_MODULE_BATTERY_STANDARD_CALIBRATION(_TWR_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(_twr_module_battery.adc_value));

            if ((voltage > 3.8) && (voltage < 7.0))
            {
                _twr_module_battery.format = TWR_MODULE_BATTERY_FORMAT_STANDARD;
                _twr_module_battery.level_low_threshold = _TWR_MODULE_BATTERY_STANDATD_DEFAULT_LEVEL_LOW;
                _twr_module_battery.level_critical_threshold = _TWR_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL;
                _twr_module_battery.valid_min = 3.8;
                _twr_module_battery.valid_max = 7.0;
            }
            else
            {
                _twr_module_battery.format = TWR_MODULE_BATTERY_FORMAT_MINI;
                _twr_module_battery.level_low_threshold = _TWR_MODULE_BATTERY_MINI_DEFAULT_LEVEL_LOW;
                _twr_module_battery.level_critical_threshold = _TWR_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL;
                _twr_module_battery.valid_min = 1.8;
                _twr_module_battery.valid_max = 3.8;
            }

            _twr_module_battery.state = TWR_MODULE_STATE_MEASURE;

            if (_twr_module_battery.measurement_active)
            {
                twr_scheduler_plan_current_now();
            }
            else
            {
                twr_scheduler_plan_current_absolute(_twr_module_battery.next_update_start);
            }

            break;
        }
        case TWR_MODULE_STATE_MEASURE:
        {
            _twr_module_battery.measurement_active = true;

            if (_twr_module_battery.update_interval == TWR_TICK_INFINITY)
            {
                _twr_module_battery.next_update_start = TWR_TICK_INFINITY;
            }
            else
            {
                _twr_module_battery.next_update_start = twr_tick_get() + _twr_module_battery.update_interval;
            }

            _twr_module_battery_measurement(ENABLE);

            twr_adc_set_event_handler(TWR_ADC_CHANNEL_A0, _twr_module_battery_adc_event_handler, NULL);

            twr_adc_async_measure(TWR_ADC_CHANNEL_A0);

            _twr_module_battery.state = TWR_MODULE_STATE_READ;

            break;
        }
        case TWR_MODULE_STATE_READ:
        {
            if (_twr_module_battery.format == TWR_MODULE_BATTERY_FORMAT_MINI)
            {
                _twr_module_battery.voltage = _TWR_MODULE_BATTERY_MINI_CALIBRATION(_TWR_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE(_twr_module_battery.adc_value));
            }
            else
            {
                _twr_module_battery.voltage = _TWR_MODULE_BATTERY_STANDARD_CALIBRATION(_TWR_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(_twr_module_battery.adc_value));
            }

            _twr_module_battery.measurement_active = false;

            if ((_twr_module_battery.voltage < _twr_module_battery.valid_min) || (_twr_module_battery.voltage > _twr_module_battery.valid_max))
            {
                _twr_module_battery.voltage = NAN;

                _twr_module_battery.state = TWR_MODULE_STATE_DETECT_PRESENT;

                twr_scheduler_plan_current_absolute(_twr_module_battery.next_update_start);

                if (_twr_module_battery.event_handler != NULL)
                {
                    _twr_module_battery.event_handler(TWR_MODULE_BATTERY_EVENT_ERROR, _twr_module_battery.event_param);
                }

                return;
            }

            _twr_module_battery.state = TWR_MODULE_STATE_UPDATE;

            goto start;
        }
        case TWR_MODULE_STATE_UPDATE:
        {
            if (_twr_module_battery.event_handler != NULL)
            {
                // Notify event based on calculated percentage
                if (_twr_module_battery.voltage <= _twr_module_battery.level_critical_threshold)
                {
                    _twr_module_battery.event_handler(TWR_MODULE_BATTERY_EVENT_LEVEL_CRITICAL, _twr_module_battery.event_param);
                }
                else if (_twr_module_battery.voltage <= _twr_module_battery.level_low_threshold)
                {
                    _twr_module_battery.event_handler(TWR_MODULE_BATTERY_EVENT_LEVEL_LOW, _twr_module_battery.event_param);
                }

                _twr_module_battery.event_handler(TWR_MODULE_BATTERY_EVENT_UPDATE, _twr_module_battery.event_param);
            }

            _twr_module_battery.state = TWR_MODULE_STATE_MEASURE;

            twr_scheduler_plan_current_absolute(_twr_module_battery.next_update_start);

            break;
        }
        default:
        {
            return;
        }
    }
}

static void _twr_module_battery_adc_event_handler(twr_adc_channel_t channel, twr_adc_event_t event, void *param)
{
    (void) channel;
    (void) param;

    if (event == TWR_ADC_EVENT_DONE)
    {

        if (!twr_adc_async_get_voltage(TWR_ADC_CHANNEL_A0, &_twr_module_battery.adc_value))
        {
            _twr_module_battery.adc_value = NAN;
        }

        _twr_module_battery_measurement(DISABLE);

        twr_scheduler_plan_now(_twr_module_battery.task_id);
    }
}

static void _twr_module_battery_measurement(int state)
{
    if (_twr_module_battery.format == TWR_MODULE_BATTERY_FORMAT_MINI)
    {
        twr_gpio_set_mode(TWR_GPIO_P1, state == ENABLE ? TWR_GPIO_MODE_OUTPUT : TWR_GPIO_MODE_ANALOG);
    }
    else
    {
        twr_gpio_set_output(TWR_GPIO_P1, state);
    }

    if (state == ENABLE)
    {
        twr_timer_start();

        twr_timer_delay(100);

        twr_timer_stop();
    }
}

