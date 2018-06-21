#include <bc_soil_sensor.h>

static void _bc_soil_sensor_tmp112_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param);

static void _bc_soil_sensor_cy8cmbr3102_event_handler(bc_cy8cmbr3102_t *self, bc_cy8cmbr3102_event_t event, void *event_param);

void bc_soil_sensor_init(bc_soil_sensor_t *self, uint64_t device_id)
{
    memset(self, 0, sizeof(*self));

    bc_i2c_init(BC_I2C_I2C_1W, BC_I2C_SPEED_100_KHZ);

    // TODO:
    // bc_ds28e17_init(&self->ds28e17, BC_GPIO_P5, device_id);
    //
    // bc_ds28e17_set_speed(&self->ds28e17, BC_I2C_SPEED_100_KHZ);

    bc_tmp112_init(&self->_tmp112, BC_I2C_I2C_1W, 0x48);

    bc_tmp112_set_event_handler(&self->_tmp112, _bc_soil_sensor_tmp112_event_handler, self);

    bc_cy8cmbr3102_init(&self->_cy8cmbr3102, BC_I2C_I2C_1W, 0x37);

    bc_cy8cmbr3102_set_event_handler(&self->_cy8cmbr3102, _bc_soil_sensor_cy8cmbr3102_event_handler, self);
}

void bc_soil_sensor_set_event_handler(bc_soil_sensor_t *self, void (*event_handler)(bc_soil_sensor_t *, bc_soil_sensor_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_soil_sensor_set_update_interval_all_sensors(bc_soil_sensor_t *self, bc_tick_t interval)
{
    bc_tmp112_set_update_interval(&self->_tmp112, interval);
    bc_cy8cmbr3102_set_scan_interval(&self->_cy8cmbr3102, interval);
}

void bc_soil_sensor_set_update_interval_thermometer(bc_soil_sensor_t *self, bc_tick_t interval)
{
    bc_tmp112_set_update_interval(&self->_tmp112, interval);
}

void bc_soil_sensor_set_update_interval_moisture_sesor(bc_soil_sensor_t *self, bc_tick_t interval)
{
    bc_cy8cmbr3102_set_scan_interval(&self->_cy8cmbr3102, interval);
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

bool bc_soil_sensor_get_moisture(bc_soil_sensor_t *self, float *moisture)
{
    // TODO

    (void) self;

    *moisture = NAN;

    return false;
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

static void _bc_soil_sensor_cy8cmbr3102_event_handler(bc_cy8cmbr3102_t *self, bc_cy8cmbr3102_event_t event, void *event_param)
{
    (void) self;

    bc_soil_sensor_t *soil_sensor = (bc_soil_sensor_t *) event_param;

    if (soil_sensor->_event_handler == NULL)
    {
        return;
    }

    if (event == BC_CY8CMBR3102_EVENT_ERROR)
    {
        soil_sensor->_event_handler(soil_sensor, BC_SOIL_SENSOR_EVENT_ERROR_MOISTURE, soil_sensor->_event_param);
    }
}
