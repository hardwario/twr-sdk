#include <bc_lis2dh12.h>
#include <bc_scheduler.h>

#define BC_LIS2DH12_DELAY_RUN 50
#define BC_LIS2DH12_DELAY_READ 50

static bc_tick_t _bc_lis2dh12_task(void *param, bc_tick_t tick_now);

static bool _bc_lis2dh12_power_down(bc_lis2dh12_t *self);
static bool _bc_lis2dh12_continuous_conversion(bc_lis2dh12_t *self);
static bool _bc_lis2dh12_read_result(bc_lis2dh12_t *self);

bool bc_lis2dh12_init(bc_lis2dh12_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    uint8_t who_am_i;

    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;
    self->_update_interval = 50;

    // Read WHO_AM_I
    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x0f, &who_am_i))
    {
        return false;
    }

    // Check WHO_AM_I register
    if (who_am_i != 0x33)
    {
        return false;
    }

    if (!_bc_lis2dh12_power_down(self))
    {
        return false;
    }

    if (!_bc_lis2dh12_continuous_conversion(self))
    {
        return false;
    }

    bc_scheduler_register(_bc_lis2dh12_task, self, BC_LIS2DH12_DELAY_RUN);

    return true;
}

void bc_lis2dh12_set_event_handler(bc_lis2dh12_t *self, void (*event_handler)(bc_lis2dh12_t *, bc_lis2dh12_event_t))
{
    self->_event_handler = event_handler;
}

void bc_lis2dh12_set_update_interval(bc_lis2dh12_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;
}

bool bc_lis2dh12_get_result_raw(bc_lis2dh12_t *self, bc_lis2dh12_result_raw_t *result_raw)
{
    result_raw->x_axis = (int16_t) self->_out_x_h;
    result_raw->x_axis <<= 8;
    result_raw->x_axis >>= 4;
    result_raw->x_axis |= (int16_t) self->_out_x_l >> 4; // TODO Clarify this

    result_raw->y_axis = (int16_t) self->_out_y_h;
    result_raw->y_axis <<= 8;
    result_raw->y_axis >>= 4;
    result_raw->y_axis |= (int16_t) self->_out_y_l >> 4; // TODO Clarify this

    result_raw->z_axis = (int16_t) self->_out_z_h;
    result_raw->z_axis <<= 8;
    result_raw->z_axis >>= 4;
    result_raw->z_axis |= (int16_t) self->_out_z_l >> 4; // TODO Clarify this

    return true;
}

bool bc_lis2dh12_get_result_g(bc_lis2dh12_t *self, bc_lis2dh12_result_g_t *result_g)
{
    bc_lis2dh12_result_raw_t result_raw;

    if (!bc_lis2dh12_get_result_raw(self, &result_raw))
    {
        return false;
    }

    result_g->x_axis = ((float) result_raw.x_axis) / 512.f;
    result_g->y_axis = ((float) result_raw.y_axis) / 512.f;
    result_g->z_axis = ((float) result_raw.z_axis) / 512.f;

    return true;
}

static bc_tick_t _bc_lis2dh12_task(void *param, bc_tick_t tick_now)
{
    bc_lis2dh12_t *self = param;

start:

    switch (self->_state)
    {
        case BC_LIS2DH12_STATE_ERROR:
        {
            self->_accelerometer_valid = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_LIS2DH12_EVENT_ERROR);
            }

            self->_state = BC_LIS2DH12_STATE_MEASURE;

            return tick_now + self->_update_interval;
        }
        case BC_LIS2DH12_STATE_MEASURE:
        {
            self->_state = BC_LIS2DH12_STATE_ERROR;

            if (!_bc_lis2dh12_continuous_conversion(self))
            {
                goto start;
            }

            self->_state = BC_LIS2DH12_STATE_READ;

            return tick_now + BC_LIS2DH12_DELAY_READ;
        }
        case BC_LIS2DH12_STATE_READ:
        {
            self->_state = BC_LIS2DH12_STATE_ERROR;

            if (!_bc_lis2dh12_read_result(self))
            {
                goto start;
            }

            if (!_bc_lis2dh12_power_down(self))
            {
                goto start;
            }

            self->_accelerometer_valid = true;

            self->_state = BC_LIS2DH12_STATE_UPDATE;

            goto start;
        }
        case BC_LIS2DH12_STATE_UPDATE:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_LIS2DH12_EVENT_UPDATE);
            }

            self->_state = BC_LIS2DH12_STATE_MEASURE;

            return tick_now + self->_update_interval;
        }
        default:
        {
            self->_state = BC_LIS2DH12_STATE_ERROR;

            goto start;
        }
    }
}

static bool _bc_lis2dh12_power_down(bc_lis2dh12_t *self)
{
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, 0x07))
    {
        return false;
    }

    return true;
}

static bool _bc_lis2dh12_continuous_conversion(bc_lis2dh12_t *self)
{
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x23, 0x98))
    {
        return false;
    }

    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, 0x27))
    {
        return false;
    }

    return true;
}

static bool _bc_lis2dh12_read_result(bc_lis2dh12_t *self)
{
    /*
     // Dont work yet, needs I2C repeated start reading
     bc_i2c_tranfer_t transfer;

     transfer.device_address = self->_i2c_address;
     transfer.memory_address = 0x28;
     transfer.buffer = &self->_out_x_l;
     transfer.length = 6;

     return bc_i2c_read(self->_i2c_channel, &transfer);*/

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x28, &self->_out_x_l))
    {
        return false;
    }

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x29, &self->_out_x_h))
    {
        return false;
    }

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2a, &self->_out_y_l))
    {
        return false;
    }

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2b, &self->_out_y_h))
    {
        return false;
    }

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2c, &self->_out_z_l))
    {
        return false;
    }

    if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2d, &self->_out_z_h))
    {
        return false;
    }

    return true;
}
