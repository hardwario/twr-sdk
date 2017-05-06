#include <bc_sht20.h>

#define BC_SHT20_DELAY_RUN 20
#define BC_SHT20_DELAY_INITIALIZATION 20
#define BC_SHT20_DELAY_MEASUREMENT_RH 50
#define BC_SHT20_DELAY_MEASUREMENT_T 100

static void _bc_sht20_task(void *param);

void bc_sht20_init(bc_sht20_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    self->_task_id = bc_scheduler_register(_bc_sht20_task, self, bc_tick_get() + BC_SHT20_DELAY_RUN);
}

void bc_sht20_set_event_handler(bc_sht20_t *self, void (*event_handler)(bc_sht20_t *, bc_sht20_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_sht20_set_update_interval(bc_sht20_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;
}

bool bc_sht20_get_humidity_raw(bc_sht20_t *self, uint16_t *raw)
{
    if (!self->_humidity_valid)
    {
        return false;
    }

    *raw = self->_reg_humidity;

    return true;
}

bool bc_sht20_get_humidity_percentage(bc_sht20_t *self, float *percentage)
{
    uint16_t raw;

    if (!bc_sht20_get_humidity_raw(self, &raw))
    {
        return false;
    }

    *percentage = -6.f + 125.f * (float) raw / 65536.f;

    if (*percentage >= 100.f)
    {
        *percentage = 100.f;
    }
    else if (*percentage <= 0.f)
    {
        *percentage = 0.f;
    }

    return true;
}

bool bc_sht20_get_temperature_raw(bc_sht20_t *self, uint16_t *raw)
{
    if (!self->_temperature_valid)
    {
        return false;
    }

    *raw = self->_reg_temperature;

    return true;
}

bool bc_sht20_get_temperature_celsius(bc_sht20_t *self, float *celsius)
{
    uint16_t raw;

    if (!bc_sht20_get_temperature_raw(self, &raw))
    {
        return false;
    }

    *celsius = -46.85f + 175.72f * (float) raw / 65536.f;

    return true;
}

static void _bc_sht20_task(void *param)
{
    bc_sht20_t *self = param;

start:

    switch (self->_state)
    {
        case BC_SHT20_STATE_ERROR:
        {
            self->_humidity_valid = false;
            self->_temperature_valid = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SHT20_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_SHT20_STATE_INITIALIZE;

            bc_scheduler_plan_current_relative(self->_update_interval);

            return;
        }
        case BC_SHT20_STATE_INITIALIZE:
        {
            self->_state = BC_SHT20_STATE_ERROR;

            uint8_t buffer[1];

            buffer[0] = 0xfe;

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SHT20_STATE_MEASURE_RH;

            bc_scheduler_plan_current_relative(BC_SHT20_DELAY_INITIALIZATION);

            return;
        }
        case BC_SHT20_STATE_MEASURE_RH:
        {
            self->_state = BC_SHT20_STATE_ERROR;

            uint8_t buffer[1];

            buffer[0] = 0xf5;

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SHT20_STATE_READ_RH;

            bc_scheduler_plan_current_relative(BC_SHT20_DELAY_MEASUREMENT_RH);

            return;
        }
        case BC_SHT20_STATE_READ_RH:
        {
            self->_state = BC_SHT20_STATE_ERROR;

            uint8_t buffer[2];

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_reg_humidity = buffer[0] << 8 | buffer[1];

            self->_humidity_valid = true;

            self->_state = BC_SHT20_STATE_MEASURE_T;

            goto start;
        }
        case BC_SHT20_STATE_MEASURE_T:
        {
            self->_state = BC_SHT20_STATE_ERROR;

            uint8_t buffer[1];

            buffer[0] = 0xf3;

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SHT20_STATE_READ_T;

            bc_scheduler_plan_current_relative(BC_SHT20_DELAY_MEASUREMENT_T);

            return;
        }
        case BC_SHT20_STATE_READ_T:
        {
            self->_state = BC_SHT20_STATE_ERROR;

            uint8_t buffer[2];

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_reg_temperature = buffer[0] << 8 | buffer[1];

            self->_temperature_valid = true;

            self->_state = BC_SHT20_STATE_UPDATE;

            goto start;
        }
        case BC_SHT20_STATE_UPDATE:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SHT20_EVENT_UPDATE, self->_event_param);
            }

            self->_state = BC_SHT20_STATE_MEASURE_RH;

            bc_scheduler_plan_current_relative(self->_update_interval);

            return;
        }
        default:
        {
            self->_state = BC_SHT20_STATE_ERROR;

            goto start;
        }
    }
}
