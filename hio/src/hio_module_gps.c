#include <hio_module_gps.h>
#include <hio_tca9534a.h>

static struct
{
    hio_module_gps_event_handler_t *event_handler;
    hio_sam_m8q_driver_t sam_m8q_driver;
    hio_sam_m8q_t sam_m8q;
    hio_tca9534a_t tca9534;
    void *event_param;

} _hio_module_gps;

static hio_tca9534a_pin_t _hio_module_gps_led_pin_lut[2] =
{
        [HIO_MODULE_GPS_LED_RED] = HIO_TCA9534A_PIN_P3,
        [HIO_MODULE_GPS_LED_GREEN] = HIO_TCA9534A_PIN_P5
};

static void _hio_module_gps_led_init(hio_led_t *self);
static void _hio_module_gps_led_on(hio_led_t *self);
static void _hio_module_gps_led_off(hio_led_t *self);
static void _hio_module_gps_sam_m8q_event_handler(hio_sam_m8q_t *self, hio_sam_m8q_event_t event, void *event_param);
static bool _hio_module_gps_sam_m8q_on(hio_sam_m8q_t *self);
static bool _hio_module_gps_sam_m8q_off(hio_sam_m8q_t *self);

bool hio_module_gps_init(void)
{
    memset(&_hio_module_gps, 0, sizeof(_hio_module_gps));

    _hio_module_gps.sam_m8q_driver.on = _hio_module_gps_sam_m8q_on;
    _hio_module_gps.sam_m8q_driver.off = _hio_module_gps_sam_m8q_off;

    if (!hio_tca9534a_init(&_hio_module_gps.tca9534, HIO_I2C_I2C0, 0x21))
    {
        return false;
    }

    if (!hio_tca9534a_write_port(&_hio_module_gps.tca9534, 0))
    {
        return false;
    }

    if (!hio_tca9534a_set_port_direction(&_hio_module_gps.tca9534, 0))
    {
        return false;
    }

    if (!hio_tca9534a_write_pin(&_hio_module_gps.tca9534, HIO_TCA9534A_PIN_P7, 1))
    {
        return false;
    }

    hio_sam_m8q_init(&_hio_module_gps.sam_m8q, HIO_I2C_I2C0, 0x42, &_hio_module_gps.sam_m8q_driver);
    hio_sam_m8q_set_event_handler(&_hio_module_gps.sam_m8q, _hio_module_gps_sam_m8q_event_handler, NULL);

    return true;
}

void hio_module_gps_set_event_handler(hio_module_gps_event_handler_t event_handler, void *event_param)
{
    _hio_module_gps.event_handler = event_handler;
    _hio_module_gps.event_param = event_param;
}

void hio_module_gps_start(void)
{
    hio_sam_m8q_start(&_hio_module_gps.sam_m8q);
}

void hio_module_gps_stop(void)
{
    hio_sam_m8q_stop(&_hio_module_gps.sam_m8q);
}

void hio_module_gps_invalidate(void)
{
    hio_sam_m8q_invalidate(&_hio_module_gps.sam_m8q);
}

bool hio_module_gps_get_time(hio_module_gps_time_t *time)
{
    return hio_sam_m8q_get_time(&_hio_module_gps.sam_m8q, time);
}

bool hio_module_gps_get_position(hio_module_gps_position_t *position)
{
    return hio_sam_m8q_get_position(&_hio_module_gps.sam_m8q, position);
}

bool hio_module_gps_get_altitude(hio_module_gps_altitude_t *altitude)
{
    return hio_sam_m8q_get_altitude(&_hio_module_gps.sam_m8q, altitude);
}

bool hio_module_gps_get_quality(hio_module_gps_quality_t *quality)
{
    return hio_sam_m8q_get_quality(&_hio_module_gps.sam_m8q, quality);
}

bool hio_module_gps_get_accuracy(hio_module_gps_accuracy_t *accuracy)
{
    return hio_sam_m8q_get_accuracy(&_hio_module_gps.sam_m8q, accuracy);
}

const hio_led_driver_t *hio_module_gps_get_led_driver(void)
{
    static const hio_led_driver_t hio_module_gps_led_driver =
    {
        .init = _hio_module_gps_led_init,
        .on = _hio_module_gps_led_on,
        .off = _hio_module_gps_led_off,
    };

    return &hio_module_gps_led_driver;
}

static void _hio_module_gps_led_init(hio_led_t *self)
{
    (void) self;
}

static void _hio_module_gps_led_on(hio_led_t *self)
{
    if (!hio_tca9534a_write_pin(&_hio_module_gps.tca9534, _hio_module_gps_led_pin_lut[self->_channel.virtual], self->_idle_state ? 0 : 1))
    {
        // TODO
    }
}

static void _hio_module_gps_led_off(hio_led_t *self)
{
    if (!hio_tca9534a_write_pin(&_hio_module_gps.tca9534, _hio_module_gps_led_pin_lut[self->_channel.virtual], self->_idle_state ? 1 : 0))
    {
        // TODO
    }
}

static void _hio_module_gps_sam_m8q_event_handler(hio_sam_m8q_t *self, hio_sam_m8q_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_hio_module_gps.event_handler == NULL)
    {
        return;
    }

    if (event == HIO_SAM_M8Q_EVENT_ERROR)
    {
        _hio_module_gps.event_handler(HIO_MODULE_GPS_EVENT_ERROR, _hio_module_gps.event_param);
    }
    else if (event == HIO_SAM_M8Q_EVENT_START)
    {
        _hio_module_gps.event_handler(HIO_MODULE_GPS_EVENT_START, _hio_module_gps.event_param);
    }
    else if (event == HIO_SAM_M8Q_EVENT_UPDATE)
    {
        _hio_module_gps.event_handler(HIO_MODULE_GPS_EVENT_UPDATE, _hio_module_gps.event_param);
    }
    else if (event == HIO_SAM_M8Q_EVENT_STOP)
    {
        _hio_module_gps.event_handler(HIO_MODULE_GPS_EVENT_STOP, _hio_module_gps.event_param);
    }
}

static bool _hio_module_gps_sam_m8q_on(hio_sam_m8q_t *self)
{
    (void) self;

    return hio_tca9534a_write_pin(&_hio_module_gps.tca9534, HIO_TCA9534A_PIN_P0, 1);
}

static bool _hio_module_gps_sam_m8q_off(hio_sam_m8q_t *self)
{
    (void) self;

    return hio_tca9534a_write_pin(&_hio_module_gps.tca9534, HIO_TCA9534A_PIN_P0, 0);
}
