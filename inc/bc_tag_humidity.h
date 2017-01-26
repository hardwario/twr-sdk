#ifndef _BC_TAG_HUMIDITY_H
#define _BC_TAG_HUMIDITY_H

#include <bc_hdc2080.h>

typedef enum
{
    BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT = 0x40,
    BC_TAG_HUMIDITY_I2C_ADDRESS_ALTERNATE = 0x41

} bc_tag_humidity_i2c_address_t;

typedef enum
{
    BC_TAG_HUMIDITY_EVENT_ERROR = BC_HDC2080_EVENT_ERROR,
    BC_TAG_HUMIDITY_EVENT_UPDATE = BC_HDC2080_EVENT_UPDATE

} bc_tag_humidity_event_t;

typedef bc_hdc2080_t bc_tag_humidity_t;

void bc_tag_humidity_init(bc_tag_humidity_t *self, bc_i2c_channel_t i2c_channel, bc_tag_humidity_i2c_address_t i2c_address);
void bc_tag_humidity_set_event_handler(bc_tag_humidity_t *self, void (*event_handler)(bc_tag_humidity_t *, bc_tag_humidity_event_t));
void bc_tag_humidity_set_update_interval(bc_tag_humidity_t *self, bc_tick_t interval);
bool bc_tag_humidity_get_humidity_raw(bc_tag_humidity_t *self, uint16_t *raw);
bool bc_tag_humidity_get_humidity_percentage(bc_tag_humidity_t *self, float *percentage);

#endif // _BC_TAG_HUMIDITY_H
