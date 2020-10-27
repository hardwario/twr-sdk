#include <bc_onewire.h>
#include <bc_error.h>

static void _bc_onewire_lock(bc_onewire_t *self);
static void _bc_onewire_unlock(bc_onewire_t *self);
static void _bc_onewire_search_reset(bc_onewire_t *self);
static void _bc_onewire_search_target_setup(bc_onewire_t *self, uint8_t family_code);
static int _bc_onewire_search_devices(bc_onewire_t *self, uint64_t *device_list, size_t device_list_size);

#define _BC_ONEWIRE_CHECK_CALLBACK(X) if (X == NULL) bc_error(BC_ERROR_CALLBACK)

bool bc_onewire_init(bc_onewire_t *self, const bc_onewire_driver_t *driver, void *driver_ctx)
{
    memset(self, 0, sizeof(*self));

    _BC_ONEWIRE_CHECK_CALLBACK(driver->init);
    _BC_ONEWIRE_CHECK_CALLBACK(driver->enable);
    _BC_ONEWIRE_CHECK_CALLBACK(driver->disable);
    _BC_ONEWIRE_CHECK_CALLBACK(driver->reset);
    _BC_ONEWIRE_CHECK_CALLBACK(driver->write_bit);
    _BC_ONEWIRE_CHECK_CALLBACK(driver->read_bit);
    _BC_ONEWIRE_CHECK_CALLBACK(driver->write_byte);
    _BC_ONEWIRE_CHECK_CALLBACK(driver->read_byte);

    self->_driver = driver;
    self->_driver_ctx = driver_ctx;

    return self->_driver->init(self->_driver_ctx);
}

bool bc_onewire_transaction_start(bc_onewire_t *self)
{
	if (self->_lock_count != 0)
	{
        return false;
	}

    _bc_onewire_lock(self);

	return true;
}

bool bc_onewire_transaction_stop(bc_onewire_t *self)
{
	_bc_onewire_unlock(self);

	return true;
}

bool bc_onewire_is_transaction(bc_onewire_t *self)
{
    return self->_lock_count != 0;
}

bool bc_onewire_reset(bc_onewire_t *self)
{
    bool state;

    _bc_onewire_lock(self);

    state = self->_driver->reset(self->_driver_ctx);

    _bc_onewire_unlock(self);

    return state;
}

void bc_onewire_select(bc_onewire_t *self, uint64_t *device_number)
{
    _bc_onewire_lock(self);

    if (*device_number == BC_ONEWIRE_DEVICE_NUMBER_SKIP_ROM)
    {
        self->_driver->write_byte(self->_driver_ctx, 0xCC);
    }
    else
    {
        self->_driver->write_byte(self->_driver_ctx, 0x55);

        for (size_t i = 0; i < sizeof(uint64_t); i++)
        {
            self->_driver->write_byte(self->_driver_ctx, ((uint8_t *) device_number)[i]);
        }
    }

    _bc_onewire_unlock(self);
}

void bc_onewire_skip_rom(bc_onewire_t *self)
{
    _bc_onewire_lock(self);
    self->_driver->write_byte(self->_driver_ctx, 0xCC);
    _bc_onewire_unlock(self);
}

void bc_onewire_write(bc_onewire_t *self, const void *buffer, size_t length)
{
    _bc_onewire_lock(self);
    for (size_t i = 0; i < length; i++)
    {
        self->_driver->write_byte(self->_driver_ctx, ((uint8_t *) buffer)[i]);
    }
    _bc_onewire_unlock(self);
}

void bc_onewire_read(bc_onewire_t *self, void *buffer, size_t length)
{
    _bc_onewire_lock(self);
    for (size_t i = 0; i < length; i++)
    {
        ((uint8_t *) buffer)[i] = self->_driver->read_byte(self->_driver_ctx);
    }
    _bc_onewire_unlock(self);
}

void bc_onewire_write_byte(bc_onewire_t *self, uint8_t byte)
{
    _bc_onewire_lock(self);
    self->_driver->write_byte(self->_driver_ctx, byte);
    _bc_onewire_unlock(self);
}

uint8_t bc_onewire_read_byte(bc_onewire_t *self)
{
    _bc_onewire_lock(self);
    uint8_t data = self->_driver->read_byte(self->_driver_ctx);
    _bc_onewire_unlock(self);
    return data;
}

void bc_onewire_write_bit(bc_onewire_t *self, int bit)
{
    _bc_onewire_lock(self);
    self->_driver->write_bit(self->_driver_ctx, bit & 0x01);
    _bc_onewire_unlock(self);
}

int bc_onewire_read_bit(bc_onewire_t *self)
{
    _bc_onewire_lock(self);
    int bit = self->_driver->read_bit(self->_driver_ctx);
    _bc_onewire_unlock(self);
    return bit;
}

int bc_onewire_search_all(bc_onewire_t *self, uint64_t *device_list, size_t device_list_size)
{
    _bc_onewire_search_reset(self);

    return _bc_onewire_search_devices(self, device_list, device_list_size);
}

int bc_onewire_search_family(bc_onewire_t *self, uint8_t family_code, uint64_t *device_list, size_t device_list_size)
{
    _bc_onewire_search_target_setup(self, family_code);

    return _bc_onewire_search_devices(self, device_list, device_list_size);
}

void bc_onewire_auto_ds28e17_sleep_mode(bc_onewire_t *self, bool on)
{
    self->_auto_ds28e17_sleep_mode = on;
}

uint8_t bc_onewire_crc8(const void *buffer, size_t length, uint8_t crc)
{
    uint8_t *_buffer = (uint8_t *) buffer;
    uint8_t inbyte;
    uint8_t i;

    while (length--)
    {
        inbyte = *_buffer++;
        for (i = 8; i; i--)
        {
            if ((crc ^ inbyte) & 0x01)
            {
                crc >>= 1;
                crc ^= 0x8C;
            }
            else
            {
                crc >>= 1;
            }
            inbyte >>= 1;
        }
    }

    return crc;
}

uint16_t bc_onewire_crc16(const void *buffer, size_t length, uint16_t crc)
{
    static const uint8_t oddparity[16] =
    { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

    uint16_t i;
    for (i = 0; i < length; i++)
    {
        uint16_t cdata = ((uint8_t *) buffer)[i];
        cdata = (cdata ^ crc) & 0xff;
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4]) crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }
    return crc;
}

void bc_onewire_search_start(bc_onewire_t *self, uint8_t family_code)
{
    if (family_code != 0)
    {
        _bc_onewire_search_target_setup(self, family_code);
    }
    else
    {
        _bc_onewire_search_reset(self);
    }
}

bool bc_onewire_search_next(bc_onewire_t *self, uint64_t *device_number)
{
    if (self->_last_device_flag)
    {
        return false;
    }

    _bc_onewire_lock(self);
    bool search_result = self->_driver->search_next(self->_driver_ctx, self, device_number);
    _bc_onewire_unlock(self);
    return search_result;
}

static void _bc_onewire_lock(bc_onewire_t *self)
{
    if ((self->_lock_count)++ == 0)
    {
        self->_driver->enable(self->_driver_ctx);
    }
}

static void _bc_onewire_unlock(bc_onewire_t *self)
{
    if (self->_lock_count < 1) bc_error(BC_ERROR_ERROR_UNLOCK);

    if (self->_lock_count == 1)
    {
        if (self->_auto_ds28e17_sleep_mode)
        {
            if (self->_driver->reset(self->_driver_ctx))
            {
                self->_driver->write_byte(self->_driver_ctx, 0xcc);
                self->_driver->write_byte(self->_driver_ctx, 0x1e);
            }
        }

        self->_driver->disable(self->_driver_ctx);
    }

    self->_lock_count--;
}

static void _bc_onewire_search_reset(bc_onewire_t *self)
{
    self->_last_discrepancy = 0;
    self->_last_device_flag = false;
    self->_last_family_discrepancy = 0;
}

static void _bc_onewire_search_target_setup(bc_onewire_t *self, uint8_t family_code)
{
    memset(self->_last_rom_no, 0, sizeof(self->_last_rom_no));
    self->_last_rom_no[0] = family_code;
    self->_last_discrepancy = 64;
    self->_last_family_discrepancy = 0;
    self->_last_device_flag = false;
}

static int _bc_onewire_search_devices(bc_onewire_t *self, uint64_t *device_list, size_t device_list_size)
{
    int devices = 0;
    int max_devices = device_list_size / sizeof(uint64_t);

    while ((devices < max_devices) && bc_onewire_search_next(self, device_list))
    {
        device_list++;
        devices++;
    }

    return devices;
}
