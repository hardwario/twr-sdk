#include <twr_sam_m8q.h>
#include <twr_gpio.h>
#include <minmea.h>

static void _twr_sam_m8q_task(void *param);
static bool _twr_sam_m8q_parse(twr_sam_m8q_t *self, const char *line);
static bool _twr_sam_m8q_feed(twr_sam_m8q_t *self, char c);
static void _twr_sam_m8q_clear(twr_sam_m8q_t *self);
static bool _twr_sam_m8q_enable(twr_sam_m8q_t *self);
static bool _twr_sam_m8q_disable(twr_sam_m8q_t *self);
static bool _twr_sam_m8q_send_config(twr_sam_m8q_t *self);

void twr_sam_m8q_init(twr_sam_m8q_t *self, twr_i2c_channel_t channel, uint8_t i2c_address, const twr_sam_m8q_driver_t *driver)
{
    memset(self, 0, sizeof(twr_sam_m8q_t));

    twr_i2c_init(TWR_I2C_I2C0, TWR_I2C_SPEED_100_KHZ);

    self->_i2c_channel = channel;
    self->_i2c_address = i2c_address;
    self->_driver = driver;

    self->_task_id = twr_scheduler_register(_twr_sam_m8q_task, self, TWR_TICK_INFINITY);
}

void twr_sam_m8q_set_event_handler(twr_sam_m8q_t *self, twr_sam_m8q_event_handler_t event_handler, void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_sam_m8q_start(twr_sam_m8q_t *self)
{
    if (!self->_running)
    {
        self->_running = true;
        self->_configured = false;

        twr_scheduler_plan_now(self->_task_id);
    }
}

void twr_sam_m8q_stop(twr_sam_m8q_t *self)
{
    if (self->_running)
    {
        self->_running = false;

        twr_scheduler_plan_now(self->_task_id);
    }
}

void twr_sam_m8q_invalidate(twr_sam_m8q_t *self)
{
    self->_rmc.valid = false;
    self->_gga.valid = false;
    self->_pubx.valid = false;
}

bool twr_sam_m8q_get_time(twr_sam_m8q_t *self, twr_sam_m8q_time_t *time)
{
    memset(time, 0, sizeof(*time));

    if (!self->_rmc.valid)
    {
        return false;
    }

    time->year = self->_rmc.date.year + 2000;
    time->month = self->_rmc.date.month;
    time->day = self->_rmc.date.day;
    time->hours = self->_rmc.time.hours;
    time->minutes = self->_rmc.time.minutes;
    time->seconds = self->_rmc.time.seconds;

    return true;
}

bool twr_sam_m8q_get_position(twr_sam_m8q_t *self, twr_sam_m8q_position_t *position)
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

bool twr_sam_m8q_get_altitude(twr_sam_m8q_t *self, twr_sam_m8q_altitude_t *altitude)
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

bool twr_sam_m8q_get_quality(twr_sam_m8q_t *self, twr_sam_m8q_quality_t *quality)
{
    memset(quality, 0, sizeof(*quality));

    if (!self->_gga.valid || !self->_pubx.valid)
    {
        return false;
    }

    quality->fix_quality = self->_gga.fix_quality;
    quality->satellites_tracked = self->_pubx.satellites;

    return true;
}

bool twr_sam_m8q_get_accuracy(twr_sam_m8q_t *self, twr_sam_m8q_accuracy_t *accuracy)
{
    memset(accuracy, 0, sizeof(*accuracy));

    if (!self->_pubx.valid || self->_gga.fix_quality < 1)
    {
        return false;
    }

    accuracy->horizontal = self->_pubx.h_accuracy;
    accuracy->vertical = self->_pubx.v_accuracy;

    return true;
}

static void _twr_sam_m8q_task(void *param)
{
    twr_sam_m8q_t *self = param;

    if (!self->_running)
    {
        self->_state = TWR_SAM_M8Q_STATE_STOP;
    }

    if (self->_running && self->_state == TWR_SAM_M8Q_STATE_STOP)
    {
        self->_state = TWR_SAM_M8Q_STATE_START;
    }

start:

    switch (self->_state)
    {
        case TWR_SAM_M8Q_STATE_ERROR:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_SAM_M8Q_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_SAM_M8Q_STATE_STOP;

            goto start;
        }
        case TWR_SAM_M8Q_STATE_START:
        {
            if (!_twr_sam_m8q_enable(self))
            {
                self->_state = TWR_SAM_M8Q_STATE_ERROR;

                goto start;
            }

            _twr_sam_m8q_clear(self);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_SAM_M8Q_EVENT_START, self->_event_param);
            }

            self->_state = TWR_SAM_M8Q_STATE_READ;

            twr_scheduler_plan_current_relative(2000);

            break;
        }
        case TWR_SAM_M8Q_STATE_READ:
        {
            uint16_t bytes_available;

            if (!twr_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0xfd, &bytes_available))
            {
                self->_state = TWR_SAM_M8Q_STATE_ERROR;

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

                twr_i2c_memory_transfer_t transfer;

                transfer.device_address = self->_i2c_address;
                transfer.memory_address = 0xff;
                transfer.buffer = self->_ddc_buffer;
                transfer.length = self->_ddc_length;

                if (!twr_i2c_memory_read(self->_i2c_channel, &transfer))
                {
                    self->_state = TWR_SAM_M8Q_STATE_ERROR;

                    goto start;
                }

                for (size_t i = 0; i < self->_ddc_length; i++)
                {
                    if (_twr_sam_m8q_feed(self, self->_ddc_buffer[i]))
                    {
                        self->_state = TWR_SAM_M8Q_STATE_UPDATE;
                    }
                }

                bytes_available -= self->_ddc_length;
            }

            if (self->_state == TWR_SAM_M8Q_STATE_UPDATE)
            {
                goto start;
            }

            twr_scheduler_plan_current_relative(100);

            break;
        }
        case TWR_SAM_M8Q_STATE_UPDATE:
        {
            self->_state = TWR_SAM_M8Q_STATE_READ;

            twr_scheduler_plan_current_relative(100);

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_SAM_M8Q_EVENT_UPDATE, self->_event_param);
            }

            goto start;
        }
        case TWR_SAM_M8Q_STATE_STOP:
        {
            self->_running = false;

            if (!_twr_sam_m8q_disable(self))
            {
                self->_state = TWR_SAM_M8Q_STATE_ERROR;

                goto start;
            }

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_SAM_M8Q_EVENT_STOP, self->_event_param);
            }

            break;
        }
        default:
        {
            self->_state = TWR_SAM_M8Q_STATE_ERROR;

            goto start;
        }
    }
}

static bool _twr_sam_m8q_parse(twr_sam_m8q_t *self, const char *line)
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
            self->_gga.altitude = minmea_tofloat(&frame.altitude);
            self->_gga.altitude_units = frame.altitude_units;
            self->_gga.valid = true;

            ret = true;
        }
    }
    else if (id == MINMEA_SENTENCE_PUBX)
    {
        struct minmea_sentence_pubx frame;

        if (minmea_parse_pubx(&frame, line))
        {
            self->_pubx.h_accuracy = minmea_tofloat(&frame.h_accuracy);
            self->_pubx.v_accuracy = minmea_tofloat(&frame.v_accuracy);
            self->_pubx.speed = minmea_tofloat(&frame.speed);
            self->_pubx.course = minmea_tofloat(&frame.course);
            self->_pubx.satellites = frame.satellites;
            self->_pubx.valid = true;

            ret = true;
        }
    }

    return ret;
}

static bool _twr_sam_m8q_feed(twr_sam_m8q_t *self, char c)
{
    bool ret = false;

    if (c == '\r' || c == '\n')
    {
        if (self->_line_length != 0)
        {
            if (!self->_line_clipped)
            {
                if (_twr_sam_m8q_parse(self, self->_line_buffer))
                {
                    ret = true;
                }
            }

            _twr_sam_m8q_clear(self);

            if (!self->_configured)
            {
                if (_twr_sam_m8q_send_config(self))
                {
                    self->_configured = true;
                }
            }
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

static void _twr_sam_m8q_clear(twr_sam_m8q_t *self)
{
    memset(self->_line_buffer, 0, sizeof(self->_line_buffer));

    self->_line_clipped = false;
    self->_line_length = 0;
}

static bool _twr_sam_m8q_enable(twr_sam_m8q_t *self)
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

static bool _twr_sam_m8q_disable(twr_sam_m8q_t *self)
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

static bool _twr_sam_m8q_send_config(twr_sam_m8q_t *self)
{
    // Enable PUBX POSITION message
    uint8_t config_msg_pubx[] = {
        0xb5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xf1, 0x00,
        0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x04, 0x3b
    };
    twr_i2c_transfer_t transfer;

    transfer.device_address = self->_i2c_address;
    transfer.buffer = config_msg_pubx;
    transfer.length = sizeof(config_msg_pubx);

    if (!twr_i2c_write(self->_i2c_channel, &transfer))
    {
        return false;
    }

    // Disable GSA message
    uint8_t config_msg_gsa[] = {
        0xb5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xf0, 0x02,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31,
    };

    transfer.device_address = self->_i2c_address;
    transfer.buffer = config_msg_gsa;
    transfer.length = sizeof(config_msg_gsa);

    if (!twr_i2c_write(self->_i2c_channel, &transfer))
    {
        return false;
    }

    // Disable GSV message
    uint8_t config_msg_gsv[] = {
        0xb5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xf0, 0x03,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38,
    };

    transfer.device_address = self->_i2c_address;
    transfer.buffer = config_msg_gsv;
    transfer.length = sizeof(config_msg_gsv);

    if (!twr_i2c_write(self->_i2c_channel, &transfer))
    {
        return false;
    }

    // Enable Galileo
    uint8_t config_gnss[] = {
        0xb5, 0x62, 0x06, 0x3e, 0x3c, 0x00, 0x00, 0x20,
        0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00,
        0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x01, 0x00,
        0x01, 0x01, 0x02, 0x04, 0x08, 0x00, 0x01, 0x00,
        0x01, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00,
        0x01, 0x01, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x01, 0x03, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x01, 0x05, 0x06, 0x08, 0x0e, 0x00, 0x01, 0x00,
        0x01, 0x01, 0x55, 0x47,
    };

    transfer.device_address = self->_i2c_address;
    transfer.buffer = config_gnss;
    transfer.length = sizeof(config_gnss);

    if (!twr_i2c_write(self->_i2c_channel, &transfer))
    {
        return false;
    }

    // Set NMEA version to 4.1
    uint8_t config_nmea[] = {
        0xb5, 0x62, 0x06, 0x17, 0x14, 0x00, 0x00, 0x41,
        0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x75, 0x57,
    };

    transfer.device_address = self->_i2c_address;
    transfer.buffer = config_nmea;
    transfer.length = sizeof(config_nmea);

    if (!twr_i2c_write(self->_i2c_channel, &transfer))
    {
        return false;
    }

    return true;
}
