#include <twr_tag_humidity.h>

static void _twr_tag_humidity_event_handler_hts221(twr_hts221_t *child, twr_hts221_event_t event, void *event_param);

static void _twr_tag_humidity_event_handler_hdc2080(twr_hdc2080_t *child, twr_hdc2080_event_t event, void *event_param);

static void _twr_tag_humidity_event_handler_sht20(twr_sht20_t *child, twr_sht20_event_t event, void *event_param);

static void _twr_tag_humidity_event_handler_sht30(twr_sht30_t *child, twr_sht30_event_t event, void *event_param);

void twr_tag_humidity_init(twr_tag_humidity_t *self, twr_tag_humidity_revision_t revision, twr_i2c_channel_t i2c_channel, twr_tag_humidity_i2c_address_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_revision = revision;

    if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R1)
    {
        twr_hts221_init(&self->_sensor.hts221, i2c_channel, 0x5f);

        twr_hts221_set_event_handler(&self->_sensor.hts221, _twr_tag_humidity_event_handler_hts221, self);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R2)
    {
        twr_hdc2080_init(&self->_sensor.hdc2080, i2c_channel, i2c_address == TWR_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT ? 0x40 : 0x41);

        twr_hdc2080_set_event_handler(&self->_sensor.hdc2080, _twr_tag_humidity_event_handler_hdc2080, self);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R3)
    {
        twr_sht20_init(&self->_sensor.sht20, i2c_channel, 0x40);

        twr_sht20_set_event_handler(&self->_sensor.sht20, _twr_tag_humidity_event_handler_sht20, self);
    }
    else
    {
        twr_sht30_init(&self->_sensor.sht30, i2c_channel, i2c_address == TWR_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT ? 0x44 : 0x45);

        twr_sht30_set_event_handler(&self->_sensor.sht30, _twr_tag_humidity_event_handler_sht30, self);
    }
}

void twr_tag_humidity_set_event_handler(twr_tag_humidity_t *self, void (*event_handler)(twr_tag_humidity_t *, twr_tag_humidity_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_tag_humidity_set_update_interval(twr_tag_humidity_t *self, twr_tick_t interval)
{
    if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R1)
    {
        twr_hts221_set_update_interval(&self->_sensor.hts221, interval);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R2)
    {
        twr_hdc2080_set_update_interval(&self->_sensor.hdc2080, interval);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R3)
    {
        twr_sht20_set_update_interval(&self->_sensor.sht20, interval);
    }
    else
    {
        twr_sht30_set_update_interval(&self->_sensor.sht30, interval);
    }
}

bool twr_tag_humidity_measure(twr_tag_humidity_t *self)
{
    if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R1)
    {
        return twr_hts221_measure(&self->_sensor.hts221);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R2)
    {
        return twr_hdc2080_measure(&self->_sensor.hdc2080);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R3)
    {
        return twr_sht20_measure(&self->_sensor.sht20);
    }
    else
    {
        return twr_sht30_measure(&self->_sensor.sht30);
    }
}

bool twr_tag_humidity_get_temperature_raw(twr_tag_humidity_t *self, uint16_t *raw)
{
    if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R1)
    {
        return false;
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R2)
    {
        return twr_hdc2080_get_temperature_raw(&self->_sensor.hdc2080, raw);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R3)
    {
        return twr_sht20_get_temperature_raw(&self->_sensor.sht20, raw);
    }
    else
    {
        return twr_sht30_get_temperature_raw(&self->_sensor.sht30, raw);
    }
}

bool twr_tag_humidity_get_temperature_celsius(twr_tag_humidity_t *self, float *celsius)
{
    if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R1)
    {
        return false;
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R2)
    {
        return twr_hdc2080_get_temperature_celsius(&self->_sensor.hdc2080, celsius);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R3)
    {
        return twr_sht20_get_temperature_celsius(&self->_sensor.sht20, celsius);
    }
    else
    {
        return twr_sht30_get_temperature_celsius(&self->_sensor.sht30, celsius);
    }
}

bool twr_tag_humidity_get_humidity_raw(twr_tag_humidity_t *self, uint16_t *raw)
{
    if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R1)
    {
        return false;
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R2)
    {
        return twr_hdc2080_get_humidity_raw(&self->_sensor.hdc2080, raw);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R3)
    {
        return twr_sht20_get_humidity_raw(&self->_sensor.sht20, raw);
    }
    else
    {
        return twr_sht30_get_humidity_raw(&self->_sensor.sht30, raw);
    }
}

bool twr_tag_humidity_get_humidity_percentage(twr_tag_humidity_t *self, float *percentage)
{
    if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R1)
    {
        return twr_hts221_get_humidity_percentage(&self->_sensor.hts221, percentage);
    }
    else  if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R2)
    {
        return twr_hdc2080_get_humidity_percentage(&self->_sensor.hdc2080, percentage);
    }
    else if (self->_revision == TWR_TAG_HUMIDITY_REVISION_R3)
    {
        return twr_sht20_get_humidity_percentage(&self->_sensor.sht20, percentage);
    }
    else
    {
        return twr_sht30_get_humidity_percentage(&self->_sensor.sht30, percentage);
    }
}

static void _twr_tag_humidity_event_handler_hts221(twr_hts221_t *child, twr_hts221_event_t event, void *event_param)
{
    (void) child;

    twr_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == TWR_HTS221_EVENT_UPDATE)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == TWR_HTS221_EVENT_ERROR)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}

static void _twr_tag_humidity_event_handler_hdc2080(twr_hdc2080_t *child, twr_hdc2080_event_t event, void *event_param)
{
    (void) child;

    twr_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == TWR_HDC2080_EVENT_UPDATE)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == TWR_HDC2080_EVENT_ERROR)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}

static void _twr_tag_humidity_event_handler_sht20(twr_sht20_t *child, twr_sht20_event_t event, void *event_param)
{
    (void) child;

    twr_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == TWR_SHT20_EVENT_UPDATE)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == TWR_SHT20_EVENT_ERROR)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}

static void _twr_tag_humidity_event_handler_sht30(twr_sht30_t *child, twr_sht30_event_t event, void *event_param)
{
    (void) child;

    twr_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == TWR_SHT30_EVENT_UPDATE)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == TWR_SHT30_EVENT_ERROR)
        {
            self->_event_handler(self, TWR_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}
