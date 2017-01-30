#include <bc_lis2dh12.h>
#include <bc_scheduler.h>

#define BC_LIS2DH12_DELAY_RUN 10
#define BC_LIS2DH12_DELAY_READ 10

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

uint8_t int1_src;
uint8_t int1_cfg_read;
uint8_t ctrl_reg3_read;

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

            if(!self->_alarm_active)
            {
                if (!_bc_lis2dh12_continuous_conversion(self))
                {
                    goto start;
                }
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

            // Power down only when no alarm is set
            /*if(!self->_alarm_active)
            {
                if (!_bc_lis2dh12_power_down(self))
                {
                    goto start;
                }
            }*/

            self->_accelerometer_valid = true;

            self->_state = BC_LIS2DH12_STATE_UPDATE;

            goto start;
        }
        case BC_LIS2DH12_STATE_UPDATE:
        {
            self->_state = BC_LIS2DH12_STATE_ERROR;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_LIS2DH12_EVENT_UPDATE);
            }

            // Read Alarm bit
            if(self->_alarm_active)
            {

                if(!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x31, &int1_src))
                {
                    goto start;
                }

                if(int1_src & (1 << 6)/* || int1_src & (1 << 1)*/)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_LIS2DH12_EVENT_ALARM);
                    }
                }

                if(!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x30, &int1_cfg_read))
                {
                    goto start;
                }

                if(!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x22, &ctrl_reg3_read))
                {
                    goto start;
                }

            }

            self->_state = BC_LIS2DH12_STATE_UPDATE; //BC_LIS2DH12_STATE_MEASURE;

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

    // ODR = 0x5 => 100Hz
    if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, 0x57))
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

bool bc_lis2dh12_set_alarm(bc_lis2dh12_t *self, bc_lis2dh12_alarm_t *alarm)
{
    if(alarm)
    {
        // Enable alarm
        self->_alarm_active = true;

        // Disable IRQ first to change the registers
        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, 0x00))
        {
            return false;
        }

        // Recalculate threshold to the 4g full-scale setting
        uint8_t int1_ths = (uint8_t)((alarm->threshold) / 0.031f);

        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x32, int1_ths))
        {
            return false;
        }

        uint8_t int1_duration = (uint8_t)(alarm->duration / 1); // /10
        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x33, int1_duration))
        {
            return false;
        }

        // CTRL_REG3
        uint8_t ctrl_reg3 = (1 << 6);
        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x22, ctrl_reg3))
        {
            return false;
        }

        // CTRL_REG6 - invert interrupt
        uint8_t ctrl_reg6 = (1 << 1);
        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x25, ctrl_reg6))
        {
            return false;
        }

        // ctr_reg5
        uint8_t ctrl_reg5 = (0 << 3); // latch interrupt request
        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x24, ctrl_reg5))
        {
            return false;
        }

        // INT_CFG1
        uint8_t int_cfg1;

        int_cfg1 = 0; //(1 << 7) | (1 << 6); // AOI = 0, 6D = 1

        int_cfg1 |= (alarm->z_high) ? (1 << 5) : 0;
        int_cfg1 |= (alarm->z_low) ? (1 << 4) : 0;

        int_cfg1 |= (alarm->y_high) ? (1 << 3) : 0;
        int_cfg1 |= (alarm->y_low) ? (1 << 2) : 0;

        int_cfg1 |= (alarm->x_high) ? (1 << 1) : 0;
        int_cfg1 |= (alarm->x_low) ? (1 << 0) : 0;

        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, int_cfg1))
        {
            return false;
        }


    }
    else
    {
        // Disable alarm
        self->_alarm_active = false;

        if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x30, 0x00))
        {
            return false;
        }
    }

    return true;
}

/*
// ODR 100Hz 0x50, enable XYZ axes 0x07
uint8_t ctrl_reg1 = 0x57;
// No filters
uint8_t ctrl_reg2 = 0x00;

uint8_t click_cfg = 0x01; // X axis single click


// 1 LSB = (full scale) / 128 => 2g/128 * 0,5 => 33
uint8_t click_ths = 33;
// 1 LSB = 1/ODR => 1/100Hz = 10ms ;  100ms/10ms = 10
uint8_t time_limit = 10;

// time latency to detect double-click, not used in single click
uint8_t time_latency = 0;
// Window for double click detection, not used in single click
uint8_t time_window = 0;

void bc_lis2dh12_set_config(bc_lis2dh12_t *self)
{
    bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, ctrl_reg1);
    bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x21, ctrl_reg2);

    bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x38, click_cfg);

    //bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x39, click_src); // read only
    bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x3A, click_ths);
    bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x3B, time_limit);

    bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x3C, time_latency);
    bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x3D, time_window);

}
*/
