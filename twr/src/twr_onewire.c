#include <twr_onewire.h>
#include <twr_error.h>

static void _twr_onewire_lock(twr_onewire_t *self);
static void _twr_onewire_unlock(twr_onewire_t *self);
static void _twr_onewire_search_reset(twr_onewire_t *self);
static void _twr_onewire_search_target_setup(twr_onewire_t *self, uint8_t family_code);
static int _twr_onewire_search_devices(twr_onewire_t *self, uint64_t *device_list, size_t device_list_size);

#define _TWR_ONEWIRE_CHECK_CALLBACK(X) if (X == NULL) twr_error(TWR_ERROR_CALLBACK)

bool twr_onewire_init(twr_onewire_t *self, const twr_onewire_driver_t *driver, void *driver_ctx)
{
    memset(self, 0, sizeof(*self));

    _TWR_ONEWIRE_CHECK_CALLBACK(driver->init);
    _TWR_ONEWIRE_CHECK_CALLBACK(driver->enable);
    _TWR_ONEWIRE_CHECK_CALLBACK(driver->disable);
    _TWR_ONEWIRE_CHECK_CALLBACK(driver->reset);
    _TWR_ONEWIRE_CHECK_CALLBACK(driver->write_bit);
    _TWR_ONEWIRE_CHECK_CALLBACK(driver->read_bit);
    _TWR_ONEWIRE_CHECK_CALLBACK(driver->write_byte);
    _TWR_ONEWIRE_CHECK_CALLBACK(driver->read_byte);

    self->_driver = driver;
    self->_driver_ctx = driver_ctx;

    return self->_driver->init(self->_driver_ctx);
}

bool twr_onewire_transaction_start(twr_onewire_t *self)
{
	if (self->_lock_count != 0)
	{
        return false;
	}

    _twr_onewire_lock(self);

	return true;
}

bool twr_onewire_transaction_stop(twr_onewire_t *self)
{
	_twr_onewire_unlock(self);

	return true;
}

bool twr_onewire_is_transaction(twr_onewire_t *self)
{
    return self->_lock_count != 0;
}

bool twr_onewire_reset(twr_onewire_t *self)
{
    bool state;

    _twr_onewire_lock(self);

    state = self->_driver->reset(self->_driver_ctx);

    _twr_onewire_unlock(self);

    return state;
}

void twr_onewire_select(twr_onewire_t *self, uint64_t *device_number)
{
    _twr_onewire_lock(self);

    if (*device_number == TWR_ONEWIRE_DEVICE_NUMBER_SKIP_ROM)
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

    _twr_onewire_unlock(self);
}

void twr_onewire_skip_rom(twr_onewire_t *self)
{
    _twr_onewire_lock(self);
    self->_driver->write_byte(self->_driver_ctx, 0xCC);
    _twr_onewire_unlock(self);
}

void twr_onewire_write(twr_onewire_t *self, const void *buffer, size_t length)
{
    _twr_onewire_lock(self);
    for (size_t i = 0; i < length; i++)
    {
        self->_driver->write_byte(self->_driver_ctx, ((uint8_t *) buffer)[i]);
    }
    _twr_onewire_unlock(self);
}

void twr_onewire_read(twr_onewire_t *self, void *buffer, size_t length)
{
    _twr_onewire_lock(self);
    for (size_t i = 0; i < length; i++)
    {
        ((uint8_t *) buffer)[i] = self->_driver->read_byte(self->_driver_ctx);
    }
    _twr_onewire_unlock(self);
}

void twr_onewire_write_byte(twr_onewire_t *self, uint8_t byte)
{
    _twr_onewire_lock(self);
    self->_driver->write_byte(self->_driver_ctx, byte);
    _twr_onewire_unlock(self);
}

uint8_t twr_onewire_read_byte(twr_onewire_t *self)
{
    _twr_onewire_lock(self);
    uint8_t data = self->_driver->read_byte(self->_driver_ctx);
    _twr_onewire_unlock(self);
    return data;
}

void twr_onewire_write_bit(twr_onewire_t *self, int bit)
{
    _twr_onewire_lock(self);
    self->_driver->write_bit(self->_driver_ctx, bit & 0x01);
    _twr_onewire_unlock(self);
}

int twr_onewire_read_bit(twr_onewire_t *self)
{
    _twr_onewire_lock(self);
    int bit = self->_driver->read_bit(self->_driver_ctx);
    _twr_onewire_unlock(self);
    return bit;
}

int twr_onewire_search_all(twr_onewire_t *self, uint64_t *device_list, size_t device_list_size)
{
    _twr_onewire_search_reset(self);

    return _twr_onewire_search_devices(self, device_list, device_list_size);
}

int twr_onewire_search_family(twr_onewire_t *self, uint8_t family_code, uint64_t *device_list, size_t device_list_size)
{
    _twr_onewire_search_target_setup(self, family_code);

    return _twr_onewire_search_devices(self, device_list, device_list_size);
}

void twr_onewire_auto_ds28e17_sleep_mode(twr_onewire_t *self, bool on)
{
    self->_auto_ds28e17_sleep_mode = on;
}

uint8_t twr_onewire_crc8(const void *buffer, size_t length, uint8_t crc)
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

uint16_t twr_onewire_crc16(const void *buffer, size_t length, uint16_t crc)
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

void twr_onewire_search_start(twr_onewire_t *self, uint8_t family_code)
{
    if (family_code != 0)
    {
        _twr_onewire_search_target_setup(self, family_code);
    }
    else
    {
        _twr_onewire_search_reset(self);
    }
}

bool twr_onewire_search_next(twr_onewire_t *self, uint64_t *device_number)
{
    if (self->_last_device_flag)
    {
        return false;
    }

    _twr_onewire_lock(self);
    bool search_result = self->_driver->search_next(self->_driver_ctx, self, device_number);
    _twr_onewire_unlock(self);
    return search_result;
}

static void _twr_onewire_lock(twr_onewire_t *self)
{
    if ((self->_lock_count)++ == 0)
    {
        self->_driver->enable(self->_driver_ctx);
    }
}

static void _twr_onewire_unlock(twr_onewire_t *self)
{
    if (self->_lock_count < 1) twr_error(TWR_ERROR_ERROR_UNLOCK);

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

static void _twr_onewire_search_reset(twr_onewire_t *self)
{
    self->_last_discrepancy = 0;
    self->_last_device_flag = false;
    self->_last_family_discrepancy = 0;
}

static void _twr_onewire_search_target_setup(twr_onewire_t *self, uint8_t family_code)
{
    memset(self->_last_rom_no, 0, sizeof(self->_last_rom_no));
    self->_last_rom_no[0] = family_code;
    self->_last_discrepancy = 64;
    self->_last_family_discrepancy = 0;
    self->_last_device_flag = false;
}

static int _twr_onewire_search_devices(twr_onewire_t *self, uint64_t *device_list, size_t device_list_size)
{
    int devices = 0;
    int max_devices = device_list_size / sizeof(uint64_t);

    while ((devices < max_devices) && twr_onewire_search_next(self, device_list))
    {
        device_list++;
        devices++;
    }

    return devices;
}
