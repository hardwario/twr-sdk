#include <bc_module_gps.h>
#include <bc_tca9534a.h>

static struct
{
    bc_module_gps_event_handler_t *event_handler;
    bc_sam_m8q_driver_t sam_m8q_driver;
    bc_sam_m8q_t sam_m8q;
    bc_tca9534a_t tca9534;
    void *event_param;

} _bc_module_gps;

static bc_tca9534a_pin_t _bc_module_gps_led_pin_lut[2] =
{
        [BC_MODULE_GPS_LED_RED] = BC_TCA9534A_PIN_P3,
        [BC_MODULE_GPS_LED_GREEN] = BC_TCA9534A_PIN_P5
};

static void _bc_module_gps_led_init(bc_led_t *self);
static void _bc_module_gps_led_on(bc_led_t *self);
static void _bc_module_gps_led_off(bc_led_t *self);
static void _bc_module_gps_sam_m8q_event_handler(bc_sam_m8q_t *self, bc_sam_m8q_event_t event, void *event_param);
static bool _bc_module_gps_sam_m8q_on(bc_sam_m8q_t *self);
static bool _bc_module_gps_sam_m8q_off(bc_sam_m8q_t *self);

bool bc_module_gps_init(void)
{
    memset(&_bc_module_gps, 0, sizeof(_bc_module_gps));

    _bc_module_gps.sam_m8q_driver.on = _bc_module_gps_sam_m8q_on;
    _bc_module_gps.sam_m8q_driver.off = _bc_module_gps_sam_m8q_off;

    if (!bc_tca9534a_init(&_bc_module_gps.tca9534, BC_I2C_I2C0, 0x21))
    {
        return false;
    }

    if (!bc_tca9534a_write_port(&_bc_module_gps.tca9534, 0))
    {
        return false;
    }

    if (!bc_tca9534a_set_port_direction(&_bc_module_gps.tca9534, 0))
    {
        return false;
    }

    bc_sam_m8q_init(&_bc_module_gps.sam_m8q, BC_I2C_I2C0, 0x42, &_bc_module_gps.sam_m8q_driver);
    bc_sam_m8q_set_event_handler(&_bc_module_gps.sam_m8q, _bc_module_gps_sam_m8q_event_handler, NULL);

    return true;
}

void bc_module_gps_set_event_handler(bc_module_gps_event_handler_t event_handler, void *event_param)
{
    _bc_module_gps.event_handler = event_handler;
    _bc_module_gps.event_param = event_param;
}

void bc_module_gps_start(void)
{
    bc_sam_m8q_start(&_bc_module_gps.sam_m8q);
}

void bc_module_gps_stop(void)
{
    bc_sam_m8q_stop(&_bc_module_gps.sam_m8q);
}

void bc_module_gps_invalidate(void)
{
    bc_sam_m8q_invalidate(&_bc_module_gps.sam_m8q);
}

bool bc_module_gps_get_time(bc_module_gps_time_t *time)
{
    return bc_sam_m8q_get_time(&_bc_module_gps.sam_m8q, time);
}

bool bc_module_gps_get_position(bc_module_gps_position_t *position)
{
    return bc_sam_m8q_get_position(&_bc_module_gps.sam_m8q, position);
}

bool bc_module_gps_get_altitude(bc_module_gps_altitude_t *altitude)
{
    return bc_sam_m8q_get_altitude(&_bc_module_gps.sam_m8q, altitude);
}

bool bc_module_gps_get_quality(bc_module_gps_quality_t *quality)
{
    return bc_sam_m8q_get_quality(&_bc_module_gps.sam_m8q, quality);
}

bool bc_module_gps_get_accuracy(bc_module_gps_accuracy_t *accuracy)
{
    return bc_sam_m8q_get_accuracy(&_bc_module_gps.sam_m8q, accuracy);
}

const bc_led_driver_t *bc_module_gps_get_led_driver(void)
{
    static const bc_led_driver_t bc_module_gps_led_driver =
    {
        .init = _bc_module_gps_led_init,
        .on = _bc_module_gps_led_on,
        .off = _bc_module_gps_led_off,
    };

    return &bc_module_gps_led_driver;
}

static void _bc_module_gps_led_init(bc_led_t *self)
{
    (void) self;
}

static void _bc_module_gps_led_on(bc_led_t *self)
{
    if (!bc_tca9534a_write_pin(&_bc_module_gps.tca9534, _bc_module_gps_led_pin_lut[self->_channel.virtual], self->_idle_state ? 0 : 1))
    {
        // TODO
    }
}

static void _bc_module_gps_led_off(bc_led_t *self)
{
    if (!bc_tca9534a_write_pin(&_bc_module_gps.tca9534, _bc_module_gps_led_pin_lut[self->_channel.virtual], self->_idle_state ? 1 : 0))
    {
        // TODO
    }
}

static void _bc_module_gps_sam_m8q_event_handler(bc_sam_m8q_t *self, bc_sam_m8q_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (_bc_module_gps.event_handler == NULL)
    {
        return;
    }

    if (event == BC_SAM_M8Q_EVENT_ERROR)
    {
        _bc_module_gps.event_handler(BC_MODULE_GPS_EVENT_ERROR, _bc_module_gps.event_param);
    }
    else if (event == BC_SAM_M8Q_EVENT_START)
    {
        _bc_module_gps.event_handler(BC_MODULE_GPS_EVENT_START, _bc_module_gps.event_param);
    }
    else if (event == BC_SAM_M8Q_EVENT_UPDATE)
    {
        _bc_module_gps.event_handler(BC_MODULE_GPS_EVENT_UPDATE, _bc_module_gps.event_param);
    }
    else if (event == BC_SAM_M8Q_EVENT_STOP)
    {
        _bc_module_gps.event_handler(BC_MODULE_GPS_EVENT_STOP, _bc_module_gps.event_param);
    }
}

static bool _bc_module_gps_sam_m8q_on(bc_sam_m8q_t *self)
{
    (void) self;

    return bc_tca9534a_write_pin(&_bc_module_gps.tca9534, BC_TCA9534A_PIN_P0, 1);
}

static bool _bc_module_gps_sam_m8q_off(bc_sam_m8q_t *self)
{
    (void) self;

    return bc_tca9534a_write_pin(&_bc_module_gps.tca9534, BC_TCA9534A_PIN_P0, 0);
}
