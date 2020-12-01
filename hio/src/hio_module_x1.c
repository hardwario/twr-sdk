#include <hio_module_x1.h>
#include <hio_onewire_ds2484.h>
#include <hio_tca9534a.h>
#include <hio_log.h>

static struct
{
    bool initialized;
    hio_tca9534a_t tca9534a;
    hio_ds2484_t ds2484;
    hio_onewire_t onewire;
    bool onewire_initialized;
    int onewire_power_semaphore;

} _hio_module_x1 = { .initialized = false };

static bool _hio_module_x1_set_slpz(void *ctx, bool state);

bool hio_module_x1_init(void)
{
    if (_hio_module_x1.initialized) {
        return true;
    }

    memset(&_hio_module_x1, 0, sizeof(_hio_module_x1));

    if (!hio_tca9534a_init(&_hio_module_x1.tca9534a, HIO_I2C_I2C0, 0x3B)) {
        hio_log_error("X1: Expander init");
        return false;
    }

    if (!hio_tca9534a_write_port(&_hio_module_x1.tca9534a, 0x48)) {
        hio_log_error("X1: Expander write_port");
        return false;
    }

    if (!hio_tca9534a_set_port_direction(&_hio_module_x1.tca9534a, 0x00)) {
        hio_log_error("X1: Expander port_direction");
        return false;
    }

    hio_onewire_ds2484_init(&_hio_module_x1.onewire, &_hio_module_x1.ds2484);
    hio_ds2484_set_slpz_handler(&_hio_module_x1.ds2484, _hio_module_x1_set_slpz, NULL);

    _hio_module_x1.initialized = true;

    return true;
}

hio_onewire_t *hio_module_x1_get_onewire(void)
{
    return &_hio_module_x1.onewire;
}

static bool _hio_module_x1_set_slpz(void *ctx, bool state)
{
    (void) ctx;

    if (state) {
        if (!hio_tca9534a_write_port(&_hio_module_x1.tca9534a, 0xc8)) {
            hio_log_error("X1: set_slzp: Expander write failed");
            return false;
        }
    } else {
        if (!hio_tca9534a_write_port(&_hio_module_x1.tca9534a, 0x48)) {
            hio_log_error("X1: set_slzp: Expander write failed");
            return false;
        }
    }

    return true;
}
