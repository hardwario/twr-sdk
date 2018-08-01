#include <bc_zssc3123.h>
#include <bc_timer.h>
#include <bc_log.h>

static uint8_t _bc_zssc3123_get_response(bc_zssc3123_t *self);
static void _bc_zssc3123_task(void *param);

bool bc_zssc3123_init(bc_zssc3123_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    bc_timer_init();

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    self->_update_interval = BC_TICK_INFINITY;

    self->_data_fetch_delay = 100;

    self->_task_id = bc_scheduler_register(_bc_zssc3123_task, self, BC_TICK_INFINITY);

    return true;
}

bool bc_zssc3123_deinit(bc_zssc3123_t *self)
{
    bc_i2c_deinit(self->_i2c_channel);

    bc_scheduler_unregister(self->_task_id);

    return true;
}

void bc_zssc3123_set_data_fetch_delay(bc_zssc3123_t *self, bc_tick_t interval)
{
    self->_data_fetch_delay = interval;
}

void bc_zssc3123_set_event_handler(bc_zssc3123_t *self, void (*event_handler)(bc_zssc3123_t *, bc_zssc3123_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_zssc3123_set_update_interval(bc_zssc3123_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        if (!self->_measurement_active)
        {
            bc_scheduler_plan_absolute(self->_task_id, BC_TICK_INFINITY);
        }
    }
    else
    {
        bc_zssc3123_measure(self);
    }
}

bool bc_zssc3123_measure(bc_zssc3123_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id);

    return true;
}

bool bc_zssc3123_get_raw_cap_data(bc_zssc3123_t *self, uint16_t *raw)
{
    if (!self->_valid)
    {
        return false;
    }

    *raw = self->_raw;

    return true;
}

bool bc_zssc3123_start_cm(bc_zssc3123_t *self)
{
    if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0xA0, 0x0000))
    {
        return false;
    }

    return (_bc_zssc3123_get_response(self) >> 6) == 2;
}

bool bc_zssc3123_end_cm(bc_zssc3123_t *self)
{
    if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x80, 0x0000))
    {
        return false;
    }

    return true;
}

bool bc_zssc3123_eeprom_read(bc_zssc3123_t *self, uint8_t adr, uint16_t *word)
{
    if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, adr, 0x0000))
    {
        return false;
    }

    bc_timer_start();

    bc_timer_delay(90);

    bc_timer_stop();

    uint8_t buffer[3];

    bc_i2c_transfer_t transfer;

    transfer.device_address = self->_i2c_address;

    transfer.buffer = buffer;

    transfer.length = sizeof(buffer);

    if (!bc_i2c_read(self->_i2c_channel, &transfer))
    {
        return false;

    }

    if ((buffer[0] & 0x03) != 1)
    {
        return false;
    }

    *word = ((uint16_t) buffer[1]) << 8 | buffer[2];

    return true;
}

bool bc_zssc3123_eeprom_write(bc_zssc3123_t *self, uint8_t address, uint16_t word)
{
    if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x40 + address, word))
    {
        return false;
    }

    bc_timer_start();

    bc_timer_delay(12000);

    bc_timer_stop();

    return (_bc_zssc3123_get_response(self) & 0x03) == 1;
}

bool bc_zssc3123_unlock_eeprom(bc_zssc3123_t *self)
{
    if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0xa2, 0x0000))
    {
        return false;
    }

    if (!bc_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0xf0, 0x0021))
    {
        return false;
    }

    return true;
}

static uint8_t _bc_zssc3123_get_response(bc_zssc3123_t *self)
{
    bc_i2c_transfer_t transfer;

    uint8_t data;

    transfer.device_address = self->_i2c_address;

    transfer.buffer = &data;

    transfer.length = 1;

    if (!bc_i2c_read(self->_i2c_channel, &transfer))
    {
        return 0xff;
    }
    else
    {
        return data;
    }
}

static bool _bc_zssc3123_data_fetch(bc_zssc3123_t *self)
{
    bc_i2c_transfer_t transfer;

    uint8_t buffer[2];

    transfer.device_address = self->_i2c_address;

    transfer.buffer = buffer;

    transfer.length = sizeof(buffer);

    if (!bc_i2c_read(self->_i2c_channel, &transfer))
    {
        return false;
    }

    if ((buffer[0] & 0xc0) == 0)
    {
        self->_valid = true;

        self->_raw = (uint16_t) (buffer[0] & 0x3f) << 8 | buffer[1];
    }

    return true;
}

static bool _bc_zssc3123_measurement_request(bc_zssc3123_t *self)
{
    bc_i2c_transfer_t transfer;

    uint8_t buffer[1] = { self->_i2c_address << 1 };

    transfer.device_address = self->_i2c_address;

    transfer.buffer = buffer;

    transfer.length = sizeof(buffer);

    if (!bc_i2c_write(self->_i2c_channel, &transfer))
    {
        return false;
    }

    return true;
}

static void _bc_zssc3123_task(void *param)
{
    bc_zssc3123_t *self = param;

start:

    switch (self->_state)
    {
        case BC_ZSSC3123_STATE_ERROR:
        {
            self->_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_ZSSC3123_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_ZSSC3123_STATE_INITIALIZE;

            bc_scheduler_plan_current_absolute(self->_next_update_start);

            return;
        }
        case BC_ZSSC3123_STATE_INITIALIZE:
        {
            self->_next_update_start = bc_tick_get() + self->_update_interval;

            if (!_bc_zssc3123_data_fetch(self))
            {
                self->_state = BC_ZSSC3123_STATE_ERROR;

                goto start;
            }

            self->_state = BC_ZSSC3123_STATE_MEASURE;

            bc_scheduler_plan_current_now();

            return;
        }
        case BC_ZSSC3123_STATE_MEASURE:
        {
            self->_next_update_start = bc_tick_get() + self->_update_interval;

            self->_measurement_active = true;

            self->_valid = false;

            if (!_bc_zssc3123_measurement_request(self))
            {
                self->_state = BC_ZSSC3123_STATE_ERROR;

                goto start;
            }

            self->_state = BC_ZSSC3123_STATE_READ;

            bc_scheduler_plan_current_from_now(self->_data_fetch_delay);

            return;
        }
        case BC_ZSSC3123_STATE_READ:
        {
            if (!_bc_zssc3123_data_fetch(self))
            {
                self->_state = BC_ZSSC3123_STATE_ERROR;

                goto start;
            }

            self->_measurement_active = false;

            self->_state = BC_ZSSC3123_STATE_MEASURE;

            bc_scheduler_plan_current_absolute(self->_next_update_start);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_ZSSC3123_EVENT_UPDATE, self->_event_param);
            }

            return;
        }
        default:
        {
            self->_state = BC_ZSSC3123_STATE_ERROR;

            goto start;
        }
    }
}
