#include <bc_sam_m8.h>
#include <minmea.h>
#include <bc_gpio.h>

#define _BC_SAM_M8_AVAILIBLE_BYTES 0xfd

#define _BC_SAM_M8_ENABLE_PIN BC_GPIO_P8

static void _bc_sam_m8_task(void *param);

static void _bc_sam_m8_parse(bc_sam_m8_t *self, const char *line);

static void _bc_sam_m8_feed(bc_sam_m8_t *self, char c);

static void _bc_sam_m8_clear(bc_sam_m8_t *self);

static bool _bc_sam_m8_read(bc_sam_m8_t *self, void *buffer, size_t size, size_t *length);

static void _bc_sam_m8_enable(void);

static void _bc_sam_m8_disable(void);

void bc_sam_m8_init(bc_sam_m8_t *self, bc_i2c_channel_t channel, uint8_t i2c_address, uint8_t i2c_address_expander, uint8_t expander_pin)
{
    memset(self, 0, sizeof(bc_sam_m8_t));

    bc_gpio_init(_BC_SAM_M8_ENABLE_PIN);

    bc_i2c_init(BC_I2C_I2C0, BC_I2C_SPEED_100_KHZ);

    self->_i2c_channel = channel;

    self->_i2c_address = i2c_address;

    self->_i2c_address_expander = i2c_address_expander;

    self->_expander_pin = expander_pin;

    self->_task_id = bc_scheduler_register(_bc_sam_m8_task, self, BC_TICK_INFINITY);
}

void bc_sam_m8_set_event_handler(bc_sam_m8_t *self, bc_sam_m8_event_handler_t event_handler, void *event_param)
{
    self->_event_handler = event_handler;

    self->_event_param = event_param;
}

void bc_sam_m8_start(bc_sam_m8_t *self, uint64_t milliseconds)
{
    uint64_t timeout = milliseconds == 0 ? 0 : bc_tick_get() + milliseconds;

    if (!self->_running)
    {
        _bc_sam_m8_clear(self);

        self->_timeout = timeout;

        self->_running = true;

        bc_scheduler_plan_now(self->_task_id);
    }
    else
    {
        if (self->_timeout < timeout || timeout == 0)
        {
            self->_timeout = timeout;
        }
    }
}

void bc_sam_m8_stop(bc_sam_m8_t *self)
{
    self->_running = false;

    bc_scheduler_plan_now(self->_task_id);
}

static void _bc_sam_m8_task(void *param)
{
    bc_sam_m8_t *self = param;

    if (!self->_running)
    {        
        self->_state = BC_SAM_M8_STATE_STOP;
    }
    
    if (self->_timeout < bc_tick_get())
    {        
        self->_state = BC_SAM_M8_STATE_TIMEOUT;
    }

    start:

    switch (self->_state)
    {

        case BC_SAM_M8_STATE_ERROR:

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_SAM_M8_STATE_STOP;

            goto start;

        case BC_SAM_M8_STATE_START:

            _bc_sam_m8_enable();

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8_EVENT_START, self->_event_param);
            }

            self->_state = BC_SAM_M8_STATE_READ;

            bc_scheduler_plan_current_relative(2000);

            break;

        case BC_SAM_M8_STATE_STOP:

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8_EVENT_STOP, self->_event_param);
            }

            self->_running = false;

            _bc_sam_m8_disable();

            break;

        case BC_SAM_M8_STATE_READ:

            if (_bc_sam_m8_read(self, self->_ddc_buffer, sizeof(self->_ddc_buffer), &self->_ddc_length))
            {
                for (size_t i = 0; i < self->_ddc_length; i++)
                {
                    _bc_sam_m8_feed(self, self->_ddc_buffer[i]);
                }
            }

            if (self->_state == BC_SAM_M8_STATE_UPDATE)
            {
                goto start;
            }
            else
            {
                bc_scheduler_plan_current_relative(100);
            }

            break;
            
        case BC_SAM_M8_STATE_UPDATE:

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8_EVENT_UPDATE, self->_event_param);
            }

            self->_state = BC_SAM_M8_STATE_STOP;

            goto start;

        case BC_SAM_M8_STATE_TIMEOUT:

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8_EVENT_TIMEOUT, self->_event_param);
            }

            self->_state = BC_SAM_M8_STATE_STOP;

            goto start;

        default:

            self->_state = BC_SAM_M8_STATE_ERROR;

            goto start;
    }
}

static void _bc_sam_m8_parse(bc_sam_m8_t *self, const char *line)
{
    self->_gnss_nmea_sentences++;

    if (minmea_sentence_id(line, true) == MINMEA_SENTENCE_RMC)
    {
        struct minmea_sentence_rmc frame;

        if (minmea_parse_rmc(&frame, line))
        {
            if (frame.valid)
            {
                self->_latitude = minmea_tocoord(&frame.latitude);

                self->_longitude = minmea_tocoord(&frame.longitude);

                self->_state = BC_SAM_M8_STATE_UPDATE;
            }
        }
    }
}

static void _bc_sam_m8_feed(bc_sam_m8_t *self, char c)
{
    if (c == '\r' || c == '\n')
    {
        if (self->_line_length != 0)
        {
            if (!self->_line_clipped)
            {
                _bc_sam_m8_parse(self, self->_line_buffer);
            }

            _bc_sam_m8_clear(self);
        }
    }
    else
    {
        if (self->_line_length < sizeof(self->_line_buffer) - 1)
        {
            self->_line_buffer[self->_line_length++] = c;
        }
        else
        {
            self->_line_clipped = true;
        }
    }
}

static void _bc_sam_m8_clear(bc_sam_m8_t *self)
{
    memset(self->_line_buffer, 0, sizeof(self->_line_buffer));

    self->_line_length = 0;

    self->_line_clipped = false;
}

static bool _bc_sam_m8_read(bc_sam_m8_t *self, void *buffer, size_t buffer_size, size_t *length)
{
    uint16_t bytes_available;

    if (!bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, _BC_SAM_M8_AVAILIBLE_BYTES, &bytes_available))
    {
        return false;
    }

    *length = bytes_available;

    if (*length > buffer_size)
    {
        *length = buffer_size;
    }

    if (*length > 64)
    {
        *length = 64;
    }

    if (*length != 0)
    {
        memset(buffer, 0, buffer_size);

        bc_i2c_memory_transfer_t transfer;

        transfer.device_address = self->_i2c_address;

        transfer.memory_address = 0xff;

        transfer.buffer = buffer;

        transfer.length = *length;

        if (!bc_i2c_memory_read(self->_i2c_channel, &transfer))
        {
            return false;
        }
    }

    return true;
}

static void _bc_sam_m8_enable(void)
{
    // TODO:
}

static void _bc_sam_m8_disable(void)
{
    // TODO:
}