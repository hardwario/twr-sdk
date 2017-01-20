#ifndef _BC_HUMIDITY_TAG_H
#define _BC_HUMIDITY_TAG_H

#include <bc_hdc2080.h>
#include <bc_hts221.h>

typedef enum
{
    BC_HUMIDITY_TAG_DEVICE_HTS221 = 0,
    BC_HUMIDITY_TAG_DEVICE_HDC2080

} bc_humidity_tag_device_t;

typedef enum
{
    BC_HUMIDITY_TAG_I2C_ADDRESS_DEFAULT = 0,
    BC_HUMIDITY_TAG_I2C_ADDRESS_ALTERNATE = 1

} bc_humidity_tag_i2c_address_t;

typedef enum
{
    BC_HTS221_I2C_ADDRESS_DEFAULT = 0x5f,
    BC_HTS221_I2C_ADDRESS_ALTERNATE = 0x40

} bc_hts221_i2c_address_t;

typedef enum
{
    BC_HDC2080_I2C_ADDRESS_DEFAULT = 0x40,
    BC_HDC2080_I2C_ADDRESS_ALTERNATE = 0x41

} bc_hdc2080_i2c_address_t;

/*
 typedef bc_hdc2080_event_t bc_humidity_tag_event_t;
 typedef bc_hdc2080_t bc_humidity_tag_t;
 */

typedef struct
{
    union
    {
        bc_hts221_t hts221;
        bc_hdc2080_t hdc2080;
    } sensor;
    bc_humidity_tag_device_t device_type;
} bc_humidity_tag_t;

typedef union
{
    bc_hts221_event_t hts221_event;
    bc_hdc2080_event_t hdc2080_event;
} bc_humidity_tag_event_t;

// TODO ... if, else if, else ... nahradit matic9 funkc9

void bc_humidity_tag_init(bc_humidity_tag_t *self, bc_i2c_channel_t i2c_channel, bc_humidity_tag_i2c_address_t i2c_address)
{
    // TODO ... dala by se dodìlat autodetekce, ale z toho vyplívá dost problémù ...

    if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HTS221)
    {
        if(i2c_address == BC_HUMIDITY_TAG_I2C_ADDRESS_DEFAULT)
        {
            bc_hts221_init((bc_hts221_t *)self, i2c_channel, BC_HTS221_I2C_ADDRESS_DEFAULT);
        }
        else
        {
            bc_hts221_init((bc_hts221_t *)self, i2c_channel, BC_HTS221_I2C_ADDRESS_ALTERNATE);
        }
    }
    else if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HDC2080)
    {
        if(i2c_address == BC_HUMIDITY_TAG_I2C_ADDRESS_DEFAULT)
        {
            bc_hdc2080_init((bc_hdc2080_t *)self, i2c_channel, BC_HDC2080_I2C_ADDRESS_DEFAULT);
        }
        else
        {
            bc_hdc2080_init((bc_hdc2080_t *)self, i2c_channel, BC_HDC2080_I2C_ADDRESS_ALTERNATE);
        }
    }
    else
    {
        for (;;);
    }
}

void bc_humidity_tag_set_event_handler(bc_humidity_tag_t *self, void (*event_handler)(bc_humidity_tag_t *, bc_humidity_tag_event_t))
{
    if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HTS221)
    {
        bc_hts221_set_event_handler((bc_hts221_t *) &self->sensor, (void (*)(bc_hts221_t *, bc_hts221_event_t)) event_handler);
    }
    else if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HDC2080)
    {
        bc_hdc2080_set_event_handler((bc_hdc2080_t *) &self->sensor, (void (*)(bc_hdc2080_t *, bc_hdc2080_event_t)) event_handler);
    }
    else
    {
        for (;;);
    }
}

void bc_humidity_tag_set_update_interval(bc_humidity_tag_t *self, bc_tick_t interval)
{
    if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HTS221)
    {
        bc_hts221_set_update_interval((bc_hts221_t *) &self->sensor, interval);
    }
    else if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HDC2080)
    {
        bc_hdc2080_set_update_interval((bc_hdc2080_t *) &self->sensor, interval);
    }
    else
    {
        for (;;);
    }
}

bool bc_humidity_tag_get_humidity_raw(bc_humidity_tag_t *self, uint16_t *raw)
{
    if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HTS221)
    {
        return false;
        // TODO ... not implemented
        // return bc_hts221_get_humidity_raw((bc_hts221_t *) &self->device, raw);
    }
    else if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HDC2080)
    {
        return bc_hdc2080_get_humidity_raw((bc_hdc2080_t *) &self->sensor, raw);
    }
    else
    {
        for (;;);
    }
}

bool bc_humidity_tag_get_humidity_percentage(bc_humidity_tag_t *self, float *percentage)
{
    if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HTS221)
    {
        return bc_hts221_get_humidity_percentage((bc_hts221_t *) &self->sensor, percentage);
    }
    else if (self->device_type == BC_HUMIDITY_TAG_DEVICE_HDC2080)
    {
        return bc_hdc2080_get_humidity_percentage((bc_hdc2080_t *) &self->sensor, percentage);
    }
    else
    {
        for (;;)
            ;
    }
}

#endif // _BC_HUMIDITY_TAG_H
