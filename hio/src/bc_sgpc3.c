#include <bc_sgpc3.h>

#define _BC_SGPC3_DELAY_RUN 30
#define _BC_SGPC3_DELAY_INITIALIZE 500
#define _BC_SGPC3_DELAY_READ_FEATURE_SET 30
#define _BC_SGPC3_DELAY_SET_POWER_MODE 30
#define _BC_SGPC3_DELAY_INIT_AIR_QUALITY 30
#define _BC_SGPC3_DELAY_SET_HUMIDITY 30
#define _BC_SGPC3_DELAY_MEASURE_AIR_QUALITY 30
#define _BC_SGPC3_DELAY_READ_AIR_QUALITY 150

static void _bc_sgpc3_task_interval(void *param);

static void _bc_sgpc3_task_measure(void *param);

static uint8_t _bc_sgpc3_calculate_crc(uint8_t *buffer, size_t length);

void bc_sgpc3_init(bc_sgpc3_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = bc_scheduler_register(_bc_sgpc3_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_sgpc3_task_measure, self, _BC_SGPC3_DELAY_RUN);

    self->_tick_ready = _BC_SGPC3_DELAY_RUN;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_100_KHZ);
}

void bc_sgpc3_set_event_handler(bc_sgpc3_t *self, void (*event_handler)(bc_sgpc3_t *, bc_sgpc3_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_sgpc3_set_update_interval(bc_sgpc3_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_sgpc3_measure(self);
    }
}

bool bc_sgpc3_measure(bc_sgpc3_t *self)
{
    if (self->_event_handler == NULL)
    {
        return true;
    }

    if (self->_hit_error && !self->_measurement_valid)
    {
        self->_event_handler(self, BC_SGPC3_EVENT_ERROR, self->_event_param);
    }
    else if (self->_measurement_valid)
    {
        self->_event_handler(self, BC_SGPC3_EVENT_UPDATE, self->_event_param);
    }

    return true;
}

bool bc_sgpc3_get_tvoc_ppb(bc_sgpc3_t *self, uint16_t *ppb)
{
    if (!self->_measurement_valid)
    {
        return false;
    }

    *ppb = self->_tvoc;

    return true;
}

float bc_sgpc3_set_compensation(bc_sgpc3_t *self, float *t_celsius, float *rh_percentage)
{
    if (t_celsius == NULL || rh_percentage == NULL)
    {
        self->_ah_scaled = 0;

        return 0.f;
    }

    double t = *t_celsius;
    double rh = *rh_percentage;
    double pws = 611.2 * exp(17.67 * t / (243.5 + t));
    double pw = pws * rh / 100.0;
    double ah = 2.165 * pw / (273.15 + t);

    self->_ah_scaled = ((uint64_t) (ah * 1000.0) * 256 * 16777) >> 24;

    return ah;
}

static void _bc_sgpc3_task_interval(void *param)
{
    bc_sgpc3_t *self = param;

    bc_sgpc3_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_sgpc3_task_measure(void *param)
{
    bc_sgpc3_t *self = param;

start:

    switch (self->_state)
    {
        case BC_SGPC3_STATE_ERROR:
        {
            self->_hit_error = true;

            self->_measurement_valid = false;

            self->_state = BC_SGPC3_STATE_INITIALIZE;

            bc_scheduler_plan_current_from_now(_BC_SGPC3_DELAY_INITIALIZE);

            return;
        }
        case BC_SGPC3_STATE_INITIALIZE:
        {
            self->_state = BC_SGPC3_STATE_GET_FEATURE_SET;

            goto start;
        }
        case BC_SGPC3_STATE_GET_FEATURE_SET:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            static const uint8_t buffer[] = { 0x20, 0x2f };

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = (uint8_t *) buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SGPC3_STATE_READ_FEATURE_SET;

            bc_scheduler_plan_current_from_now(_BC_SGPC3_DELAY_SET_POWER_MODE);

            return;
        }
        case BC_SGPC3_STATE_SET_POWER_MODE:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            uint8_t buffer[5];

            buffer[0] = 0x20;
            buffer[1] = 0x9f;
            buffer[2] = 0x00;
            buffer[3] = 0x00;
            buffer[4] = _bc_sgpc3_calculate_crc(&buffer[2], 2);

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SGPC3_STATE_READ_FEATURE_SET;

            bc_scheduler_plan_current_from_now(_BC_SGPC3_DELAY_READ_FEATURE_SET);

            return;
        }
        case BC_SGPC3_STATE_READ_FEATURE_SET:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            uint8_t buffer[3];

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            if (_bc_sgpc3_calculate_crc(&buffer[0], 3) != 0)
            {
                goto start;
            }

            if (buffer[0] != 0x10 || buffer[1] != 0x06)
            {
                goto start;
            }

            self->_state = BC_SGPC3_STATE_INIT_AIR_QUALITY;

            bc_scheduler_plan_current_from_now(_BC_SGPC3_DELAY_INIT_AIR_QUALITY);

            return;
        }
        case BC_SGPC3_STATE_INIT_AIR_QUALITY:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            static const uint8_t buffer[] = { 0x20, 0xae };

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = (uint8_t *) buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SGPC3_STATE_SET_HUMIDITY;

            bc_scheduler_plan_current_from_now(_BC_SGPC3_DELAY_SET_HUMIDITY);

            return;
        }
        case BC_SGPC3_STATE_SET_HUMIDITY:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            uint8_t buffer[5];

            buffer[0] = 0x20;
            buffer[1] = 0x61;
            buffer[2] = self->_ah_scaled >> 8;
            buffer[3] = self->_ah_scaled;
            buffer[4] = _bc_sgpc3_calculate_crc(&buffer[2], 2);

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SGPC3_STATE_MEASURE_AIR_QUALITY;

            bc_scheduler_plan_current_from_now(_BC_SGPC3_DELAY_MEASURE_AIR_QUALITY);

            self->_tick_last_measurement = bc_scheduler_get_spin_tick();

            return;
        }
        case BC_SGPC3_STATE_MEASURE_AIR_QUALITY:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            static const uint8_t buffer[] = { 0x20, 0x08 };

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = (uint8_t *) buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_write(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            self->_state = BC_SGPC3_STATE_READ_AIR_QUALITY;

            bc_scheduler_plan_current_from_now(_BC_SGPC3_DELAY_READ_AIR_QUALITY);

            return;
        }
        case BC_SGPC3_STATE_READ_AIR_QUALITY:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            uint8_t buffer[3];

            bc_i2c_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.buffer = buffer;
            transfer.length = sizeof(buffer);

            if (!bc_i2c_read(self->_i2c_channel, &transfer))
            {
                goto start;
            }

            if (_bc_sgpc3_calculate_crc(&buffer[0], 3) != 0)
            {
                goto start;
            }

            self->_tvoc = (buffer[0] << 8) | buffer[1];

            self->_measurement_valid = true;

            self->_state = BC_SGPC3_STATE_SET_HUMIDITY;

            bc_scheduler_plan_current_absolute(self->_tick_last_measurement + 30000);

            return;
        }
        default:
        {
            self->_state = BC_SGPC3_STATE_ERROR;

            goto start;
        }
    }
}

static uint8_t _bc_sgpc3_calculate_crc(uint8_t *buffer, size_t length)
{
    uint8_t crc = 0xff;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= buffer[i];

        for (int j = 0; j < 8; j++)
        {
            if ((crc & 0x80) != 0)
            {
                crc = (crc << 1) ^ 0x31;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}
