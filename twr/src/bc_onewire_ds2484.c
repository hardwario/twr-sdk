#include <bc_onewire_ds2484.h>
#include <bc_system.h>
#include <bc_timer.h>
#include <bc_i2c.h>
#include <bc_log.h>

static bool _bc_onewire_ds2484_init(void *ctx);
static bool _bc_onewire_ds2484_enable(void *ctx);
static bool _bc_onewire_ds2484_disable(void *ctx);
static bool _bc_onewire_ds2484_reset(void *ctx);
static void _bc_onewire_ds2484_write_bit(void *ctx, uint8_t bit);
static uint8_t _bc_onewire_ds2484_read_bit(void *ctx);
static void _bc_onewire_ds2484_write_byte(void *ctx, uint8_t byte);
static uint8_t _bc_onewire_ds2484_read_byte(void *ctx);
static bool _bc_onewire_ds2484_search_next(void *ctx, bc_onewire_t *onewire, uint64_t *device_number);

static const bc_onewire_driver_t _bc_onewire_ds2484_driver =
{
    .init = _bc_onewire_ds2484_init,
    .enable = _bc_onewire_ds2484_enable,
    .disable = _bc_onewire_ds2484_disable,
    .reset = _bc_onewire_ds2484_reset,
    .write_bit = _bc_onewire_ds2484_write_bit,
    .read_bit = _bc_onewire_ds2484_read_bit,
    .write_byte = _bc_onewire_ds2484_write_byte,
    .read_byte = _bc_onewire_ds2484_read_byte,
    .search_next = _bc_onewire_ds2484_search_next
};

void bc_onewire_ds2484_init(bc_onewire_t *onewire, bc_ds2484_t *bc_ds2484)
{
    bc_onewire_init(onewire, &_bc_onewire_ds2484_driver, bc_ds2484);
}

const bc_onewire_driver_t *bc_onewire_det_driver(void)
{
    return &_bc_onewire_ds2484_driver;
}

static bool _bc_onewire_ds2484_init(void *ctx)
{
    return bc_ds2484_init((bc_ds2484_t *) ctx, BC_I2C_I2C0);
}

static bool _bc_onewire_ds2484_enable(void *ctx)
{
    bc_ds2484_enable((bc_ds2484_t *) ctx);
    return true;
}

static bool _bc_onewire_ds2484_disable(void *ctx)
{
    bc_ds2484_disable((bc_ds2484_t *) ctx);
    return true;
}

static bool _bc_onewire_ds2484_reset(void *ctx)
{
    return bc_ds2484_reset((bc_ds2484_t *) ctx);
}

static void _bc_onewire_ds2484_write_bit(void *ctx, uint8_t bit)
{
    (void) ctx;
    (void) bit;
    bc_log_warning("onewire_ds2484_write_bit not implemented");
}

static uint8_t _bc_onewire_ds2484_read_bit(void *ctx)
{
    uint8_t bit = 0;
    bc_ds2484_read_bit((bc_ds2484_t *) ctx, &bit);
    return bit;
}

static void _bc_onewire_ds2484_write_byte(void *ctx, uint8_t byte)
{
    bc_ds2484_write_byte((bc_ds2484_t *) ctx, byte);
}

static uint8_t _bc_onewire_ds2484_read_byte(void *ctx)
{
    uint8_t byte = 0;
    bc_ds2484_read_byte((bc_ds2484_t *) ctx, &byte);
    return byte;
}

static bool _bc_onewire_ds2484_search_next(void *ctx, bc_onewire_t *onewire, uint64_t *device_number)
{
    bc_ds2484_t *ds2484 = (bc_ds2484_t *) ctx;

    *device_number = 0;

    if (!bc_ds2484_reset(ds2484))
    {
        bc_log_warning("ds2484_reset");
        return false;
    }

    if (!bc_ds2484_write_byte(ds2484, 0xf0)) // search command
    {
        bc_log_warning("ds2484_write_byte(0xf0)");
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

        if (!bc_ds2484_triplet(ds2484, direction))
        {
            bc_log_warning("ds2484_triplet");
            return false;
        }

        if (!bc_ds2484_busy_wait(ds2484))
        {
            bc_log_warning("ds2484_busy_wait");
            return false;
        }

        status = bc_ds2484_status_get(ds2484);

        id        = status & BC_DS2484_STATUS_SBR;
        comp_id   = status & BC_DS2484_STATUS_TSB;
        direction = status & BC_DS2484_STATUS_DIR;

        if (id && comp_id)
        {
            bc_log_warning("id && comp_id");
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

    if (bc_onewire_crc8(onewire->_last_rom_no, sizeof(onewire->_last_rom_no), 0) != 0)
    {
        return false;
    }

    memcpy(device_number, onewire->_last_rom_no, sizeof(onewire->_last_rom_no));

    return true;
}
