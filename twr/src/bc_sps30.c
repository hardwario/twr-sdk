#include <bc_sps30.h>

#define _BC_SPS30_DELAY_RUN 100
#define _BC_SPS30_DELAY_INITIALIZE 1000
#define _BC_SPS30_DELAY_READ 30
#define _BC_SPS30_DELAY_MEASUREMENT 250

#define BC_SPS30_NUM_WORDS(x) (sizeof(x) / 2)

#define be16_to_cpu(s) (((uint16_t)(s) << 8) | (0xff & ((uint16_t)(s)) >> 8))
#define be32_to_cpu(s) (((uint32_t)be16_to_cpu(s) << 16) | (0xffff & (be16_to_cpu((s) >> 16))))

static void _bc_sps30_task_interval(void *param);

static void _bc_sps30_task_measure(void *param);

static uint8_t _bc_sps30_calculate_crc(uint8_t *buffer, size_t length);
static bool _bc_sps30_convert_to_words(uint8_t *buffer, size_t buffer_length, uint16_t *data, size_t data_length);

void bc_sps30_init(bc_sps30_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = bc_scheduler_register(_bc_sps30_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_sps30_task_measure, self, _BC_SPS30_DELAY_RUN);

    self->_state = BC_SPS30_STATE_INITIALIZE;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_100_KHZ);
}

void bc_sps30_set_event_handler(bc_sps30_t *self, void (*event_handler)(bc_sps30_t *, bc_sps30_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_sps30_set_startup_time(bc_sps30_t *self, bc_tick_t startup_time)
{
    self->_startup_time = startup_time;
}

void bc_sps30_set_update_interval(bc_sps30_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, _BC_SPS30_DELAY_INITIALIZE);
    }
}

bool bc_sps30_measure(bc_sps30_t *self)
{
    if (self->_state == BC_SPS30_STATE_READY)
    {
        self->_state = BC_SPS30_STATE_START_MEASUREMENT;
        self->_start_time = bc_tick_get();

        bc_scheduler_plan_now(self->_task_id_measure);

        return true;
    }

    return false;
}

bool bc_sps30_get_mass_concentration(bc_sps30_t *self, bc_sps30_mass_concentration_t *mass_concentration)
{
    if (!self->_measurement_valid)
    {
        return false;
    }

    mass_concentration->mc_1p0 = self->_mass_concentration.mc_1p0;
    mass_concentration->mc_2p5 = self->_mass_concentration.mc_2p5;
    mass_concentration->mc_4p0 = self->_mass_concentration.mc_4p0;
    mass_concentration->mc_10p0 = self->_mass_concentration.mc_10p0;

    return true;
}

bool bc_sps30_get_number_concentration(bc_sps30_t *self, bc_sps30_number_concentration_t *number_concentration)
{
    if (!self->_measurement_valid)
    {
        return false;
    }

    number_concentration->nc_0p5 = self->_number_concentration.nc_0p5;
    number_concentration->nc_1p0 = self->_number_concentration.nc_1p0;
    number_concentration->nc_2p5 = self->_number_concentration.nc_2p5;
    number_concentration->nc_4p0 = self->_number_concentration.nc_4p0;
    number_concentration->nc_10p0 = self->_number_concentration.nc_10p0;

    return true;
}

bool bc_sps30_get_typical_particle_size(bc_sps30_t *self, float *typical_particle_size)
{
    if (!self->_measurement_valid)
    {
        return false;
    }

    *typical_particle_size = self->_typical_particle_size;

    return true;
}

static void _bc_sps30_task_interval(void *param)
{
    bc_sps30_t *self = param;

    bc_sps30_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_sps30_task_measure(void *param)
{
    bc_sps30_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case BC_SPS30_STATE_ERROR:
            {
                self->_measurement_valid = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_SPS30_EVENT_ERROR, self->_event_param);
                }

                self->_state = BC_SPS30_STATE_INITIALIZE;

                continue;
            }
            case BC_SPS30_STATE_READY:
            {
                return;
            }
            case BC_SPS30_STATE_INITIALIZE:
            {
                self->_state = BC_SPS30_STATE_GET_SERIAL_NUMBER;

                continue;
            }
            case BC_SPS30_STATE_GET_SERIAL_NUMBER:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                static const uint8_t buffer[] = { 0xd0, 0x33 };

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = (uint8_t *) buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_write(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                self->_state = BC_SPS30_STATE_READ_SERIAL_NUMBER;

                bc_scheduler_plan_current_from_now(_BC_SPS30_DELAY_READ);

                return;
            }
            case BC_SPS30_STATE_READ_SERIAL_NUMBER:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                uint8_t buffer[48];
                union {
                    char serial[32];
                    uint16_t __enforce_alignment;
                } data;

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_read(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                if (!_bc_sps30_convert_to_words(buffer, sizeof(buffer),
                    (uint16_t *) data.serial, BC_SPS30_NUM_WORDS(data.serial)))
                {
                    continue;
                }

                self->_state = BC_SPS30_STATE_READY;

                continue;
            }
            case BC_SPS30_STATE_START_MEASUREMENT:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                uint8_t buffer[5];

                buffer[0] = 0x00;
                buffer[1] = 0x10;
                buffer[2] = 0x03;
                buffer[3] = 0x00;
                buffer[4] = _bc_sps30_calculate_crc(&buffer[2], 2);

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_write(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                self->_state = BC_SPS30_STATE_SET_DATAREADY_FLAG;

                continue;
            }
            case BC_SPS30_STATE_SET_DATAREADY_FLAG:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                static const uint8_t buffer[] = { 0x02, 0x02 };

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = (uint8_t *) buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_write(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                self->_state = BC_SPS30_STATE_READ_DATAREADY_FLAG;

                bc_scheduler_plan_current_from_now(_BC_SPS30_DELAY_READ);

                return;
            }
            case BC_SPS30_STATE_READ_DATAREADY_FLAG:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                uint8_t buffer[3];

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_read(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                if (_bc_sps30_calculate_crc(&buffer[0], 2) != buffer[2])
                {
                    continue;
                }

                if (buffer[1] == 0x01)
                {
                    self->_state = BC_SPS30_STATE_GET_MEASUREMENT_DATA;

                    continue;
                }

                self->_state = BC_SPS30_STATE_READ_DATAREADY_FLAG;

                bc_scheduler_plan_current_from_now(_BC_SPS30_DELAY_MEASUREMENT);

                return;
            }
            case BC_SPS30_STATE_GET_MEASUREMENT_DATA:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                static const uint8_t buffer[] = { 0x03, 0x00 };

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = (uint8_t *) buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_write(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                self->_state = BC_SPS30_STATE_READ_MEASUREMENT_DATA;

                bc_scheduler_plan_current_from_now(_BC_SPS30_DELAY_READ);

                return;
            }
            case BC_SPS30_STATE_READ_MEASUREMENT_DATA:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                uint8_t buffer[60];
                union {
                    uint16_t uint16_t[2];
                    uint32_t u;
                    float f;
                } val, data[10];

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = (uint8_t *) buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_read(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                if (!_bc_sps30_convert_to_words(buffer, sizeof(buffer), data->uint16_t, BC_SPS30_NUM_WORDS(data)))
                {
                    continue;
                }

                val.u = be32_to_cpu(data[0].u);
                self->_mass_concentration.mc_1p0 = val.f;
                val.u = be32_to_cpu(data[1].u);
                self->_mass_concentration.mc_2p5 = val.f;
                val.u = be32_to_cpu(data[2].u);
                self->_mass_concentration.mc_4p0 = val.f;
                val.u = be32_to_cpu(data[3].u);
                self->_mass_concentration.mc_10p0 = val.f;
                val.u = be32_to_cpu(data[4].u);
                self->_number_concentration.nc_0p5 = val.f;
                val.u = be32_to_cpu(data[5].u);
                self->_number_concentration.nc_1p0 = val.f;
                val.u = be32_to_cpu(data[6].u);
                self->_number_concentration.nc_2p5 = val.f;
                val.u = be32_to_cpu(data[7].u);
                self->_number_concentration.nc_4p0 = val.f;
                val.u = be32_to_cpu(data[8].u);
                self->_number_concentration.nc_10p0 = val.f;
                val.u = be32_to_cpu(data[9].u);
                self->_typical_particle_size = val.f;

                self->_measurement_valid = true;

                if ((bc_tick_get() - self->_start_time) > self->_startup_time)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_SPS30_EVENT_UPDATE, self->_event_param);
                    }

                    self->_state = BC_SPS30_STATE_STOP_MEASUREMENT;
                }
                else
                {
                    self->_state = BC_SPS30_STATE_SET_DATAREADY_FLAG;
                }

                continue;
            }
            case BC_SPS30_STATE_STOP_MEASUREMENT:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                static const uint8_t buffer[] = { 0x01, 0x04 };

                bc_i2c_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.buffer = (uint8_t *) buffer;
                transfer.length = sizeof(buffer);

                if (!bc_i2c_write(self->_i2c_channel, &transfer))
                {
                    continue;
                }

                self->_state = BC_SPS30_STATE_READY;

                continue;
            }
            default:
            {
                self->_state = BC_SPS30_STATE_ERROR;

                continue;
            }
        }
    }
}

static uint8_t _bc_sps30_calculate_crc(uint8_t *buffer, size_t length)
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

static bool _bc_sps30_convert_to_words(uint8_t *buffer, size_t buffer_length, uint16_t *data, size_t data_length)
{
    uint8_t *data8 = (uint8_t *) data;
    size_t i, j;

    if (buffer_length != (data_length * 3))
    {
        return false;
    }

    for (i = 0, j = 0; i < buffer_length; i += 3)
    {
        if (_bc_sps30_calculate_crc(&buffer[i], 2) != buffer[i + 2])
        {
            return false;
        }

        data8[j++] = buffer[i];
        data8[j++] = buffer[i + 1];
    }

    return true;
}
