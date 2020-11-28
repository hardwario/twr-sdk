#include <stm32l0xx.h>
#include <hio_module_battery.h>
#include <hio_gpio.h>
#include <hio_adc.h>
#include <hio_scheduler.h>
#include <hio_timer.h>

#define _HIO_MODULE_BATTERY_CELL_VOLTAGE 1.5f

#define _HIO_MODULE_BATTERY_STANDATD_DEFAULT_LEVEL_LOW        (1.2 * 4)
#define _HIO_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL   (1.0 * 4)

#define _HIO_MODULE_BATTERY_MINI_DEFAULT_LEVEL_LOW        (1.2 * 2)
#define _HIO_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL   (1.0 * 2)

#define _HIO_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)      ((100. * ((__VOLTAGE__) - _HIO_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL)) / ((_HIO_MODULE_BATTERY_CELL_VOLTAGE * 2) - _HIO_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL))
#define _HIO_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(__VOLTAGE__)  ((100. * ((__VOLTAGE__) - _HIO_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL)) / ((_HIO_MODULE_BATTERY_CELL_VOLTAGE * 4) - _HIO_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL))

#define _HIO_MODULE_BATTERY_MINI_CALIBRATION(__VOLTAGE__) ((__VOLTAGE__) * 1.095f + 0.0069f)
#define _HIO_MODULE_BATTERY_STANDARD_CALIBRATION(__VOLTAGE__) ((__VOLTAGE__) * 1.1068f + 0.0212f)

#define _HIO_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE(__RESULT__)       ((__RESULT__) * (1 / (5.0 / (5.0 + 10.0))))
#define _HIO_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(__RESULT__)   ((__RESULT__) * (1 / 0.13))

typedef enum
{
        HIO_MODULE_STATE_DETECT_PRESENT = 0,
        HIO_MODULE_STATE_DETECT_FORMAT = 1,
        HIO_MODULE_STATE_MEASURE = 2,
        HIO_MODULE_STATE_READ = 3,
        HIO_MODULE_STATE_UPDATE = 4

} _hio_module_battery_state_t;

static struct
{
    float voltage;
    float valid_min;
    float valid_max;
    hio_module_battery_format_t format;
    void (*event_handler)(hio_module_battery_event_t, void *);
    void *event_param;
    bool measurement_active;
    float level_low_threshold;
    float level_critical_threshold;
    hio_tick_t update_interval;
    hio_tick_t next_update_start;
    hio_scheduler_task_id_t task_id;
    float adc_value;
    _hio_module_battery_state_t state;

} _hio_module_battery;

static bool _hio_module_battery_present_test(void);
static void _hio_module_battery_task(void *param);
static void _hio_module_battery_adc_event_handler(hio_adc_channel_t channel, hio_adc_event_t event, void *param);
static void _hio_module_battery_measurement(int state);

void hio_module_battery_init(void)
{
    memset(&_hio_module_battery, 0, sizeof(_hio_module_battery));

    _hio_module_battery.voltage = NAN;
    _hio_module_battery.adc_value = NAN;
    _hio_module_battery.update_interval = HIO_TICK_INFINITY;
    _hio_module_battery.task_id = hio_scheduler_register(_hio_module_battery_task, NULL, HIO_TICK_INFINITY);

    hio_gpio_init(HIO_GPIO_P0);
    hio_gpio_init(HIO_GPIO_P1);

    hio_timer_init();
}

void hio_module_battery_set_event_handler(void (*event_handler)(hio_module_battery_event_t, void *), void *event_param)
{
    _hio_module_battery.event_handler = event_handler;
    _hio_module_battery.event_param = event_param;
}

void hio_module_battery_set_update_interval(hio_tick_t interval)
{
    _hio_module_battery.update_interval = interval;

    if (_hio_module_battery.update_interval == HIO_TICK_INFINITY)
    {
        if (!_hio_module_battery.measurement_active)
        {
            hio_scheduler_plan_absolute(_hio_module_battery.task_id, HIO_TICK_INFINITY);
        }
    }
    else
    {
        hio_module_battery_measure();
    }
}

void hio_module_battery_set_threshold_levels(float level_low_threshold, float level_critical_threshold)
{
    _hio_module_battery.level_low_threshold = level_low_threshold;
    _hio_module_battery.level_critical_threshold = level_critical_threshold;
}

hio_module_battery_format_t hio_module_battery_get_format()
{
    return _hio_module_battery.format;
}

bool hio_module_battery_measure(void)
{
    if (_hio_module_battery.measurement_active)
    {
        return false;
    }

    _hio_module_battery.measurement_active = true;

    hio_scheduler_plan_now(_hio_module_battery.task_id);

    return true;
}

bool hio_module_battery_get_voltage(float *voltage)
{
    *voltage = _hio_module_battery.voltage;

    return !isnan(_hio_module_battery.voltage);
}

bool hio_module_battery_get_charge_level(int *percentage)
{
    float voltage;

    if (hio_module_battery_get_voltage(&voltage))
    {
        // Calculate the percentage of charge
        if (_hio_module_battery.format == HIO_MODULE_BATTERY_FORMAT_MINI)
        {
            *percentage = _HIO_MODULE_BATTERY_MINI_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
        }
        else
        {
            *percentage = _HIO_MODULE_BATTERY_STANDARD_VOLTAGE_ON_BATTERY_TO_PERCENTAGE(voltage);
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

bool hio_module_battery_is_present(void)
{
    if (_hio_module_battery.state != HIO_MODULE_STATE_DETECT_PRESENT)
    {
        return true;
    }

    return _hio_module_battery_present_test();
}

static bool _hio_module_battery_present_test(void)
{
    hio_system_pll_enable();

    hio_gpio_set_mode(HIO_GPIO_P1, HIO_GPIO_MODE_OUTPUT);

    hio_gpio_set_output(HIO_GPIO_P1, 0);

    hio_gpio_set_mode(HIO_GPIO_P0, HIO_GPIO_MODE_OUTPUT);

    hio_gpio_set_output(HIO_GPIO_P0, 1);

    hio_gpio_set_pull(HIO_GPIO_P0, HIO_GPIO_PULL_DOWN);

    hio_gpio_set_mode(HIO_GPIO_P0, HIO_GPIO_MODE_INPUT);

    __NOP();

    int value = hio_gpio_get_input(HIO_GPIO_P0);

    hio_system_pll_disable();

    return value != 0;
}

static void _hio_module_battery_task(void *param)
{
    (void) param;

start:

    switch (_hio_module_battery.state)
    {

        case HIO_MODULE_STATE_DETECT_PRESENT:
        {
            if (_hio_module_battery.update_interval == HIO_TICK_INFINITY)
            {
                _hio_module_battery.next_update_start = HIO_TICK_INFINITY;
            }
            else
            {
                _hio_module_battery.next_update_start = hio_tick_get() + _hio_module_battery.update_interval;
            }

            _hio_module_battery.format = HIO_MODULE_BATTERY_FORMAT_UNKNOWN;

            if (!_hio_module_battery_present_test())
            {
                hio_scheduler_plan_current_absolute(_hio_module_battery.next_update_start);

                if (_hio_module_battery.next_update_start == HIO_TICK_INFINITY)
                {
                    _hio_module_battery.measurement_active = false;
                }

                if (_hio_module_battery.event_handler != NULL)
                {
                    _hio_module_battery.event_handler(HIO_MODULE_BATTERY_EVENT_ERROR, _hio_module_battery.event_param);
                }

                return;
            }

            _hio_module_battery_measurement(ENABLE);

            hio_adc_init();
            hio_adc_oversampling_set(HIO_ADC_CHANNEL_A0, HIO_ADC_OVERSAMPLING_256);
            hio_adc_set_event_handler(HIO_ADC_CHANNEL_A0, _hio_module_battery_adc_event_handler, NULL);

            hio_adc_async_measure(HIO_ADC_CHANNEL_A0);

            _hio_module_battery.state = HIO_MODULE_STATE_DETECT_FORMAT;

            break;
        }
        case HIO_MODULE_STATE_DETECT_FORMAT:
        {
            float voltage = _HIO_MODULE_BATTERY_STANDARD_CALIBRATION(_HIO_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(_hio_module_battery.adc_value));

            if ((voltage > 3.8) && (voltage < 7.0))
            {
                _hio_module_battery.format = HIO_MODULE_BATTERY_FORMAT_STANDARD;
                _hio_module_battery.level_low_threshold = _HIO_MODULE_BATTERY_STANDATD_DEFAULT_LEVEL_LOW;
                _hio_module_battery.level_critical_threshold = _HIO_MODULE_BATTERY_DEFAULT_DEFAULT_LEVEL_CRITICAL;
                _hio_module_battery.valid_min = 3.8;
                _hio_module_battery.valid_max = 7.0;
            }
            else
            {
                _hio_module_battery.format = HIO_MODULE_BATTERY_FORMAT_MINI;
                _hio_module_battery.level_low_threshold = _HIO_MODULE_BATTERY_MINI_DEFAULT_LEVEL_LOW;
                _hio_module_battery.level_critical_threshold = _HIO_MODULE_BATTERY_MINI_DEFAULT_LEVEL_CRITICAL;
                _hio_module_battery.valid_min = 1.8;
                _hio_module_battery.valid_max = 3.8;
            }

            _hio_module_battery.state = HIO_MODULE_STATE_MEASURE;

            if (_hio_module_battery.measurement_active)
            {
                hio_scheduler_plan_current_now();
            }
            else
            {
                hio_scheduler_plan_current_absolute(_hio_module_battery.next_update_start);
            }

            break;
        }
        case HIO_MODULE_STATE_MEASURE:
        {
            if (_hio_module_battery.update_interval == HIO_TICK_INFINITY)
            {
                _hio_module_battery.next_update_start = HIO_TICK_INFINITY;
            }
            else
            {
                _hio_module_battery.next_update_start = hio_tick_get() + _hio_module_battery.update_interval;
            }

            _hio_module_battery_measurement(ENABLE);

            hio_adc_set_event_handler(HIO_ADC_CHANNEL_A0, _hio_module_battery_adc_event_handler, NULL);

            hio_adc_async_measure(HIO_ADC_CHANNEL_A0);

            _hio_module_battery.state = HIO_MODULE_STATE_READ;

            break;
        }
        case HIO_MODULE_STATE_READ:
        {
            if (_hio_module_battery.format == HIO_MODULE_BATTERY_FORMAT_MINI)
            {
                _hio_module_battery.voltage = _HIO_MODULE_BATTERY_MINI_CALIBRATION(_HIO_MODULE_BATTERY_MINI_RESULT_TO_VOLTAGE(_hio_module_battery.adc_value));
            }
            else
            {
                _hio_module_battery.voltage = _HIO_MODULE_BATTERY_STANDARD_CALIBRATION(_HIO_MODULE_BATTERY_STANDARD_RESULT_TO_VOLTAGE(_hio_module_battery.adc_value));
            }

            _hio_module_battery.measurement_active = false;

            if ((_hio_module_battery.voltage < _hio_module_battery.valid_min) || (_hio_module_battery.voltage > _hio_module_battery.valid_max))
            {
                _hio_module_battery.voltage = NAN;

                _hio_module_battery.state = HIO_MODULE_STATE_DETECT_PRESENT;

                hio_scheduler_plan_current_absolute(_hio_module_battery.next_update_start);

                if (_hio_module_battery.event_handler != NULL)
                {
                    _hio_module_battery.event_handler(HIO_MODULE_BATTERY_EVENT_ERROR, _hio_module_battery.event_param);
                }

                return;
            }

            _hio_module_battery.state = HIO_MODULE_STATE_UPDATE;

            goto start;
        }
        case HIO_MODULE_STATE_UPDATE:
        {
            if (_hio_module_battery.event_handler != NULL)
            {
                // Notify event based on calculated percentage
                if (_hio_module_battery.voltage <= _hio_module_battery.level_critical_threshold)
                {
                    _hio_module_battery.event_handler(HIO_MODULE_BATTERY_EVENT_LEVEL_CRITICAL, _hio_module_battery.event_param);
                }
                else if (_hio_module_battery.voltage <= _hio_module_battery.level_low_threshold)
                {
                    _hio_module_battery.event_handler(HIO_MODULE_BATTERY_EVENT_LEVEL_LOW, _hio_module_battery.event_param);
                }

                _hio_module_battery.event_handler(HIO_MODULE_BATTERY_EVENT_UPDATE, _hio_module_battery.event_param);
            }

            _hio_module_battery.state = HIO_MODULE_STATE_MEASURE;

            hio_scheduler_plan_current_absolute(_hio_module_battery.next_update_start);

            break;
        }
        default:
        {
            return;
        }
    }
}

static void _hio_module_battery_adc_event_handler(hio_adc_channel_t channel, hio_adc_event_t event, void *param)
{
    (void) channel;
    (void) param;

    if (event == HIO_ADC_EVENT_DONE)
    {

        if (!hio_adc_async_get_voltage(HIO_ADC_CHANNEL_A0, &_hio_module_battery.adc_value))
        {
            _hio_module_battery.adc_value = NAN;
        }

        _hio_module_battery_measurement(DISABLE);

        hio_scheduler_plan_now(_hio_module_battery.task_id);
    }
}

static void _hio_module_battery_measurement(int state)
{
    if (_hio_module_battery.format == HIO_MODULE_BATTERY_FORMAT_MINI)
    {
        hio_gpio_set_mode(HIO_GPIO_P1, state == ENABLE ? HIO_GPIO_MODE_OUTPUT : HIO_GPIO_MODE_ANALOG);
    }
    else
    {
        hio_gpio_set_output(HIO_GPIO_P1, state);
    }

    if (state == ENABLE)
    {
        hio_timer_start();

        hio_timer_delay(100);

        hio_timer_stop();
    }
}

