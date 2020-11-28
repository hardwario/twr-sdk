#include <hio_tag_humidity.h>

static void _hio_tag_humidity_event_handler_hts221(hio_hts221_t *child, hio_hts221_event_t event, void *event_param);

static void _hio_tag_humidity_event_handler_hdc2080(hio_hdc2080_t *child, hio_hdc2080_event_t event, void *event_param);

static void _hio_tag_humidity_event_handler_sht20(hio_sht20_t *child, hio_sht20_event_t event, void *event_param);

static void _hio_tag_humidity_event_handler_sht30(hio_sht30_t *child, hio_sht30_event_t event, void *event_param);

void hio_tag_humidity_init(hio_tag_humidity_t *self, hio_tag_humidity_revision_t revision, hio_i2c_channel_t i2c_channel, hio_tag_humidity_i2c_address_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_revision = revision;

    if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R1)
    {
        hio_hts221_init(&self->_sensor.hts221, i2c_channel, 0x5f);

        hio_hts221_set_event_handler(&self->_sensor.hts221, _hio_tag_humidity_event_handler_hts221, self);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R2)
    {
        hio_hdc2080_init(&self->_sensor.hdc2080, i2c_channel, i2c_address == HIO_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT ? 0x40 : 0x41);

        hio_hdc2080_set_event_handler(&self->_sensor.hdc2080, _hio_tag_humidity_event_handler_hdc2080, self);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R3)
    {
        hio_sht20_init(&self->_sensor.sht20, i2c_channel, 0x40);

        hio_sht20_set_event_handler(&self->_sensor.sht20, _hio_tag_humidity_event_handler_sht20, self);
    }
    else
    {
        hio_sht30_init(&self->_sensor.sht30, i2c_channel, i2c_address == HIO_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT ? 0x44 : 0x45);

        hio_sht30_set_event_handler(&self->_sensor.sht30, _hio_tag_humidity_event_handler_sht30, self);
    }
}

void hio_tag_humidity_set_event_handler(hio_tag_humidity_t *self, void (*event_handler)(hio_tag_humidity_t *, hio_tag_humidity_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_tag_humidity_set_update_interval(hio_tag_humidity_t *self, hio_tick_t interval)
{
    if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R1)
    {
        hio_hts221_set_update_interval(&self->_sensor.hts221, interval);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R2)
    {
        hio_hdc2080_set_update_interval(&self->_sensor.hdc2080, interval);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R3)
    {
        hio_sht20_set_update_interval(&self->_sensor.sht20, interval);
    }
    else
    {
        hio_sht30_set_update_interval(&self->_sensor.sht30, interval);
    }
}

bool hio_tag_humidity_measure(hio_tag_humidity_t *self)
{
    if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R1)
    {
        return hio_hts221_measure(&self->_sensor.hts221);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R2)
    {
        return hio_hdc2080_measure(&self->_sensor.hdc2080);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R3)
    {
        return hio_sht20_measure(&self->_sensor.sht20);
    }
    else
    {
        return hio_sht30_measure(&self->_sensor.sht30);
    }
}

bool hio_tag_humidity_get_temperature_raw(hio_tag_humidity_t *self, uint16_t *raw)
{
    if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R1)
    {
        return false;
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R2)
    {
        return hio_hdc2080_get_temperature_raw(&self->_sensor.hdc2080, raw);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R3)
    {
        return hio_sht20_get_temperature_raw(&self->_sensor.sht20, raw);
    }
    else
    {
        return hio_sht30_get_temperature_raw(&self->_sensor.sht30, raw);
    }
}

bool hio_tag_humidity_get_temperature_celsius(hio_tag_humidity_t *self, float *celsius)
{
    if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R1)
    {
        return false;
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R2)
    {
        return hio_hdc2080_get_temperature_celsius(&self->_sensor.hdc2080, celsius);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R3)
    {
        return hio_sht20_get_temperature_celsius(&self->_sensor.sht20, celsius);
    }
    else
    {
        return hio_sht30_get_temperature_celsius(&self->_sensor.sht30, celsius);
    }
}

bool hio_tag_humidity_get_humidity_raw(hio_tag_humidity_t *self, uint16_t *raw)
{
    if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R1)
    {
        return false;
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R2)
    {
        return hio_hdc2080_get_humidity_raw(&self->_sensor.hdc2080, raw);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R3)
    {
        return hio_sht20_get_humidity_raw(&self->_sensor.sht20, raw);
    }
    else
    {
        return hio_sht30_get_humidity_raw(&self->_sensor.sht30, raw);
    }
}

bool hio_tag_humidity_get_humidity_percentage(hio_tag_humidity_t *self, float *percentage)
{
    if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R1)
    {
        return hio_hts221_get_humidity_percentage(&self->_sensor.hts221, percentage);
    }
    else  if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R2)
    {
        return hio_hdc2080_get_humidity_percentage(&self->_sensor.hdc2080, percentage);
    }
    else if (self->_revision == HIO_TAG_HUMIDITY_REVISION_R3)
    {
        return hio_sht20_get_humidity_percentage(&self->_sensor.sht20, percentage);
    }
    else
    {
        return hio_sht30_get_humidity_percentage(&self->_sensor.sht30, percentage);
    }
}

static void _hio_tag_humidity_event_handler_hts221(hio_hts221_t *child, hio_hts221_event_t event, void *event_param)
{
    (void) child;

    hio_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == HIO_HTS221_EVENT_UPDATE)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == HIO_HTS221_EVENT_ERROR)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}

static void _hio_tag_humidity_event_handler_hdc2080(hio_hdc2080_t *child, hio_hdc2080_event_t event, void *event_param)
{
    (void) child;

    hio_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == HIO_HDC2080_EVENT_UPDATE)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == HIO_HDC2080_EVENT_ERROR)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}

static void _hio_tag_humidity_event_handler_sht20(hio_sht20_t *child, hio_sht20_event_t event, void *event_param)
{
    (void) child;

    hio_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == HIO_SHT20_EVENT_UPDATE)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == HIO_SHT20_EVENT_ERROR)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}

static void _hio_tag_humidity_event_handler_sht30(hio_sht30_t *child, hio_sht30_event_t event, void *event_param)
{
    (void) child;

    hio_tag_humidity_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == HIO_SHT30_EVENT_UPDATE)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_UPDATE, self->_event_param);
        }
        else if (event == HIO_SHT30_EVENT_ERROR)
        {
            self->_event_handler(self, HIO_TAG_HUMIDITY_EVENT_ERROR, self->_event_param);
        }
    }
}
