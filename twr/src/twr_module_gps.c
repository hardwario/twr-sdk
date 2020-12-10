#include <twr_module_gps.h>
#include <twr_tca9534a.h>

static struct
{
    twr_module_gps_event_handler_t *event_handler;
    twr_sam_m8q_driver_t sam_m8q_driver;
    twr_sam_m8q_t sam_m8q;
    twr_tca9534a_t tca9534;
    void *event_param;

} _twr_module_gps;

static twr_tca9534a_pin_t _twr_module_gps_led_pin_lut[2] =
{
        [TWR_MODULE_GPS_LED_RED] = TWR_TCA9534A_PIN_P3,
        [TWR_MODULE_GPS_LED_GREEN] = TWR_TCA9534A_PIN_P5
};

static void _twr_module_gps_led_init(twr_led_t *self);
static void _twr_module_gps_led_on(twr_led_t *self);
static void _twr_module_gps_led_off(twr_led_t *self);
static void _twr_module_gps_sam_m8q_event_handler(twr_sam_m8q_t *self, twr_sam_m8q_event_t event, void *event_param);
static bool _twr_module_gps_sam_m8q_on(twr_sam_m8q_t *self);
static bool _twr_module_gps_sam_m8q_off(twr_sam_m8q_t *self);

bool twr_module_gps_init(void)
{
    memset(&_twr_module_gps, 0, sizeof(_twr_module_gps));

    _twr_module_gps.sam_m8q_driver.on = _twr_module_gps_sam_m8q_on;
    _twr_module_gps.sam_m8q_driver.off = _twr_module_gps_sam_m8q_off;

    if (!twr_tca9534a_init(&_twr_module_gps.tca9534, TWR_I2C_I2C0, 0x21))
    {
        return false;
    }

    if (!twr_tca9534a_write_port(&_twr_module_gps.tca9534, 0))
    {
        return false;
    }

    if (!twr_tca9534a_set_port_direction(&_twr_module_gps.tca9534, 0))
    {
        return false;
    }

    if (!twr_tca9534a_write_pin(&_twr_module_gps.tca9534, TWR_TCA9534A_PIN_P7, 1))
    {
        return false;
    }

    twr_sam_m8q_init(&_twr_module_gps.sam_m8q, TWR_I2C_I2C0, 0x42, &_twr_module_gps.sam_m8q_driver);
    twr_sam_m8q_set_event_handler(&_twr_module_gps.sam_m8q, _twr_module_gps_sam_m8q_event_handler, NULL);

    return true;
}

void twr_module_gps_set_event_handler(twr_module_gps_event_handler_t event_handler, void *event_param)
{
    _twr_module_gps.event_handler = event_handler;
    _twr_module_gps.event_param = event_param;
}

void twr_module_gps_start(void)
{
    twr_sam_m8q_start(&_twr_module_gps.sam_m8q);
}

void twr_module_gps_stop(void)
{
    twr_sam_m8q_stop(&_twr_module_gps.sam_m8q);
}

void twr_module_gps_invalidate(void)
{
    twr_sam_m8q_invalidate(&_twr_module_gps.sam_m8q);
}

bool twr_module_gps_get_time(twr_module_gps_time_t *time)
{
    return twr_sam_m8q_get_time(&_twr_module_gps.sam_m8q, time);
}

bool twr_module_gps_get_position(twr_module_gps_position_t *position)
{
    return twr_sam_m8q_get_position(&_twr_module_gps.sam_m8q, position);
}

bool twr_module_gps_get_altitude(twr_module_gps_altitude_t *altitude)
{
    return twr_sam_m8q_get_altitude(&_twr_module_gps.sam_m8q, altitude);
}

bool twr_module_gps_get_quality(twr_module_gps_quality_t *quality)
{
    return twr_sam_m8q_get_quality(&_twr_module_gps.sam_m8q, quality);
}

bool twr_module_gps_get_accuracy(twr_module_gps_accuracy_t *accuracy)
{
    return twr_sam_m8q_get_accuracy(&_twr_module_gps.sam_m8q, accuracy);
}

const twr_led_driver_t *twr_module_gps_get_led_driver(void)
{
    static const twr_led_driver_t twr_module_gps_led_driver =
    {
        .init = _twr_module_gps_led_init,
        .on = _twr_module_gps_led_on,
        .off = _twr_module_gps_led_off,
    };

    return &twr_module_gps_led_driver;
}

static void _twr_module_gps_led_init(twr_led_t *self)
{
    (void) self;
}

static void _twr_module_gps_led_on(twr_led_t *self)
{
    if (!twr_tca9534a_write_pin(&_twr_module_gps.tca9534, _twr_module_gps_led_pin_lut[self->_channel.virtual], self->_idle_state ? 0 : 1))
    {
        // TODO
    }
}

static void _twr_module_gps_led_off(twr_led_t *self)
{
    if (!twr_tca9534a_write_pin(&_twr_module_gps.tca9534, _twr_module_gps_led_pin_lut[self->_channel.virtual], self->_idle_state ? 1 : 0))
    {
        // TODO
    }
}

static void _twr_module_gps_sam_m8q_event_handler(twr_sam_m8q_t *self, twr_sam_m8q_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_twr_module_gps.event_handler == NULL)
    {
        return;
    }

    if (event == TWR_SAM_M8Q_EVENT_ERROR)
    {
        _twr_module_gps.event_handler(TWR_MODULE_GPS_EVENT_ERROR, _twr_module_gps.event_param);
    }
    else if (event == TWR_SAM_M8Q_EVENT_START)
    {
        _twr_module_gps.event_handler(TWR_MODULE_GPS_EVENT_START, _twr_module_gps.event_param);
    }
    else if (event == TWR_SAM_M8Q_EVENT_UPDATE)
    {
        _twr_module_gps.event_handler(TWR_MODULE_GPS_EVENT_UPDATE, _twr_module_gps.event_param);
    }
    else if (event == TWR_SAM_M8Q_EVENT_STOP)
    {
        _twr_module_gps.event_handler(TWR_MODULE_GPS_EVENT_STOP, _twr_module_gps.event_param);
    }
}

static bool _twr_module_gps_sam_m8q_on(twr_sam_m8q_t *self)
{
    (void) self;

    return twr_tca9534a_write_pin(&_twr_module_gps.tca9534, TWR_TCA9534A_PIN_P0, 1);
}

static bool _twr_module_gps_sam_m8q_off(twr_sam_m8q_t *self)
{
    (void) self;

    return twr_tca9534a_write_pin(&_twr_module_gps.tca9534, TWR_TCA9534A_PIN_P0, 0);
}
