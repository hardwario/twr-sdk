#include <bc_module_gps.h>

// TODO:
#define _BC_MODULE_GPS_I2C_CHANNEL BC_I2C_I2C0

// TODO:
#define _BC_MODULE_GPS_I2C_ADDRESS 0x00

// TODO:
#define _BC_MODULE_GPS_I2C_ADDRESS_EXPANDER 0x00

// TODO:
#define _BC_MODULE_GPS_EXPANDER_PIN (1 << 0)

void bc_module_gps_init(bc_module_gps_t *self)
{
    bc_sam_m8_init(self, _BC_MODULE_GPS_I2C_CHANNEL, _BC_MODULE_GPS_I2C_ADDRESS, _BC_MODULE_GPS_I2C_ADDRESS_EXPANDER, _BC_MODULE_GPS_EXPANDER_PIN);
}

void bc_module_gps_set_event_handler(bc_module_gps_t *self, bc_module_gps_event_handler_t event_handler, void *event_param)
{
    bc_sam_m8_set_event_handler(self, event_handler, event_param);
}

void bc_module_gps_start(bc_module_gps_t *self, uint64_t milliseconds)
{
    bc_sam_m8_start(self, milliseconds);
}

void bc_module_gps_stop(bc_module_gps_t *self)
{
    bc_sam_m8_stop(self);
}

bool bc_module_gps_get_position(bc_module_gps_t *self, float *latitude, float *longitude)
{
    return bc_sam_m8_get_position(self, latitude, longitude);
}
