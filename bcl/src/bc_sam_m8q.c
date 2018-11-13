#include <bc_sam_m8q.h>
#include <bc_gpio.h>
#include <minmea.h>

static void _bc_sam_m8q_task(void *param);
static bool _bc_sam_m8q_parse(bc_sam_m8q_t *self, const char *line);
static bool _bc_sam_m8q_feed(bc_sam_m8q_t *self, char c);
static void _bc_sam_m8q_clear(bc_sam_m8q_t *self);
static bool _bc_sam_m8q_enable(bc_sam_m8q_t *self);
static bool _bc_sam_m8q_disable(bc_sam_m8q_t *self);

void bc_sam_m8q_init(bc_sam_m8q_t *self, bc_i2c_channel_t channel, uint8_t i2c_address, const bc_sam_m8q_driver_t *driver)
{
    memset(self, 0, sizeof(bc_sam_m8q_t));

    bc_i2c_init(BC_I2C_I2C0, BC_I2C_SPEED_100_KHZ);

    self->_i2c_channel = channel;
    self->_i2c_address = i2c_address;
    self->_driver = driver;

    self->_task_id = bc_scheduler_register(_bc_sam_m8q_task, self, BC_TICK_INFINITY);
}

void bc_sam_m8q_set_event_handler(bc_sam_m8q_t *self, bc_sam_m8q_event_handler_t event_handler, void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_sam_m8q_start(bc_sam_m8q_t *self)
{
    if (!self->_running)
    {
        self->_running = true;

        bc_scheduler_plan_now(self->_task_id);
    }
}

void bc_sam_m8q_stop(bc_sam_m8q_t *self)
{
    if (self->_running)
    {
        self->_running = false;

        bc_scheduler_plan_now(self->_task_id);
    }
}

void bc_sam_m8q_invalidate(bc_sam_m8q_t *self)
{
    self->_rmc.valid = false;
    self->_gga.valid = false;
}

bool bc_sam_m8q_get_time(bc_sam_m8q_t *self, bc_sam_m8q_time_t *time)
{
    memset(time, 0, sizeof(*time));

    if (!self->_rmc.valid)
    {
        return false;
    }

    time->year = self->_rmc.date.year;
    time->month = self->_rmc.date.month;
    time->day = self->_rmc.date.day;
    time->hours = self->_rmc.time.hours;
    time->minutes = self->_rmc.time.minutes;
    time->seconds = self->_rmc.time.seconds;

    return true;
}

bool bc_sam_m8q_get_position(bc_sam_m8q_t *self, bc_sam_m8q_position_t *position)
{
    memset(position, 0, sizeof(*position));

    if (!self->_rmc.valid)
    {
        return false;
    }

    position->latitude = self->_rmc.latitude;
    position->longitude = self->_rmc.longitude;

    return true;
}

bool bc_sam_m8q_get_altitude(bc_sam_m8q_t *self, bc_sam_m8q_altitude_t *altitude)
{
    memset(altitude, 0, sizeof(*altitude));

    if (!self->_gga.valid || self->_gga.fix_quality < 1)
    {
        return false;
    }

    altitude->altitude = self->_gga.altitude;
    altitude->units = self->_gga.altitude_units;

    return true;
}

bool bc_sam_m8q_get_quality(bc_sam_m8q_t *self, bc_sam_m8q_quality_t *quality)
{
    memset(quality, 0, sizeof(*quality));

    if (!self->_gga.valid)
    {
        return false;
    }

    quality->fix_quality = self->_gga.fix_quality;
    quality->satellites_tracked = self->_gga.satellites_tracked;

    return true;
}

static void _bc_sam_m8q_task(void *param)
{
    bc_sam_m8q_t *self = param;

    if (!self->_running)
    {
        self->_state = BC_SAM_M8Q_STATE_STOP;
    }

start:

    switch (self->_state)
    {
        case BC_SAM_M8Q_STATE_ERROR:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8Q_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_SAM_M8Q_STATE_STOP;

            goto start;
        }
        case BC_SAM_M8Q_STATE_START:
        {
            if (!_bc_sam_m8q_enable(self))
            {
                self->_state = BC_SAM_M8Q_STATE_ERROR;

                goto start;
            }

            _bc_sam_m8q_clear(self);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8Q_EVENT_START, self->_event_param);
            }

            self->_state = BC_SAM_M8Q_STATE_READ;

            bc_scheduler_plan_current_relative(2000);

            break;
        }
        case BC_SAM_M8Q_STATE_READ:
        {
            uint16_t bytes_available;

            if (!bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0xfd, &bytes_available))
            {
                self->_state = BC_SAM_M8Q_STATE_ERROR;

                goto start;
            }

            while (bytes_available != 0)
            {
                self->_ddc_length = bytes_available;

                if (self->_ddc_length > sizeof(self->_ddc_buffer))
                {
                    self->_ddc_length = sizeof(self->_ddc_buffer);
                }

                memset(self->_ddc_buffer, 0, sizeof(self->_ddc_buffer));

                bc_i2c_memory_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.memory_address = 0xff;
                transfer.buffer = self->_ddc_buffer;
                transfer.length = self->_ddc_length;

                if (!bc_i2c_memory_read(self->_i2c_channel, &transfer))
                {
                    self->_state = BC_SAM_M8Q_STATE_ERROR;

                    goto start;
                }

                for (size_t i = 0; i < self->_ddc_length; i++)
                {
                    if (_bc_sam_m8q_feed(self, self->_ddc_buffer[i]))
                    {
                        self->_state = BC_SAM_M8Q_STATE_UPDATE;
                    }
                }

                bytes_available -= self->_ddc_length;
            }

            if (self->_state == BC_SAM_M8Q_STATE_UPDATE)
            {
                goto start;
            }

            bc_scheduler_plan_current_relative(100);

            break;
        }
        case BC_SAM_M8Q_STATE_UPDATE:
        {
            self->_state = BC_SAM_M8Q_STATE_READ;

            bc_scheduler_plan_current_relative(100);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8Q_EVENT_UPDATE, self->_event_param);
            }

            goto start;
        }
        case BC_SAM_M8Q_STATE_STOP:
        {
            self->_running = false;

            if (!_bc_sam_m8q_disable(self))
            {
                self->_state = BC_SAM_M8Q_STATE_ERROR;

                goto start;
            }

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_SAM_M8Q_EVENT_STOP, self->_event_param);
            }

            break;
        }
        default:
        {
            self->_state = BC_SAM_M8Q_STATE_ERROR;

            goto start;
        }
    }
}

static bool _bc_sam_m8q_parse(bc_sam_m8q_t *self, const char *line)
{
    bool ret = false;

    enum minmea_sentence_id id = minmea_sentence_id(line, true);

    if (id == MINMEA_SENTENCE_RMC)
    {
        struct minmea_sentence_rmc frame;

        if (minmea_parse_rmc(&frame, line))
        {
            if (frame.valid)
            {
                self->_rmc.date.year = frame.date.year;
                self->_rmc.date.month = frame.date.month;
                self->_rmc.date.day = frame.date.day;
                self->_rmc.time.hours = frame.time.hours;
                self->_rmc.time.minutes = frame.time.minutes;
                self->_rmc.time.seconds = frame.time.seconds;
                self->_rmc.latitude = minmea_tocoord(&frame.latitude);
                self->_rmc.longitude = minmea_tocoord(&frame.longitude);
                self->_rmc.valid = true;

                ret = true;
            }
        }
    }
    else if (id == MINMEA_SENTENCE_GGA)
    {
        struct minmea_sentence_gga frame;

        if (minmea_parse_gga(&frame, line))
        {
            self->_gga.fix_quality = frame.fix_quality;
            self->_gga.satellites_tracked = frame.satellites_tracked;
            self->_gga.altitude = minmea_tofloat(&frame.altitude);
            self->_gga.altitude_units = frame.altitude_units;
            self->_gga.valid = true;

            ret = true;
        }
    }

    return ret;
}

static bool _bc_sam_m8q_feed(bc_sam_m8q_t *self, char c)
{
    bool ret = false;

    if (c == '\r' || c == '\n')
    {
        if (self->_line_length != 0)
        {
            if (!self->_line_clipped)
            {
                if (_bc_sam_m8q_parse(self, self->_line_buffer))
                {
                    ret = true;
                }
            }

            _bc_sam_m8q_clear(self);
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

    return ret;
}

static void _bc_sam_m8q_clear(bc_sam_m8q_t *self)
{
    memset(self->_line_buffer, 0, sizeof(self->_line_buffer));

    self->_line_clipped = false;
    self->_line_length = 0;
}

static bool _bc_sam_m8q_enable(bc_sam_m8q_t *self)
{
    if (self->_driver != NULL)
    {
        if (!self->_driver->on(self))
        {
            return false;
        }
    }

    return true;
}

static bool _bc_sam_m8q_disable(bc_sam_m8q_t *self)
{
    if (self->_driver != NULL)
    {
        if (!self->_driver->off(self))
        {
            return false;
        }
    }

    return true;
}
