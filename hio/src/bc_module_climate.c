#include <bc_module_climate.h>
#include <bc_tmp112.h>
#include <bc_sht20.h>
#include <bc_opt3001.h>
#include <bc_mpl3115a2.h>
#include <bc_sht30.h>

static struct
{
    void (*event_handler)(bc_module_climate_event_t, void *);
    void *event_param;
    bc_module_climate_revision_t revision;
    bc_tmp112_t tmp112;
    bc_opt3001_t opt3001;
    bc_mpl3115a2_t mpl3115a2;
    union {
        bc_sht20_t _20;
        bc_sht30_t _30;
    } sht;
    struct {
        bc_tick_t thermometer;
        bc_tick_t hygrometer;
    } update_interval;

} _bc_module_climate;

static void _bc_module_climate_tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param);

static void _bc_module_climate_sht20_event_handler(bc_sht20_t *self, bc_sht20_event_t event, void *event_param);

static void _bc_module_climate_sht30_event_handler(bc_sht30_t *self, bc_sht30_event_t event, void *event_param);

static void _bc_module_climate_opt3001_event_handler(bc_opt3001_t *self, bc_opt3001_event_t event, void *event_param);

static void _bc_module_climate_mpl3115a2_event_handler(bc_mpl3115a2_t *self, bc_mpl3115a2_event_t event, void *event_param);

void bc_module_climate_init(void)
{
    memset(&_bc_module_climate, 0, sizeof(_bc_module_climate));

    _bc_module_climate.update_interval.thermometer = BC_TICK_INFINITY;
    _bc_module_climate.update_interval.hygrometer = BC_TICK_INFINITY;

    bc_tmp112_init(&_bc_module_climate.tmp112, BC_I2C_I2C0, 0x48);
    bc_tmp112_set_event_handler(&_bc_module_climate.tmp112, _bc_module_climate_tmp112_event_handler, NULL);

    bc_sht20_init(&_bc_module_climate.sht._20, BC_I2C_I2C0, 0x40);
    bc_sht20_set_event_handler(&_bc_module_climate.sht._20, _bc_module_climate_sht20_event_handler, NULL);

    bc_opt3001_init(&_bc_module_climate.opt3001, BC_I2C_I2C0, 0x44);
    bc_opt3001_set_event_handler(&_bc_module_climate.opt3001, _bc_module_climate_opt3001_event_handler, NULL);

    bc_mpl3115a2_init(&_bc_module_climate.mpl3115a2, BC_I2C_I2C0, 0x60);
    bc_mpl3115a2_set_event_handler(&_bc_module_climate.mpl3115a2, _bc_module_climate_mpl3115a2_event_handler, NULL);
}

void bc_module_climate_set_event_handler(void (*event_handler)(bc_module_climate_event_t, void *), void *event_param)
{
    _bc_module_climate.event_handler = event_handler;
    _bc_module_climate.event_param = event_param;
}

void bc_module_climate_set_update_interval_all_sensors(bc_tick_t interval)
{
    _bc_module_climate.update_interval.thermometer = interval;
    _bc_module_climate.update_interval.hygrometer = interval;

    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        bc_tmp112_set_update_interval(&_bc_module_climate.tmp112, interval);
        bc_sht20_set_update_interval(&_bc_module_climate.sht._20, interval);
    }
    else
    {
        bc_sht30_set_update_interval(&_bc_module_climate.sht._30, interval);
    }
    bc_opt3001_set_update_interval(&_bc_module_climate.opt3001, interval);
    bc_mpl3115a2_set_update_interval(&_bc_module_climate.mpl3115a2, interval);
}

void bc_module_climate_set_update_interval_thermometer(bc_tick_t interval)
{
    _bc_module_climate.update_interval.thermometer = interval;
    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        bc_tmp112_set_update_interval(&_bc_module_climate.tmp112, interval);
    }
}

void bc_module_climate_set_update_interval_hygrometer(bc_tick_t interval)
{
    _bc_module_climate.update_interval.hygrometer = interval;
    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        bc_sht20_set_update_interval(&_bc_module_climate.sht._20, interval);
    }
    else
    {
        bc_sht30_set_update_interval(&_bc_module_climate.sht._30, interval);
    }
}

void bc_module_climate_set_update_interval_lux_meter(bc_tick_t interval)
{
    bc_opt3001_set_update_interval(&_bc_module_climate.opt3001, interval);
}

void bc_module_climate_set_update_interval_barometer(bc_tick_t interval)
{
    bc_mpl3115a2_set_update_interval(&_bc_module_climate.mpl3115a2, interval);
}

bool bc_module_climate_measure_all_sensors(void)
{
    bool ret = true;

    ret &= bc_tmp112_measure(&_bc_module_climate.tmp112);

    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        ret &= bc_sht20_measure(&_bc_module_climate.sht._20);
    }
    else
    {
        ret &= bc_sht30_measure(&_bc_module_climate.sht._30);
    }

    ret &= bc_opt3001_measure(&_bc_module_climate.opt3001);

    ret &= bc_mpl3115a2_measure(&_bc_module_climate.mpl3115a2);

    return ret;
}

bool bc_module_climate_measure_thermometer(void)
{
    return bc_tmp112_measure(&_bc_module_climate.tmp112);
}

bool bc_module_climate_measure_hygrometer(void)
{
    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        return bc_sht20_measure(&_bc_module_climate.sht._20);
    }
    return bc_sht30_measure(&_bc_module_climate.sht._30);
}

bool bc_module_climate_measure_lux_meter(void)
{
    return bc_opt3001_measure(&_bc_module_climate.opt3001);
}

bool bc_module_climate_measure_barometer(void)
{
    return bc_mpl3115a2_measure(&_bc_module_climate.mpl3115a2);
}

bool bc_module_climate_get_temperature_celsius(float *celsius)
{
    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        return bc_tmp112_get_temperature_celsius(&_bc_module_climate.tmp112, celsius);
    }
    return bc_sht30_get_temperature_celsius(&_bc_module_climate.sht._30, celsius);
}

bool bc_module_climate_get_temperature_fahrenheit(float *fahrenheit)
{
    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        return bc_tmp112_get_temperature_fahrenheit(&_bc_module_climate.tmp112, fahrenheit);
    }
    return bc_sht30_get_temperature_fahrenheit(&_bc_module_climate.sht._30, fahrenheit);
}

bool bc_module_climate_get_temperature_kelvin(float *kelvin)
{
    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        return bc_tmp112_get_temperature_kelvin(&_bc_module_climate.tmp112, kelvin);
    }
    return bc_sht30_get_temperature_kelvin(&_bc_module_climate.sht._30, kelvin);
}

bool bc_module_climate_get_humidity_percentage(float *percentage)
{
    if (_bc_module_climate.revision == BC_MODULE_CLIMATE_REVISION_R1)
    {
        return bc_sht20_get_humidity_percentage(&_bc_module_climate.sht._20, percentage);
    }
    return bc_sht30_get_humidity_percentage(&_bc_module_climate.sht._30, percentage);
}

bool bc_module_climate_get_illuminance_lux(float *lux)
{
    return bc_opt3001_get_illuminance_lux(&_bc_module_climate.opt3001, lux);
}

bool bc_module_climate_get_altitude_meter(float *meter)
{
    return bc_mpl3115a2_get_altitude_meter(&_bc_module_climate.mpl3115a2, meter);
}

bool bc_module_climate_get_pressure_pascal(float *pascal)
{
    return bc_mpl3115a2_get_pressure_pascal(&_bc_module_climate.mpl3115a2, pascal);
}

static void _bc_module_climate_tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_bc_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == BC_TMP112_EVENT_UPDATE)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER, _bc_module_climate.event_param);
    }
    else if (event == BC_TMP112_EVENT_ERROR)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_ERROR_THERMOMETER, _bc_module_climate.event_param);
    }
}

static void _bc_module_climate_sht20_event_handler(bc_sht20_t *self, bc_sht20_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_bc_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == BC_SHT20_EVENT_UPDATE)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER, _bc_module_climate.event_param);
    }
    else if (event == BC_SHT20_EVENT_ERROR)
    {
        bc_tmp112_deinit(&_bc_module_climate.tmp112);
        bc_sht20_deinit(&_bc_module_climate.sht._20);

        _bc_module_climate.revision = BC_MODULE_CLIMATE_REVISION_R2;

        bc_sht30_init(&_bc_module_climate.sht._30, BC_I2C_I2C0, 0x45);
        bc_sht30_set_event_handler(&_bc_module_climate.sht._30, _bc_module_climate_sht30_event_handler, NULL);
        bc_sht30_set_update_interval(&_bc_module_climate.sht._30, _bc_module_climate.update_interval.hygrometer);

        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_ERROR_HYGROMETER, _bc_module_climate.event_param);
    }
}

static void _bc_module_climate_sht30_event_handler(bc_sht30_t *self, bc_sht30_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_bc_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == BC_SHT30_EVENT_UPDATE)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER, _bc_module_climate.event_param);
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER, _bc_module_climate.event_param);
    }
    else if (event == BC_SHT30_EVENT_ERROR)
    {
        bc_sht30_deinit(&_bc_module_climate.sht._30);

        _bc_module_climate.revision = BC_MODULE_CLIMATE_REVISION_R1;

        bc_sht20_init(&_bc_module_climate.sht._20, BC_I2C_I2C0, 0x40);
        bc_sht20_set_event_handler(&_bc_module_climate.sht._20, _bc_module_climate_sht20_event_handler, NULL);
        bc_sht20_set_update_interval(&_bc_module_climate.sht._20, _bc_module_climate.update_interval.hygrometer);

        bc_tmp112_init(&_bc_module_climate.tmp112, BC_I2C_I2C0, 0x48);
        bc_tmp112_set_event_handler(&_bc_module_climate.tmp112, _bc_module_climate_tmp112_event_handler, NULL);
        bc_tmp112_set_update_interval(&_bc_module_climate.tmp112 , _bc_module_climate.update_interval.thermometer);

        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_ERROR_THERMOMETER, _bc_module_climate.event_param);
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_ERROR_HYGROMETER, _bc_module_climate.event_param);
    }
}

static void _bc_module_climate_opt3001_event_handler(bc_opt3001_t *self, bc_opt3001_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_bc_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == BC_OPT3001_EVENT_UPDATE)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_UPDATE_LUX_METER, _bc_module_climate.event_param);
    }
    else if (event == BC_OPT3001_EVENT_ERROR)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_ERROR_LUX_METER, _bc_module_climate.event_param);
    }
}

static void _bc_module_climate_mpl3115a2_event_handler(bc_mpl3115a2_t *self, bc_mpl3115a2_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_bc_module_climate.event_handler == NULL)
    {
        return;
    }

    if (event == BC_MPL3115A2_EVENT_UPDATE)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER, _bc_module_climate.event_param);
    }
    else if (event == BC_MPL3115A2_EVENT_ERROR)
    {
        _bc_module_climate.event_handler(BC_MODULE_CLIMATE_EVENT_ERROR_BAROMETER, _bc_module_climate.event_param);
    }
}
