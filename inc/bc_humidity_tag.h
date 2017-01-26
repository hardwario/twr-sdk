#ifndef _BC_HUMIDITY_TAG_H
#define _BC_HUMIDITY_TAG_H

#include <bc_hdc2080.h>
#include <bc_hts221.h>
#include <bc_common.h>
#include <bc_tick.h>

typedef enum
{
    BC_HUMIDITY_TAG_DEVICE_HTS221 = 0,
    BC_HUMIDITY_TAG_DEVICE_HDC2080,
    BC_HUMIDITY_TAG_DEVICE_COUNT

} bc_humidity_tag_device_t;

typedef enum
{
    BC_HUMIDITY_TAG_I2C_ADDRESS_DEFAULT = 0,
    BC_HUMIDITY_TAG_I2C_ADDRESS_ALTERNATE,
    BC_HUMIDITY_TAG_I2C_ADDRESS_COUNT

} bc_humidity_tag_i2c_address_t;

typedef enum
{
#if (BC_HTS221_EVENT_ERROR == BC_HDC2080_EVENT_ERROR)
    BC_HUMIDITY_TAG_EVENT_ERROR = BC_HTS221_EVENT_ERROR,
#endif

#if (BC_HTS221_EVENT_UPDATE == BC_HDC2080_EVENT_UPDATE)
    BC_HUMIDITY_TAG_EVENT_UPDATE = BC_HTS221_EVENT_UPDATE,
#endif
} bc_humidity_tag_event_t;

typedef struct
{
    union
    {
        bc_hts221_t hts221;
        bc_hdc2080_t hdc2080;
    } sensor;
    bc_humidity_tag_device_t device_type;
} bc_humidity_tag_t;

void bc_humidity_tag_init(bc_humidity_tag_t *self, uint8_t i2c_channel, bc_humidity_tag_i2c_address_t i2c_address);
void bc_humidity_tag_set_event_handler(bc_humidity_tag_t *self, void (*event_handler)(bc_humidity_tag_t *, bc_humidity_tag_event_t));
void bc_humidity_tag_set_update_interval(bc_humidity_tag_t *self, bc_tick_t interval);
bool bc_humidity_tag_get_humidity_raw(bc_humidity_tag_t *self, uint16_t *raw);
bool bc_humidity_tag_get_humidity_percentage(bc_humidity_tag_t *self, float *percentage);

#endif // _BC_HUMIDITY_TAG_H
