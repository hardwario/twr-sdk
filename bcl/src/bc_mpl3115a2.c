#include <bc_mpl3115a2.h>
#include <bc_scheduler.h>

#define BC_MPL3115A2_DELAY_RUN 1500
#define BC_MPL3115A2_DELAY_RESET 1500
#define BC_MPL3115A2_DELAY_MEASUREMENT 1500

static void _bc_mpl3115a2_task(void *param);

void bc_mpl3115a2_init(bc_mpl3115a2_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    bc_scheduler_register(_bc_mpl3115a2_task, self, BC_MPL3115A2_DELAY_RUN);
}

void bc_mpl3115a2_set_event_handler(bc_mpl3115a2_t *self, void (*event_handler)(bc_mpl3115a2_t *, bc_mpl3115a2_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_mpl3115a2_set_update_interval(bc_mpl3115a2_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;
}

bool bc_mpl3115a2_get_altitude_meter(bc_mpl3115a2_t *self, float *meter)
{
    if (!self->_altitude_valid)
    {
        return false;
    }

    uint32_t out_p = (uint32_t) self->_reg_out_p_msb_altitude << 16 | (uint32_t) self->_reg_out_p_csb_altitude << 8 | (uint32_t) self->_reg_out_p_lsb_altitude;

    *meter = ((float) out_p) / 256.f;

    return true;
}

bool bc_mpl3115a2_get_pressure_pascal(bc_mpl3115a2_t *self, float *pascal)
{
    if (!self->_pressure_valid)
    {
        return false;
    }

    uint32_t out_p = (uint32_t) self->_reg_out_p_msb_pressure << 16 | (uint32_t) self->_reg_out_p_csb_pressure << 8 | (uint32_t) self->_reg_out_p_lsb_pressure;

    *pascal = ((float) out_p) / 64.f;

    return true;
}

static void _bc_mpl3115a2_task(void *param)
{
    bc_mpl3115a2_t *self = param;

start:

    switch (self->_state)
    {
        case BC_MPL3115A2_STATE_ERROR:
        {
            self->_altitude_valid = false;
            self->_pressure_valid = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_MPL3115A2_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_MPL3115A2_STATE_INITIALIZE;

            bc_scheduler_plan_current_relative(self->_update_interval);

            return;
        }
        case BC_MPL3115A2_STATE_INITIALIZE:
        {
            bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0x04);

            self->_state = BC_MPL3115A2_STATE_MEASURE_ALTITUDE;

            bc_scheduler_plan_current_relative(BC_MPL3115A2_DELAY_RESET);

            return;
        }
        case BC_MPL3115A2_STATE_MEASURE_ALTITUDE:
        {
            self->_state = BC_MPL3115A2_STATE_ERROR;

            if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0xb8))
            {
                goto start;
            }

            if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x13, 0x07))
            {
                goto start;
            }

            if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0xba))
            {
                goto start;
            }

            self->_state = BC_MPL3115A2_STATE_READ_ALTITUDE;

            bc_scheduler_plan_current_relative(BC_MPL3115A2_DELAY_MEASUREMENT);

            return;
        }
        case BC_MPL3115A2_STATE_READ_ALTITUDE:
        {
            self->_state = BC_MPL3115A2_STATE_ERROR;

            uint8_t reg_status;

            if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x00, &reg_status))
            {
                goto start;
            }

            if (reg_status != 0x0e)
            {
                goto start;
            }

            uint8_t buffer[5];

            bc_i2c_tranfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.memory_address = 0x01;
            transfer.buffer = buffer;
            transfer.length = 5;

            if (!bc_i2c_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_reg_out_p_msb_altitude = buffer[0];
            self->_reg_out_p_csb_altitude = buffer[1];
            self->_reg_out_p_lsb_altitude = buffer[2];
            self->_reg_out_t_msb_altitude = buffer[3];
            self->_reg_out_t_lsb_altitude = buffer[4];

            self->_altitude_valid = true;

            self->_state = BC_MPL3115A2_STATE_MEASURE_PRESSURE;

            goto start;
        }
        case BC_MPL3115A2_STATE_MEASURE_PRESSURE:
        {
            self->_state = BC_MPL3115A2_STATE_ERROR;

            if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0x38))
            {
                goto start;
            }

            if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x13, 0x07))
            {
                goto start;
            }

            if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0x3a))
            {
                goto start;
            }

            self->_state = BC_MPL3115A2_STATE_READ_PRESSURE;

            bc_scheduler_plan_current_relative(BC_MPL3115A2_DELAY_MEASUREMENT);

            return;
        }
        case BC_MPL3115A2_STATE_READ_PRESSURE:
        {
            self->_state = BC_MPL3115A2_STATE_ERROR;

            uint8_t reg_status;

            if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x00, &reg_status))
            {
                goto start;
            }

            if (reg_status != 0x0e)
            {
                goto start;
            }

            uint8_t buffer[5];

            bc_i2c_tranfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.memory_address = 0x01;
            transfer.buffer = buffer;
            transfer.length = 5;

            if (!bc_i2c_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_reg_out_p_msb_pressure = buffer[0];
            self->_reg_out_p_csb_pressure = buffer[1];
            self->_reg_out_p_lsb_pressure = buffer[2];
            self->_reg_out_t_msb_pressure = buffer[3];
            self->_reg_out_t_lsb_pressure = buffer[4];

            self->_pressure_valid = true;

            self->_state = BC_MPL3115A2_STATE_UPDATE;

            goto start;
        }
        case BC_MPL3115A2_STATE_UPDATE:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_MPL3115A2_EVENT_UPDATE, self->_event_param);
            }

            self->_state = BC_MPL3115A2_STATE_MEASURE_ALTITUDE;

            bc_scheduler_plan_current_relative(self->_update_interval);

            return;
        }
        default:
        {
            self->_state = BC_MPL3115A2_STATE_ERROR;

            goto start;
        }
    }
}
