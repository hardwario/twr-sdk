#include <hio_zssc3123.h>
#include <hio_timer.h>
#include <hio_log.h>

static uint8_t _hio_zssc3123_get_response(hio_zssc3123_t *self);
static void _hio_zssc3123_task(void *param);

bool hio_zssc3123_init(hio_zssc3123_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    hio_timer_init();

    hio_i2c_init(self->_i2c_channel, HIO_I2C_SPEED_400_KHZ);

    self->_update_interval = HIO_TICK_INFINITY;

    self->_data_fetch_delay = 100;

    self->_task_id = hio_scheduler_register(_hio_zssc3123_task, self, HIO_TICK_INFINITY);

    return true;
}

bool hio_zssc3123_deinit(hio_zssc3123_t *self)
{
    hio_i2c_deinit(self->_i2c_channel);

    hio_scheduler_unregister(self->_task_id);

    return true;
}

void hio_zssc3123_set_data_fetch_delay(hio_zssc3123_t *self, hio_tick_t interval)
{
    self->_data_fetch_delay = interval;
}

void hio_zssc3123_set_event_handler(hio_zssc3123_t *self, void (*event_handler)(hio_zssc3123_t *, hio_zssc3123_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_zssc3123_set_update_interval(hio_zssc3123_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        if (!self->_measurement_active)
        {
            hio_scheduler_plan_absolute(self->_task_id, HIO_TICK_INFINITY);
        }
    }
    else
    {
        hio_zssc3123_measure(self);
    }
}

bool hio_zssc3123_measure(hio_zssc3123_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    hio_scheduler_plan_now(self->_task_id);

    return true;
}

bool hio_zssc3123_get_raw_cap_data(hio_zssc3123_t *self, uint16_t *raw)
{
    if (!self->_valid)
    {
        return false;
    }

    *raw = self->_raw;

    return true;
}

bool hio_zssc3123_start_cm(hio_zssc3123_t *self)
{
    if (!hio_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0xA0, 0x0000))
    {
        return false;
    }

    return (_hio_zssc3123_get_response(self) >> 6) == 2;
}

bool hio_zssc3123_end_cm(hio_zssc3123_t *self)
{
    if (!hio_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x80, 0x0000))
    {
        return false;
    }

    return true;
}

bool hio_zssc3123_eeprom_read(hio_zssc3123_t *self, uint8_t adr, uint16_t *word)
{
    if (!hio_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, adr, 0x0000))
    {
        return false;
    }

    hio_timer_start();

    hio_timer_delay(90);

    hio_timer_stop();

    uint8_t buffer[3];

    hio_i2c_transfer_t transfer;

    transfer.device_address = self->_i2c_address;

    transfer.buffer = buffer;

    transfer.length = sizeof(buffer);

    if (!hio_i2c_read(self->_i2c_channel, &transfer))
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

bool hio_zssc3123_eeprom_write(hio_zssc3123_t *self, uint8_t address, uint16_t word)
{
    if (!hio_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0x40 + address, word))
    {
        return false;
    }

    hio_timer_start();

    hio_timer_delay(12000);

    hio_timer_stop();

    return (_hio_zssc3123_get_response(self) & 0x03) == 1;
}

bool hio_zssc3123_unlock_eeprom(hio_zssc3123_t *self)
{
    if (!hio_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0xa2, 0x0000))
    {
        return false;
    }

    if (!hio_i2c_memory_write_16b(self->_i2c_channel, self->_i2c_address, 0xf0, 0x0021))
    {
        return false;
    }

    return true;
}

static uint8_t _hio_zssc3123_get_response(hio_zssc3123_t *self)
{
    hio_i2c_transfer_t transfer;

    uint8_t data;

    transfer.device_address = self->_i2c_address;

    transfer.buffer = &data;

    transfer.length = 1;

    if (!hio_i2c_read(self->_i2c_channel, &transfer))
    {
        return 0xff;
    }
    else
    {
        return data;
    }
}

static bool _hio_zssc3123_data_fetch(hio_zssc3123_t *self)
{
    hio_i2c_transfer_t transfer;

    uint8_t buffer[2];

    transfer.device_address = self->_i2c_address;

    transfer.buffer = buffer;

    transfer.length = sizeof(buffer);

    if (!hio_i2c_read(self->_i2c_channel, &transfer))
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

static bool _hio_zssc3123_measurement_request(hio_zssc3123_t *self)
{
    hio_i2c_transfer_t transfer;

    uint8_t buffer[1] = { self->_i2c_address << 1 };

    transfer.device_address = self->_i2c_address;

    transfer.buffer = buffer;

    transfer.length = sizeof(buffer);

    if (!hio_i2c_write(self->_i2c_channel, &transfer))
    {
        return false;
    }

    return true;
}

static void _hio_zssc3123_task(void *param)
{
    hio_zssc3123_t *self = param;

start:

    switch (self->_state)
    {
        case HIO_ZSSC3123_STATE_ERROR:
        {
            self->_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_ZSSC3123_EVENT_ERROR, self->_event_param);
            }

            self->_state = HIO_ZSSC3123_STATE_INITIALIZE;

            hio_scheduler_plan_current_absolute(self->_next_update_start);

            return;
        }
        case HIO_ZSSC3123_STATE_INITIALIZE:
        {
            self->_next_update_start = hio_tick_get() + self->_update_interval;

            if (!_hio_zssc3123_data_fetch(self))
            {
                self->_state = HIO_ZSSC3123_STATE_ERROR;

                goto start;
            }

            self->_state = HIO_ZSSC3123_STATE_MEASURE;

            hio_scheduler_plan_current_now();

            return;
        }
        case HIO_ZSSC3123_STATE_MEASURE:
        {
            self->_next_update_start = hio_tick_get() + self->_update_interval;

            self->_measurement_active = true;

            self->_valid = false;

            if (!_hio_zssc3123_measurement_request(self))
            {
                self->_state = HIO_ZSSC3123_STATE_ERROR;

                goto start;
            }

            self->_state = HIO_ZSSC3123_STATE_READ;

            hio_scheduler_plan_current_from_now(self->_data_fetch_delay);

            return;
        }
        case HIO_ZSSC3123_STATE_READ:
        {
            if (!_hio_zssc3123_data_fetch(self))
            {
                self->_state = HIO_ZSSC3123_STATE_ERROR;

                goto start;
            }

            self->_measurement_active = false;

            self->_state = HIO_ZSSC3123_STATE_MEASURE;

            hio_scheduler_plan_current_absolute(self->_next_update_start);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_ZSSC3123_EVENT_UPDATE, self->_event_param);
            }

            return;
        }
        default:
        {
            self->_state = HIO_ZSSC3123_STATE_ERROR;

            goto start;
        }
    }
}
