#include <hio_module_climate.h>
#include <hio_tmp112.h>
#include <hio_sht20.h>
#include <hio_opt3001.h>
#include <hio_mpl3115a2.h>
#include <hio_sht30.h>

static struct
{
    void (*event_handler)(hio_module_climate_event_t, void *);
    void *event_param;
    hio_module_climate_revision_t revision;
    hio_tmp112_t tmp112;
    hio_opt3001_t opt3001;
    hio_mpl3115a2_t mpl3115a2;
    union {
        hio_sht20_t _20;
        hio_sht30_t _30;
    } sht;
    struct {
        hio_tick_t thermometer;
        hio_tick_t hygrometer;
    } update_interval;

} _hio_module_climate;

static void _hio_module_climate_tmp112_event_handler(hio_tmp112_t *self, hio_tmp112_event_t event, void *event_param);

static void _hio_module_climate_sht20_event_handler(hio_sht20_t *self, hio_sht20_event_t event, void *event_param);

static void _hio_module_climate_sht30_event_handler(hio_sht30_t *self, hio_sht30_event_t event, void *event_param);

static void _hio_module_climate_opt3001_event_handler(hio_opt3001_t *self, hio_opt3001_event_t event, void *event_param);

static void _hio_module_climate_mpl3115a2_event_handler(hio_mpl3115a2_t *self, hio_mpl3115a2_event_t event, void *event_param);

void hio_module_climate_init(void)
{
    memset(&_hio_module_climate, 0, sizeof(_hio_module_climate));

    _hio_module_climate.update_interval.thermometer = HIO_TICK_INFINITY;
    _hio_module_climate.update_interval.hygrometer = HIO_TICK_INFINITY;

    hio_tmp112_init(&_hio_module_climate.tmp112, HIO_I2C_I2C0, 0x48);
    hio_tmp112_set_event_handler(&_hio_module_climate.tmp112, _hio_module_climate_tmp112_event_handler, NULL);

    hio_sht20_init(&_hio_module_climate.sht._20, HIO_I2C_I2C0, 0x40);
    hio_sht20_set_event_handler(&_hio_module_climate.sht._20, _hio_module_climate_sht20_event_handler, NULL);

    hio_opt3001_init(&_hio_module_climate.opt3001, HIO_I2C_I2C0, 0x44);
    hio_opt3001_set_event_handler(&_hio_module_climate.opt3001, _hio_module_climate_opt3001_event_handler, NULL);

    hio_mpl3115a2_init(&_hio_module_climate.mpl3115a2, HIO_I2C_I2C0, 0x60);
    hio_mpl3115a2_set_event_handler(&_hio_module_climate.mpl3115a2, _hio_module_climate_mpl3115a2_event_handler, NULL);
}

void hio_module_climate_set_event_handler(void (*event_handler)(hio_module_climate_event_t, void *), void *event_param)
{
    _hio_module_climate.event_handler = event_handler;
    _hio_module_climate.event_param = event_param;
}

void hio_module_climate_set_update_interval_all_sensors(hio_tick_t interval)
{
    _hio_module_climate.update_interval.thermometer = interval;
    _hio_module_climate.update_interval.hygrometer = interval;

    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        hio_tmp112_set_update_interval(&_hio_module_climate.tmp112, interval);
        hio_sht20_set_update_interval(&_hio_module_climate.sht._20, interval);
    }
    else
    {
        hio_sht30_set_update_interval(&_hio_module_climate.sht._30, interval);
    }
    hio_opt3001_set_update_interval(&_hio_module_climate.opt3001, interval);
    hio_mpl3115a2_set_update_interval(&_hio_module_climate.mpl3115a2, interval);
}

void hio_module_climate_set_update_interval_thermometer(hio_tick_t interval)
{
    _hio_module_climate.update_interval.thermometer = interval;
    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        hio_tmp112_set_update_interval(&_hio_module_climate.tmp112, interval);
    }
}

void hio_module_climate_set_update_interval_hygrometer(hio_tick_t interval)
{
    _hio_module_climate.update_interval.hygrometer = interval;
    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        hio_sht20_set_update_interval(&_hio_module_climate.sht._20, interval);
    }
    else
    {
        hio_sht30_set_update_interval(&_hio_module_climate.sht._30, interval);
    }
}

void hio_module_climate_set_update_interval_lux_meter(hio_tick_t interval)
{
    hio_opt3001_set_update_interval(&_hio_module_climate.opt3001, interval);
}

void hio_module_climate_set_update_interval_barometer(hio_tick_t interval)
{
    hio_mpl3115a2_set_update_interval(&_hio_module_climate.mpl3115a2, interval);
}

bool hio_module_climate_measure_all_sensors(void)
{
    bool ret = true;

    ret &= hio_tmp112_measure(&_hio_module_climate.tmp112);

    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        ret &= hio_sht20_measure(&_hio_module_climate.sht._20);
    }
    else
    {
        ret &= hio_sht30_measure(&_hio_module_climate.sht._30);
    }

    ret &= hio_opt3001_measure(&_hio_module_climate.opt3001);

    ret &= hio_mpl3115a2_measure(&_hio_module_climate.mpl3115a2);

    return ret;
}

bool hio_module_climate_measure_thermometer(void)
{
    return hio_tmp112_measure(&_hio_module_climate.tmp112);
}

bool hio_module_climate_measure_hygrometer(void)
{
    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        return hio_sht20_measure(&_hio_module_climate.sht._20);
    }
    return hio_sht30_measure(&_hio_module_climate.sht._30);
}

bool hio_module_climate_measure_lux_meter(void)
{
    return hio_opt3001_measure(&_hio_module_climate.opt3001);
}

bool hio_module_climate_measure_barometer(void)
{
    return hio_mpl3115a2_measure(&_hio_module_climate.mpl3115a2);
}

bool hio_module_climate_get_temperature_celsius(float *celsius)
{
    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        return hio_tmp112_get_temperature_celsius(&_hio_module_climate.tmp112, celsius);
    }
    return hio_sht30_get_temperature_celsius(&_hio_module_climate.sht._30, celsius);
}

bool hio_module_climate_get_temperature_fahrenheit(float *fahrenheit)
{
    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        return hio_tmp112_get_temperature_fahrenheit(&_hio_module_climate.tmp112, fahrenheit);
    }
    return hio_sht30_get_temperature_fahrenheit(&_hio_module_climate.sht._30, fahrenheit);
}

bool hio_module_climate_get_temperature_kelvin(float *kelvin)
{
    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        return hio_tmp112_get_temperature_kelvin(&_hio_module_climate.tmp112, kelvin);
    }
    return hio_sht30_get_temperature_kelvin(&_hio_module_climate.sht._30, kelvin);
}

bool hio_module_climate_get_humidity_percentage(float *percentage)
{
    if (_hio_module_climate.revision == HIO_MODULE_CLIMATE_REVISION_R1)
    {
        return hio_sht20_get_humidity_percentage(&_hio_module_climate.sht._20, percentage);
    }
    return hio_sht30_get_humidity_percentage(&_hio_module_climate.sht._30, percentage);
}

bool hio_module_climate_get_illuminance_lux(float *lux)
{
    return hio_opt3001_get_illuminance_lux(&_hio_module_climate.opt3001, lux);
}

bool hio_module_climate_get_altitude_meter(float *meter)
{
    return hio_mpl3115a2_get_altitude_meter(&_hio_module_climate.mpl3115a2, meter);
}

bool hio_module_climate_get_pressure_pascal(float *pascal)
{
    return hio_mpl3115a2_get_pressure_pascal(&_hio_module_climate.mpl3115a2, pascal);
}

static void _hio_module_climate_tmp112_event_handler(hio_tmp112_t *self, hio_tmp112_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_hio_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == HIO_TMP112_EVENT_UPDATE)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER, _hio_module_climate.event_param);
    }
    else if (event == HIO_TMP112_EVENT_ERROR)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_ERROR_THERMOMETER, _hio_module_climate.event_param);
    }
}

static void _hio_module_climate_sht20_event_handler(hio_sht20_t *self, hio_sht20_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_hio_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == HIO_SHT20_EVENT_UPDATE)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER, _hio_module_climate.event_param);
    }
    else if (event == HIO_SHT20_EVENT_ERROR)
    {
        hio_tmp112_deinit(&_hio_module_climate.tmp112);
        hio_sht20_deinit(&_hio_module_climate.sht._20);

        _hio_module_climate.revision = HIO_MODULE_CLIMATE_REVISION_R2;

        hio_sht30_init(&_hio_module_climate.sht._30, HIO_I2C_I2C0, 0x45);
        hio_sht30_set_event_handler(&_hio_module_climate.sht._30, _hio_module_climate_sht30_event_handler, NULL);
        hio_sht30_set_update_interval(&_hio_module_climate.sht._30, _hio_module_climate.update_interval.hygrometer);

        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_ERROR_HYGROMETER, _hio_module_climate.event_param);
    }
}

static void _hio_module_climate_sht30_event_handler(hio_sht30_t *self, hio_sht30_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_hio_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == HIO_SHT30_EVENT_UPDATE)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER, _hio_module_climate.event_param);
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER, _hio_module_climate.event_param);
    }
    else if (event == HIO_SHT30_EVENT_ERROR)
    {
        hio_sht30_deinit(&_hio_module_climate.sht._30);

        _hio_module_climate.revision = HIO_MODULE_CLIMATE_REVISION_R1;

        hio_sht20_init(&_hio_module_climate.sht._20, HIO_I2C_I2C0, 0x40);
        hio_sht20_set_event_handler(&_hio_module_climate.sht._20, _hio_module_climate_sht20_event_handler, NULL);
        hio_sht20_set_update_interval(&_hio_module_climate.sht._20, _hio_module_climate.update_interval.hygrometer);

        hio_tmp112_init(&_hio_module_climate.tmp112, HIO_I2C_I2C0, 0x48);
        hio_tmp112_set_event_handler(&_hio_module_climate.tmp112, _hio_module_climate_tmp112_event_handler, NULL);
        hio_tmp112_set_update_interval(&_hio_module_climate.tmp112 , _hio_module_climate.update_interval.thermometer);

        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_ERROR_THERMOMETER, _hio_module_climate.event_param);
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_ERROR_HYGROMETER, _hio_module_climate.event_param);
    }
}

static void _hio_module_climate_opt3001_event_handler(hio_opt3001_t *self, hio_opt3001_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_hio_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == HIO_OPT3001_EVENT_UPDATE)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_UPDATE_LUX_METER, _hio_module_climate.event_param);
    }
    else if (event == HIO_OPT3001_EVENT_ERROR)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_ERROR_LUX_METER, _hio_module_climate.event_param);
    }
}

static void _hio_module_climate_mpl3115a2_event_handler(hio_mpl3115a2_t *self, hio_mpl3115a2_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_hio_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == HIO_MPL3115A2_EVENT_UPDATE)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER, _hio_module_climate.event_param);
    }
    else if (event == HIO_MPL3115A2_EVENT_ERROR)
    {
        _hio_module_climate.event_handler(HIO_MODULE_CLIMATE_EVENT_ERROR_BAROMETER, _hio_module_climate.event_param);
    }
}
