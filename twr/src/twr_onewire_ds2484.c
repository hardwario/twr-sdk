#include <twr_onewire_ds2484.h>
#include <twr_system.h>
#include <twr_timer.h>
#include <twr_i2c.h>
#include <twr_log.h>

static bool _twr_onewire_ds2484_init(void *ctx);
static bool _twr_onewire_ds2484_enable(void *ctx);
static bool _twr_onewire_ds2484_disable(void *ctx);
static bool _twr_onewire_ds2484_reset(void *ctx);
static void _twr_onewire_ds2484_write_bit(void *ctx, uint8_t bit);
static uint8_t _twr_onewire_ds2484_read_bit(void *ctx);
static void _twr_onewire_ds2484_write_byte(void *ctx, uint8_t byte);
static uint8_t _twr_onewire_ds2484_read_byte(void *ctx);
static bool _twr_onewire_ds2484_search_next(void *ctx, twr_onewire_t *onewire, uint64_t *device_number);

static const twr_onewire_driver_t _twr_onewire_ds2484_driver =
{
    .init = _twr_onewire_ds2484_init,
    .enable = _twr_onewire_ds2484_enable,
    .disable = _twr_onewire_ds2484_disable,
    .reset = _twr_onewire_ds2484_reset,
    .write_bit = _twr_onewire_ds2484_write_bit,
    .read_bit = _twr_onewire_ds2484_read_bit,
    .write_byte = _twr_onewire_ds2484_write_byte,
    .read_byte = _twr_onewire_ds2484_read_byte,
    .search_next = _twr_onewire_ds2484_search_next
};

void twr_onewire_ds2484_init(twr_onewire_t *onewire, twr_ds2484_t *twr_ds2484)
{
    twr_onewire_init(onewire, &_twr_onewire_ds2484_driver, twr_ds2484);
}

const twr_onewire_driver_t *twr_onewire_det_driver(void)
{
    return &_twr_onewire_ds2484_driver;
}

static bool _twr_onewire_ds2484_init(void *ctx)
{
    return twr_ds2484_init((twr_ds2484_t *) ctx, TWR_I2C_I2C0);
}

static bool _twr_onewire_ds2484_enable(void *ctx)
{
    twr_ds2484_enable((twr_ds2484_t *) ctx);
    return true;
}

static bool _twr_onewire_ds2484_disable(void *ctx)
{
    twr_ds2484_disable((twr_ds2484_t *) ctx);
    return true;
}

static bool _twr_onewire_ds2484_reset(void *ctx)
{
    return twr_ds2484_reset((twr_ds2484_t *) ctx);
}

static void _twr_onewire_ds2484_write_bit(void *ctx, uint8_t bit)
{
    (void) ctx;
    (void) bit;
    twr_log_warning("onewire_ds2484_write_bit not implemented");
}

static uint8_t _twr_onewire_ds2484_read_bit(void *ctx)
{
    uint8_t bit = 0;
    twr_ds2484_read_bit((twr_ds2484_t *) ctx, &bit);
    return bit;
}

static void _twr_onewire_ds2484_write_byte(void *ctx, uint8_t byte)
{
    twr_ds2484_write_byte((twr_ds2484_t *) ctx, byte);
}

static uint8_t _twr_onewire_ds2484_read_byte(void *ctx)
{
    uint8_t byte = 0;
    twr_ds2484_read_byte((twr_ds2484_t *) ctx, &byte);
    return byte;
}

static bool _twr_onewire_ds2484_search_next(void *ctx, twr_onewire_t *onewire, uint64_t *device_number)
{
    twr_ds2484_t *ds2484 = (twr_ds2484_t *) ctx;

    *device_number = 0;

    if (!twr_ds2484_reset(ds2484))
    {
        twr_log_warning("ds2484_reset");
        return false;
    }

    if (!twr_ds2484_write_byte(ds2484, 0xf0)) // search command
    {
        twr_log_warning("ds2484_write_byte(0xf0)");
        return false;
    }

    uint8_t id_bit_number = 1;
    uint8_t rom_byte_number = 0;
    uint8_t rom_byte_mask = 1;
    uint8_t last_zero = 0;

    do
    {
        uint8_t status;
        uint8_t id;
        uint8_t comp_id;
        uint8_t direction;

        if (id_bit_number < onewire->_last_discrepancy)
        {
            direction = onewire->_last_rom_no[rom_byte_number] & rom_byte_mask;
        }
        else
        {
            direction = id_bit_number == onewire->_last_discrepancy;
        }

        if (!twr_ds2484_triplet(ds2484, direction))
        {
            twr_log_warning("ds2484_triplet");
            return false;
        }

        if (!twr_ds2484_busy_wait(ds2484))
        {
            twr_log_warning("ds2484_busy_wait");
            return false;
        }

        status = twr_ds2484_status_get(ds2484);

        id        = status & TWR_DS2484_STATUS_SBR;
        comp_id   = status & TWR_DS2484_STATUS_TSB;
        direction = status & TWR_DS2484_STATUS_DIR;

        if (id && comp_id)
        {
            twr_log_warning("id && comp_id");
            return false;
        }

        if (!id && !comp_id && !direction)
        {
            last_zero = id_bit_number;
        }

        if (direction)
        {
            onewire->_last_rom_no[rom_byte_number] |= rom_byte_mask;
        }
        else
        {
            onewire->_last_rom_no[rom_byte_number] &= ~rom_byte_mask;
        }

        id_bit_number++;
        rom_byte_mask <<= 1;

        if (rom_byte_mask == 0)
        {
            rom_byte_number++;
            rom_byte_mask = 1;
        }

    } while (rom_byte_number < 8);

    onewire->_last_device_flag = last_zero == 0;

    onewire->_last_discrepancy = last_zero;

    if (twr_onewire_crc8(onewire->_last_rom_no, sizeof(onewire->_last_rom_no), 0) != 0)
    {
        return false;
    }

    memcpy(device_number, onewire->_last_rom_no, sizeof(onewire->_last_rom_no));

    return true;
}
