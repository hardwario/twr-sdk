#include <twr_mpl3115a2.h>

#define _TWR_MPL3115A2_DELAY_RUN 1500
#define _TWR_MPL3115A2_DELAY_INITIALIZATION 1500
#define _TWR_MPL3115A2_DELAY_MEASUREMENT 1500

static void _twr_mpl3115a2_task_interval(void *param);

static void _twr_mpl3115a2_task_measure(void *param);

void twr_mpl3115a2_init(twr_mpl3115a2_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = twr_scheduler_register(_twr_mpl3115a2_task_interval, self, TWR_TICK_INFINITY);
    self->_task_id_measure = twr_scheduler_register(_twr_mpl3115a2_task_measure, self, _TWR_MPL3115A2_DELAY_RUN);

    self->_tick_ready = twr_tick_get() + _TWR_MPL3115A2_DELAY_RUN;

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);
}

void twr_mpl3115a2_deinit(twr_mpl3115a2_t *self)
{
    twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0x04);

    twr_scheduler_unregister(self->_task_id_interval);

    twr_scheduler_unregister(self->_task_id_measure);
}

void twr_mpl3115a2_set_event_handler(twr_mpl3115a2_t *self, void (*event_handler)(twr_mpl3115a2_t *, twr_mpl3115a2_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_mpl3115a2_set_update_interval(twr_mpl3115a2_t *self, twr_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(self->_task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        twr_mpl3115a2_measure(self);
    }
}

bool twr_mpl3115a2_measure(twr_mpl3115a2_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    twr_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

bool twr_mpl3115a2_get_altitude_meter(twr_mpl3115a2_t *self, float *meter)
{
    if (!self->_altitude_valid)
    {
        return false;
    }

    int32_t out_pa = (uint32_t) self->_reg_out_p_msb_altitude << 24 | (uint32_t) self->_reg_out_p_csb_altitude << 16 | (uint32_t) (self->_reg_out_p_lsb_altitude & 0xf0) << 8;

    *meter = ((float) out_pa) / 65536.f;

    return true;
}

bool twr_mpl3115a2_get_pressure_pascal(twr_mpl3115a2_t *self, float *pascal)
{
    if (!self->_pressure_valid)
    {
        return false;
    }

    uint32_t out_p = (uint32_t) self->_reg_out_p_msb_pressure << 16 | (uint32_t) self->_reg_out_p_csb_pressure << 8 | (uint32_t) self->_reg_out_p_lsb_pressure;

    *pascal = ((float) out_p) / 64.f;

    return true;
}

static void _twr_mpl3115a2_task_interval(void *param)
{
    twr_mpl3115a2_t *self = param;

    twr_mpl3115a2_measure(self);

    twr_scheduler_plan_current_relative(self->_update_interval);
}

static void _twr_mpl3115a2_task_measure(void *param)
{
    twr_mpl3115a2_t *self = param;

start:

    switch (self->_state)
    {
        case TWR_MPL3115A2_STATE_ERROR:
        {
            self->_altitude_valid = false;
            self->_pressure_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_MPL3115A2_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_MPL3115A2_STATE_INITIALIZE;

            return;
        }
        case TWR_MPL3115A2_STATE_INITIALIZE:
        {
            twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0x04);

            self->_state = TWR_MPL3115A2_STATE_MEASURE_ALTITUDE;

            self->_tick_ready = twr_tick_get() + _TWR_MPL3115A2_DELAY_INITIALIZATION;

            if (self->_measurement_active)
            {
                twr_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case TWR_MPL3115A2_STATE_MEASURE_ALTITUDE:
        {
            self->_state = TWR_MPL3115A2_STATE_ERROR;

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0xb8))
            {
                goto start;
            }

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x13, 0x07))
            {
                goto start;
            }

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0xba))
            {
                goto start;
            }

            self->_state = TWR_MPL3115A2_STATE_READ_ALTITUDE;

            twr_scheduler_plan_current_from_now(_TWR_MPL3115A2_DELAY_MEASUREMENT);

            return;
        }
        case TWR_MPL3115A2_STATE_READ_ALTITUDE:
        {
            self->_state = TWR_MPL3115A2_STATE_ERROR;

            uint8_t reg_status;

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x00, &reg_status))
            {
                goto start;
            }

            if ((reg_status & 0x04) == 0)
            {
                goto start;
            }

            uint8_t buffer[5];

            twr_i2c_memory_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.memory_address = 0x01;
            transfer.buffer = buffer;
            transfer.length = 5;

            if (!twr_i2c_memory_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_reg_out_p_msb_altitude = buffer[0];
            self->_reg_out_p_csb_altitude = buffer[1];
            self->_reg_out_p_lsb_altitude = buffer[2];
            self->_reg_out_t_msb_altitude = buffer[3];
            self->_reg_out_t_lsb_altitude = buffer[4];

            self->_altitude_valid = true;

            self->_state = TWR_MPL3115A2_STATE_MEASURE_PRESSURE;

            goto start;
        }
        case TWR_MPL3115A2_STATE_MEASURE_PRESSURE:
        {
            self->_state = TWR_MPL3115A2_STATE_ERROR;

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0x38))
            {
                goto start;
            }

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x13, 0x07))
            {
                goto start;
            }

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x26, 0x3a))
            {
                goto start;
            }

            self->_state = TWR_MPL3115A2_STATE_READ_PRESSURE;

            twr_scheduler_plan_current_from_now(_TWR_MPL3115A2_DELAY_MEASUREMENT);

            return;
        }
        case TWR_MPL3115A2_STATE_READ_PRESSURE:
        {
            self->_state = TWR_MPL3115A2_STATE_ERROR;

            uint8_t reg_status;

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0x00, &reg_status))
            {
                goto start;
            }

            if ((reg_status & 0x04) == 0)
            {
                goto start;
            }

            uint8_t buffer[5];

            twr_i2c_memory_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.memory_address = 0x01;
            transfer.buffer = buffer;
            transfer.length = 5;

            if (!twr_i2c_memory_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_reg_out_p_msb_pressure = buffer[0];
            self->_reg_out_p_csb_pressure = buffer[1];
            self->_reg_out_p_lsb_pressure = buffer[2];
            self->_reg_out_t_msb_pressure = buffer[3];
            self->_reg_out_t_lsb_pressure = buffer[4];

            self->_pressure_valid = true;

            self->_state = TWR_MPL3115A2_STATE_UPDATE;

            goto start;
        }
        case TWR_MPL3115A2_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_MPL3115A2_EVENT_UPDATE, self->_event_param);
            }

            self->_state = TWR_MPL3115A2_STATE_MEASURE_ALTITUDE;

            return;
        }
        default:
        {
            self->_state = TWR_MPL3115A2_STATE_ERROR;

            goto start;
        }
    }
}
