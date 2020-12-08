#include <twr_module_climate.h>
#include <twr_tmp112.h>
#include <twr_sht20.h>
#include <twr_opt3001.h>
#include <twr_mpl3115a2.h>
#include <twr_sht30.h>

static struct
{
    void (*event_handler)(twr_module_climate_event_t, void *);
    void *event_param;
    twr_module_climate_revision_t revision;
    twr_tmp112_t tmp112;
    twr_opt3001_t opt3001;
    twr_mpl3115a2_t mpl3115a2;
    union {
        twr_sht20_t _20;
        twr_sht30_t _30;
    } sht;
    struct {
        twr_tick_t thermometer;
        twr_tick_t hygrometer;
    } update_interval;

} _twr_module_climate;

static void _twr_module_climate_tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param);

static void _twr_module_climate_sht20_event_handler(twr_sht20_t *self, twr_sht20_event_t event, void *event_param);

static void _twr_module_climate_sht30_event_handler(twr_sht30_t *self, twr_sht30_event_t event, void *event_param);

static void _twr_module_climate_opt3001_event_handler(twr_opt3001_t *self, twr_opt3001_event_t event, void *event_param);

static void _twr_module_climate_mpl3115a2_event_handler(twr_mpl3115a2_t *self, twr_mpl3115a2_event_t event, void *event_param);

void twr_module_climate_init(void)
{
    memset(&_twr_module_climate, 0, sizeof(_twr_module_climate));

    _twr_module_climate.update_interval.thermometer = TWR_TICK_INFINITY;
    _twr_module_climate.update_interval.hygrometer = TWR_TICK_INFINITY;

    twr_tmp112_init(&_twr_module_climate.tmp112, TWR_I2C_I2C0, 0x48);
    twr_tmp112_set_event_handler(&_twr_module_climate.tmp112, _twr_module_climate_tmp112_event_handler, NULL);

    twr_sht20_init(&_twr_module_climate.sht._20, TWR_I2C_I2C0, 0x40);
    twr_sht20_set_event_handler(&_twr_module_climate.sht._20, _twr_module_climate_sht20_event_handler, NULL);

    twr_opt3001_init(&_twr_module_climate.opt3001, TWR_I2C_I2C0, 0x44);
    twr_opt3001_set_event_handler(&_twr_module_climate.opt3001, _twr_module_climate_opt3001_event_handler, NULL);

    twr_mpl3115a2_init(&_twr_module_climate.mpl3115a2, TWR_I2C_I2C0, 0x60);
    twr_mpl3115a2_set_event_handler(&_twr_module_climate.mpl3115a2, _twr_module_climate_mpl3115a2_event_handler, NULL);
}

void twr_module_climate_set_event_handler(void (*event_handler)(twr_module_climate_event_t, void *), void *event_param)
{
    _twr_module_climate.event_handler = event_handler;
    _twr_module_climate.event_param = event_param;
}

void twr_module_climate_set_update_interval_all_sensors(twr_tick_t interval)
{
    _twr_module_climate.update_interval.thermometer = interval;
    _twr_module_climate.update_interval.hygrometer = interval;

    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        twr_tmp112_set_update_interval(&_twr_module_climate.tmp112, interval);
        twr_sht20_set_update_interval(&_twr_module_climate.sht._20, interval);
    }
    else
    {
        twr_sht30_set_update_interval(&_twr_module_climate.sht._30, interval);
    }
    twr_opt3001_set_update_interval(&_twr_module_climate.opt3001, interval);
    twr_mpl3115a2_set_update_interval(&_twr_module_climate.mpl3115a2, interval);
}

void twr_module_climate_set_update_interval_thermometer(twr_tick_t interval)
{
    _twr_module_climate.update_interval.thermometer = interval;
    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        twr_tmp112_set_update_interval(&_twr_module_climate.tmp112, interval);
    }
}

void twr_module_climate_set_update_interval_hygrometer(twr_tick_t interval)
{
    _twr_module_climate.update_interval.hygrometer = interval;
    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        twr_sht20_set_update_interval(&_twr_module_climate.sht._20, interval);
    }
    else
    {
        twr_sht30_set_update_interval(&_twr_module_climate.sht._30, interval);
    }
}

void twr_module_climate_set_update_interval_lux_meter(twr_tick_t interval)
{
    twr_opt3001_set_update_interval(&_twr_module_climate.opt3001, interval);
}

void twr_module_climate_set_update_interval_barometer(twr_tick_t interval)
{
    twr_mpl3115a2_set_update_interval(&_twr_module_climate.mpl3115a2, interval);
}

bool twr_module_climate_measure_all_sensors(void)
{
    bool ret = true;

    ret &= twr_tmp112_measure(&_twr_module_climate.tmp112);

    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        ret &= twr_sht20_measure(&_twr_module_climate.sht._20);
    }
    else
    {
        ret &= twr_sht30_measure(&_twr_module_climate.sht._30);
    }

    ret &= twr_opt3001_measure(&_twr_module_climate.opt3001);

    ret &= twr_mpl3115a2_measure(&_twr_module_climate.mpl3115a2);

    return ret;
}

bool twr_module_climate_measure_thermometer(void)
{
    return twr_tmp112_measure(&_twr_module_climate.tmp112);
}

bool twr_module_climate_measure_hygrometer(void)
{
    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        return twr_sht20_measure(&_twr_module_climate.sht._20);
    }
    return twr_sht30_measure(&_twr_module_climate.sht._30);
}

bool twr_module_climate_measure_lux_meter(void)
{
    return twr_opt3001_measure(&_twr_module_climate.opt3001);
}

bool twr_module_climate_measure_barometer(void)
{
    return twr_mpl3115a2_measure(&_twr_module_climate.mpl3115a2);
}

bool twr_module_climate_get_temperature_celsius(float *celsius)
{
    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        return twr_tmp112_get_temperature_celsius(&_twr_module_climate.tmp112, celsius);
    }
    return twr_sht30_get_temperature_celsius(&_twr_module_climate.sht._30, celsius);
}

bool twr_module_climate_get_temperature_fahrenheit(float *fahrenheit)
{
    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        return twr_tmp112_get_temperature_fahrenheit(&_twr_module_climate.tmp112, fahrenheit);
    }
    return twr_sht30_get_temperature_fahrenheit(&_twr_module_climate.sht._30, fahrenheit);
}

bool twr_module_climate_get_temperature_kelvin(float *kelvin)
{
    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        return twr_tmp112_get_temperature_kelvin(&_twr_module_climate.tmp112, kelvin);
    }
    return twr_sht30_get_temperature_kelvin(&_twr_module_climate.sht._30, kelvin);
}

bool twr_module_climate_get_humidity_percentage(float *percentage)
{
    if (_twr_module_climate.revision == TWR_MODULE_CLIMATE_REVISION_R1)
    {
        return twr_sht20_get_humidity_percentage(&_twr_module_climate.sht._20, percentage);
    }
    return twr_sht30_get_humidity_percentage(&_twr_module_climate.sht._30, percentage);
}

bool twr_module_climate_get_illuminance_lux(float *lux)
{
    return twr_opt3001_get_illuminance_lux(&_twr_module_climate.opt3001, lux);
}

bool twr_module_climate_get_altitude_meter(float *meter)
{
    return twr_mpl3115a2_get_altitude_meter(&_twr_module_climate.mpl3115a2, meter);
}

bool twr_module_climate_get_pressure_pascal(float *pascal)
{
    return twr_mpl3115a2_get_pressure_pascal(&_twr_module_climate.mpl3115a2, pascal);
}

static void _twr_module_climate_tmp112_event_handler(twr_tmp112_t *self, twr_tmp112_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_twr_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == TWR_TMP112_EVENT_UPDATE)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER, _twr_module_climate.event_param);
    }
    else if (event == TWR_TMP112_EVENT_ERROR)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_ERROR_THERMOMETER, _twr_module_climate.event_param);
    }
}

static void _twr_module_climate_sht20_event_handler(twr_sht20_t *self, twr_sht20_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_twr_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == TWR_SHT20_EVENT_UPDATE)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER, _twr_module_climate.event_param);
    }
    else if (event == TWR_SHT20_EVENT_ERROR)
    {
        twr_tmp112_deinit(&_twr_module_climate.tmp112);
        twr_sht20_deinit(&_twr_module_climate.sht._20);

        _twr_module_climate.revision = TWR_MODULE_CLIMATE_REVISION_R2;

        twr_sht30_init(&_twr_module_climate.sht._30, TWR_I2C_I2C0, 0x45);
        twr_sht30_set_event_handler(&_twr_module_climate.sht._30, _twr_module_climate_sht30_event_handler, NULL);
        twr_sht30_set_update_interval(&_twr_module_climate.sht._30, _twr_module_climate.update_interval.hygrometer);

        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_ERROR_HYGROMETER, _twr_module_climate.event_param);
    }
}

static void _twr_module_climate_sht30_event_handler(twr_sht30_t *self, twr_sht30_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_twr_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == TWR_SHT30_EVENT_UPDATE)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER, _twr_module_climate.event_param);
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER, _twr_module_climate.event_param);
    }
    else if (event == TWR_SHT30_EVENT_ERROR)
    {
        twr_sht30_deinit(&_twr_module_climate.sht._30);

        _twr_module_climate.revision = TWR_MODULE_CLIMATE_REVISION_R1;

        twr_sht20_init(&_twr_module_climate.sht._20, TWR_I2C_I2C0, 0x40);
        twr_sht20_set_event_handler(&_twr_module_climate.sht._20, _twr_module_climate_sht20_event_handler, NULL);
        twr_sht20_set_update_interval(&_twr_module_climate.sht._20, _twr_module_climate.update_interval.hygrometer);

        twr_tmp112_init(&_twr_module_climate.tmp112, TWR_I2C_I2C0, 0x48);
        twr_tmp112_set_event_handler(&_twr_module_climate.tmp112, _twr_module_climate_tmp112_event_handler, NULL);
        twr_tmp112_set_update_interval(&_twr_module_climate.tmp112 , _twr_module_climate.update_interval.thermometer);

        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_ERROR_THERMOMETER, _twr_module_climate.event_param);
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_ERROR_HYGROMETER, _twr_module_climate.event_param);
    }
}

static void _twr_module_climate_opt3001_event_handler(twr_opt3001_t *self, twr_opt3001_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_twr_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == TWR_OPT3001_EVENT_UPDATE)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_UPDATE_LUX_METER, _twr_module_climate.event_param);
    }
    else if (event == TWR_OPT3001_EVENT_ERROR)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_ERROR_LUX_METER, _twr_module_climate.event_param);
    }
}

static void _twr_module_climate_mpl3115a2_event_handler(twr_mpl3115a2_t *self, twr_mpl3115a2_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_twr_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == TWR_MPL3115A2_EVENT_UPDATE)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER, _twr_module_climate.event_param);
    }
    else if (event == TWR_MPL3115A2_EVENT_ERROR)
    {
        _twr_module_climate.event_handler(TWR_MODULE_CLIMATE_EVENT_ERROR_BAROMETER, _twr_module_climate.event_param);
    }
}
