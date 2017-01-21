#include <bc_humidity_tag.h>

typedef enum
{
    _BC_HUMIDITY_TAG_FUNCTION_INIT = 0,
    _BC_HUMIDITY_TAG_FUNCTION_SET_EVENT_HANDLER,
    _BC_HUMIDITY_TAG_FUNCTION_SET_UPDATE_INTERVAL,
    _BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_RAW,
    _BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_PERCENTAGE,
    _BC_HUMIDITY_TAG_FUNCTION_COUNT
} _bc_humidity_tag_function_t;

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

__attribute__((weak)) bool bc_hts221_get_humidity_raw(bc_hts221_t *self, uint16_t *raw);

/* Dirty, another way have to be found, */
/* but iso c forbids conversion of function pointer to object pointer type */
static const uint32_t fcn_table[BC_HUMIDITY_TAG_DEVICE_COUNT][_BC_HUMIDITY_TAG_FUNCTION_COUNT] =
{
    [BC_HUMIDITY_TAG_DEVICE_HTS221][_BC_HUMIDITY_TAG_FUNCTION_INIT] = (const uint32_t)&bc_hts221_init,
    [BC_HUMIDITY_TAG_DEVICE_HDC2080][_BC_HUMIDITY_TAG_FUNCTION_INIT] = (const uint32_t)&bc_hdc2080_init,

    [BC_HUMIDITY_TAG_DEVICE_HTS221][_BC_HUMIDITY_TAG_FUNCTION_SET_EVENT_HANDLER] = (const uint32_t)&bc_hts221_set_event_handler,
    [BC_HUMIDITY_TAG_DEVICE_HDC2080][_BC_HUMIDITY_TAG_FUNCTION_SET_EVENT_HANDLER] = (const uint32_t)&bc_hdc2080_set_event_handler,

    [BC_HUMIDITY_TAG_DEVICE_HTS221][_BC_HUMIDITY_TAG_FUNCTION_SET_UPDATE_INTERVAL] = (const uint32_t)&bc_hts221_set_update_interval,
    [BC_HUMIDITY_TAG_DEVICE_HDC2080][_BC_HUMIDITY_TAG_FUNCTION_SET_UPDATE_INTERVAL] = (const uint32_t)&bc_hdc2080_set_update_interval,

    [BC_HUMIDITY_TAG_DEVICE_HTS221][_BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_RAW] = (const uint32_t)&bc_hts221_get_humidity_raw,
    [BC_HUMIDITY_TAG_DEVICE_HDC2080][_BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_RAW] = (const uint32_t)&bc_hdc2080_get_humidity_raw,

    [BC_HUMIDITY_TAG_DEVICE_HTS221][_BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_PERCENTAGE] = (const uint32_t)&bc_hts221_get_humidity_percentage,
    [BC_HUMIDITY_TAG_DEVICE_HDC2080][_BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_PERCENTAGE] = (const uint32_t)&bc_hdc2080_get_humidity_percentage,
};

#define _BC_HUMIDITY_TAG_INIT(_SELF_, _I2C_CHANNEL_, _I2C_ADDRESS_) (((void (*)(bc_humidity_tag_t *, uint8_t, uint8_t)) fcn_table[self->device_type][_BC_HUMIDITY_TAG_FUNCTION_SET_EVENT_HANDLER])((void *)&_SELF_->sensor, _I2C_CHANNEL_, _I2C_ADDRESS_))
#define _BC_HUMIDITY_TAG_SET_EVENT_HANDLER(_SELF_, _EVENT_HANDLER_) (((void (*)(bc_humidity_tag_t *, uint32_t)) fcn_table[_SELF_->device_type][_BC_HUMIDITY_TAG_FUNCTION_SET_EVENT_HANDLER])((void *)&_SELF_->sensor, (uint32_t)_EVENT_HANDLER_))
#define _BC_HUMIDITY_TAG_SET_UPDATE_INTERVAL(_SELF_, _INTERVAL_) (((void (*)(bc_humidity_tag_t *, bc_tick_t)) fcn_table[_SELF_->device_type][_BC_HUMIDITY_TAG_FUNCTION_SET_UPDATE_INTERVAL])((void *)&_SELF_->sensor, _INTERVAL_))
#define _BC_HUMIDITY_TAG_GET_HUMIDITY_RAW(_SELF_, _P_RAW_) (((bool (*)(bc_humidity_tag_t *, uint16_t *)) fcn_table[_SELF_->device_type][_BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_RAW])((void *)&_SELF_->sensor, _P_RAW_))
#define _BC_HUMIDITY_TAG_GET_HUMIDITY_PERCENTAGE(_SELF_, _P_PERCENTAGE_) (((bool (*)(bc_humidity_tag_t *, float *)) fcn_table[_SELF_->device_type][_BC_HUMIDITY_TAG_FUNCTION_GET_HUMIDITY_PERCENTAGE])((void *)&_SELF_->sensor, _P_PERCENTAGE_))

void bc_humidity_tag_init(bc_humidity_tag_t *self, uint8_t i2c_channel, bc_humidity_tag_i2c_address_t i2c_address)
{
    _BC_HUMIDITY_TAG_INIT(self, i2c_channel, i2c_address);
}

void bc_humidity_tag_set_event_handler(bc_humidity_tag_t *self, void (*event_handler)(bc_humidity_tag_t *, bc_humidity_tag_event_t))
{
    _BC_HUMIDITY_TAG_SET_EVENT_HANDLER(self, event_handler);
}

void bc_humidity_tag_set_update_interval(bc_humidity_tag_t *self, bc_tick_t interval)
{
    _BC_HUMIDITY_TAG_SET_UPDATE_INTERVAL(self, interval);
}

bool bc_humidity_tag_get_humidity_raw(bc_humidity_tag_t *self, uint16_t *raw)
{
    return _BC_HUMIDITY_TAG_GET_HUMIDITY_RAW(self, raw);
}

bool bc_humidity_tag_get_humidity_percentage(bc_humidity_tag_t *self, float *percentage)
{
    return _BC_HUMIDITY_TAG_GET_HUMIDITY_PERCENTAGE(self, percentage);
}

__attribute__((weak)) bool bc_hts221_get_humidity_raw(bc_hts221_t *self, uint16_t *raw)
{
    return false;
}
