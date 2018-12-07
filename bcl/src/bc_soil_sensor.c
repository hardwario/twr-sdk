#include <bc_soil_sensor.h>
#include <bc_module_sensor.h>
#include <bc_timer.h>
#include <bc_i2c.h>
#include <bc_system.h>
#include <bc_tick.h>

static void _bc_soil_sensor_tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param);

static void _bc_soil_sensor_zssc3123_event_handler(bc_zssc3123_t *self, bc_zssc3123_event_t event, void *event_param);

void bc_soil_sensor_init(bc_soil_sensor_t *self, uint64_t device_number)
{
    memset(self, 0, sizeof(*self));

    self->_device_number = device_number;

    // TODO:
    // bc_ds28e17_init(&self->ds28e17, BC_GPIO_P5, device_id);
    //
    // bc_ds28e17_set_speed(&self->ds28e17, BC_I2C_SPEED_100_KHZ);

    bc_zssc3123_init(&self->_zssc3123, BC_I2C_I2C_1W, 0x28);

    bc_zssc3123_set_data_fetch_delay(&self->_zssc3123, 12);

    bc_zssc3123_set_event_handler(&self->_zssc3123, _bc_soil_sensor_zssc3123_event_handler, self);

    bc_tmp112_init(&self->_tmp112, BC_I2C_I2C_1W, 0x48);

    bc_tmp112_set_event_handler(&self->_tmp112, _bc_soil_sensor_tmp112_event_handler, self);
}

void bc_soil_sensor_set_event_handler(bc_soil_sensor_t *self, void (*event_handler)(bc_soil_sensor_t *, bc_soil_sensor_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;

    self->_event_param = event_param;
}

void bc_soil_sensor_set_update_interval_all_sensors(bc_soil_sensor_t *self, bc_tick_t interval)
{
    bc_tmp112_set_update_interval(&self->_tmp112, interval);

    bc_zssc3123_set_update_interval(&self->_zssc3123, interval);
}

void bc_soil_sensor_set_update_interval_thermometer(bc_soil_sensor_t *self, bc_tick_t interval)
{
    bc_tmp112_set_update_interval(&self->_tmp112, interval);
}

void bc_soil_sensor_set_update_interval_moisture_sesor(bc_soil_sensor_t *self, bc_tick_t interval)
{
    bc_zssc3123_set_update_interval(&self->_zssc3123, interval);
}

bool bc_soil_sensor_get_temperature_celsius(bc_soil_sensor_t *self, float *celsius)
{
    return bc_tmp112_get_temperature_celsius(&self->_tmp112, celsius);
}

bool bc_soil_sensor_get_temperature_fahrenheit(bc_soil_sensor_t *self, float *fahrenheit)
{
    return bc_tmp112_get_temperature_fahrenheit(&self->_tmp112, fahrenheit);
}

bool bc_soil_sensor_get_temperature_kelvin(bc_soil_sensor_t *self, float *kelvin)
{
    return bc_tmp112_get_temperature_kelvin(&self->_tmp112, kelvin);
}

bool bc_soil_sensor_get_moisture(bc_soil_sensor_t *self, int *moisture)
{
    static const int min = 1700;
    static const int max = 3000;

    uint16_t raw;

    if (!bc_zssc3123_get_raw_cap_data(&self->_zssc3123, &raw))
    {
        return false;
    }

    if (raw < min)
    {
        raw = min;
    }
    else if (raw > max)
    {
        raw = max;
    }

    *moisture = ((raw - min) * 100) / (max - min);

    return true;
}

uint64_t bc_soil_sensor_get_device_number(bc_soil_sensor_t *self)
{
    return self->_device_number;
}

static void _bc_soil_sensor_tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param)
{
    (void) self;

    bc_soil_sensor_t *soil_sensor = (bc_soil_sensor_t *) event_param;

    if (soil_sensor->_event_handler == NULL)
    {
        return;
    }

    if (event == BC_TMP112_EVENT_UPDATE)
    {
        soil_sensor->_event_handler(soil_sensor, BC_SOIL_SENSOR_EVENT_UPDATE_THERMOMETER, soil_sensor->_event_param);
    }
    else if (event == BC_TMP112_EVENT_ERROR)
    {
        soil_sensor->_event_handler(soil_sensor, BC_SOIL_SENSOR_EVENT_ERROR_THERMOMETER, soil_sensor->_event_param);
    }
}

static void _bc_soil_sensor_zssc3123_event_handler(bc_zssc3123_t *self, bc_zssc3123_event_t event, void *event_param)
{
    (void) self;

    bc_soil_sensor_t *soil_sensor = (bc_soil_sensor_t *) event_param;

    if (soil_sensor->_event_handler == NULL)
    {
        return;
    }

    if (event == BC_ZSSC3123_EVENT_UPDATE)
    {
        soil_sensor->_event_handler(soil_sensor, BC_SOIL_SENSOR_EVENT_UPDATE_MOISTURE, soil_sensor->_event_param);
    }
    else if (event == BC_ZSSC3123_EVENT_ERROR)
    {
        soil_sensor->_event_handler(soil_sensor, BC_SOIL_SENSOR_EVENT_ERROR_MOISTURE, soil_sensor->_event_param);
    }
}
