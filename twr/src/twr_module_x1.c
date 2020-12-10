#include <twr_module_x1.h>
#include <twr_onewire_ds2484.h>
#include <twr_tca9534a.h>
#include <twr_log.h>

static struct
{
    bool initialized;
    twr_tca9534a_t tca9534a;
    twr_ds2484_t ds2484;
    twr_onewire_t onewire;
    bool onewire_initialized;
    int onewire_power_semaphore;

} _twr_module_x1 = { .initialized = false };

static bool _twr_module_x1_set_slpz(void *ctx, bool state);

bool twr_module_x1_init(void)
{
    if (_twr_module_x1.initialized) {
        return true;
    }

    memset(&_twr_module_x1, 0, sizeof(_twr_module_x1));

    if (!twr_tca9534a_init(&_twr_module_x1.tca9534a, TWR_I2C_I2C0, 0x3B)) {
        twr_log_error("X1: Expander init");
        return false;
    }

    if (!twr_tca9534a_write_port(&_twr_module_x1.tca9534a, 0x48)) {
        twr_log_error("X1: Expander write_port");
        return false;
    }

    if (!twr_tca9534a_set_port_direction(&_twr_module_x1.tca9534a, 0x00)) {
        twr_log_error("X1: Expander port_direction");
        return false;
    }

    twr_onewire_ds2484_init(&_twr_module_x1.onewire, &_twr_module_x1.ds2484);
    twr_ds2484_set_slpz_handler(&_twr_module_x1.ds2484, _twr_module_x1_set_slpz, NULL);

    _twr_module_x1.initialized = true;

    return true;
}

twr_onewire_t *twr_module_x1_get_onewire(void)
{
    return &_twr_module_x1.onewire;
}

static bool _twr_module_x1_set_slpz(void *ctx, bool state)
{
    (void) ctx;

    if (state) {
        if (!twr_tca9534a_write_port(&_twr_module_x1.tca9534a, 0xc8)) {
            twr_log_error("X1: set_slzp: Expander write failed");
            return false;
        }
    } else {
        if (!twr_tca9534a_write_port(&_twr_module_x1.tca9534a, 0x48)) {
            twr_log_error("X1: set_slzp: Expander write failed");
            return false;
        }
    }

    return true;
}
