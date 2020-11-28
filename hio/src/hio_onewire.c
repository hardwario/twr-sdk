#include <hio_onewire.h>
#include <hio_error.h>

static void _hio_onewire_lock(hio_onewire_t *self);
static void _hio_onewire_unlock(hio_onewire_t *self);
static void _hio_onewire_search_reset(hio_onewire_t *self);
static void _hio_onewire_search_target_setup(hio_onewire_t *self, uint8_t family_code);
static int _hio_onewire_search_devices(hio_onewire_t *self, uint64_t *device_list, size_t device_list_size);

#define _HIO_ONEWIRE_CHECK_CALLBACK(X) if (X == NULL) hio_error(HIO_ERROR_CALLBACK)

bool hio_onewire_init(hio_onewire_t *self, const hio_onewire_driver_t *driver, void *driver_ctx)
{
    memset(self, 0, sizeof(*self));

    _HIO_ONEWIRE_CHECK_CALLBACK(driver->init);
    _HIO_ONEWIRE_CHECK_CALLBACK(driver->enable);
    _HIO_ONEWIRE_CHECK_CALLBACK(driver->disable);
    _HIO_ONEWIRE_CHECK_CALLBACK(driver->reset);
    _HIO_ONEWIRE_CHECK_CALLBACK(driver->write_bit);
    _HIO_ONEWIRE_CHECK_CALLBACK(driver->read_bit);
    _HIO_ONEWIRE_CHECK_CALLBACK(driver->write_byte);
    _HIO_ONEWIRE_CHECK_CALLBACK(driver->read_byte);

    self->_driver = driver;
    self->_driver_ctx = driver_ctx;

    return self->_driver->init(self->_driver_ctx);
}

bool hio_onewire_transaction_start(hio_onewire_t *self)
{
	if (self->_lock_count != 0)
	{
        return false;
	}

    _hio_onewire_lock(self);

	return true;
}

bool hio_onewire_transaction_stop(hio_onewire_t *self)
{
	_hio_onewire_unlock(self);

	return true;
}

bool hio_onewire_is_transaction(hio_onewire_t *self)
{
    return self->_lock_count != 0;
}

bool hio_onewire_reset(hio_onewire_t *self)
{
    bool state;

    _hio_onewire_lock(self);

    state = self->_driver->reset(self->_driver_ctx);

    _hio_onewire_unlock(self);

    return state;
}

void hio_onewire_select(hio_onewire_t *self, uint64_t *device_number)
{
    _hio_onewire_lock(self);

    if (*device_number == HIO_ONEWIRE_DEVICE_NUMBER_SKIP_ROM)
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

    _hio_onewire_unlock(self);
}

void hio_onewire_skip_rom(hio_onewire_t *self)
{
    _hio_onewire_lock(self);
    self->_driver->write_byte(self->_driver_ctx, 0xCC);
    _hio_onewire_unlock(self);
}

void hio_onewire_write(hio_onewire_t *self, const void *buffer, size_t length)
{
    _hio_onewire_lock(self);
    for (size_t i = 0; i < length; i++)
    {
        self->_driver->write_byte(self->_driver_ctx, ((uint8_t *) buffer)[i]);
    }
    _hio_onewire_unlock(self);
}

void hio_onewire_read(hio_onewire_t *self, void *buffer, size_t length)
{
    _hio_onewire_lock(self);
    for (size_t i = 0; i < length; i++)
    {
        ((uint8_t *) buffer)[i] = self->_driver->read_byte(self->_driver_ctx);
    }
    _hio_onewire_unlock(self);
}

void hio_onewire_write_byte(hio_onewire_t *self, uint8_t byte)
{
    _hio_onewire_lock(self);
    self->_driver->write_byte(self->_driver_ctx, byte);
    _hio_onewire_unlock(self);
}

uint8_t hio_onewire_read_byte(hio_onewire_t *self)
{
    _hio_onewire_lock(self);
    uint8_t data = self->_driver->read_byte(self->_driver_ctx);
    _hio_onewire_unlock(self);
    return data;
}

void hio_onewire_write_bit(hio_onewire_t *self, int bit)
{
    _hio_onewire_lock(self);
    self->_driver->write_bit(self->_driver_ctx, bit & 0x01);
    _hio_onewire_unlock(self);
}

int hio_onewire_read_bit(hio_onewire_t *self)
{
    _hio_onewire_lock(self);
    int bit = self->_driver->read_bit(self->_driver_ctx);
    _hio_onewire_unlock(self);
    return bit;
}

int hio_onewire_search_all(hio_onewire_t *self, uint64_t *device_list, size_t device_list_size)
{
    _hio_onewire_search_reset(self);

    return _hio_onewire_search_devices(self, device_list, device_list_size);
}

int hio_onewire_search_family(hio_onewire_t *self, uint8_t family_code, uint64_t *device_list, size_t device_list_size)
{
    _hio_onewire_search_target_setup(self, family_code);

    return _hio_onewire_search_devices(self, device_list, device_list_size);
}

void hio_onewire_auto_ds28e17_sleep_mode(hio_onewire_t *self, bool on)
{
    self->_auto_ds28e17_sleep_mode = on;
}

uint8_t hio_onewire_crc8(const void *buffer, size_t length, uint8_t crc)
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

uint16_t hio_onewire_crc16(const void *buffer, size_t length, uint16_t crc)
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

void hio_onewire_search_start(hio_onewire_t *self, uint8_t family_code)
{
    if (family_code != 0)
    {
        _hio_onewire_search_target_setup(self, family_code);
    }
    else
    {
        _hio_onewire_search_reset(self);
    }
}

bool hio_onewire_search_next(hio_onewire_t *self, uint64_t *device_number)
{
    if (self->_last_device_flag)
    {
        return false;
    }

    _hio_onewire_lock(self);
    bool search_result = self->_driver->search_next(self->_driver_ctx, self, device_number);
    _hio_onewire_unlock(self);
    return search_result;
}

static void _hio_onewire_lock(hio_onewire_t *self)
{
    if ((self->_lock_count)++ == 0)
    {
        self->_driver->enable(self->_driver_ctx);
    }
}

static void _hio_onewire_unlock(hio_onewire_t *self)
{
    if (self->_lock_count < 1) hio_error(HIO_ERROR_ERROR_UNLOCK);

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

static void _hio_onewire_search_reset(hio_onewire_t *self)
{
    self->_last_discrepancy = 0;
    self->_last_device_flag = false;
    self->_last_family_discrepancy = 0;
}

static void _hio_onewire_search_target_setup(hio_onewire_t *self, uint8_t family_code)
{
    memset(self->_last_rom_no, 0, sizeof(self->_last_rom_no));
    self->_last_rom_no[0] = family_code;
    self->_last_discrepancy = 64;
    self->_last_family_discrepancy = 0;
    self->_last_device_flag = false;
}

static int _hio_onewire_search_devices(hio_onewire_t *self, uint64_t *device_list, size_t device_list_size)
{
    int devices = 0;
    int max_devices = device_list_size / sizeof(uint64_t);

    while ((devices < max_devices) && hio_onewire_search_next(self, device_list))
    {
        device_list++;
        devices++;
    }

    return devices;
}
