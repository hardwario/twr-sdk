
#include <twr_config.h>
#include <twr_sha256.h>
#include <twr_eeprom.h>

#define _CONFIG_SIZEOF_HEADER sizeof(config_header_t)
#define _CONFIG_SIZEOF_CONFIG sizeof(config_t)

#define _CONFIG_ADDRESS_HEADER 0
#define _CONFIG_ADDRESS_CONFIG (_CONFIG_ADDRESS_HEADER + _CONFIG_SIZEOF_HEADER)

typedef struct
{
    uint32_t signature;
    void *config;
    size_t size;
    void *init_config;

} twr_config_t;

twr_config_t _twr_config;

#pragma pack(push, 1)

typedef struct
{
    uint64_t signature;
    uint8_t version;
    uint16_t length;
    uint8_t hash[6];

} config_header_t;

#pragma pack(pop)

static void _config_eeprom_read(uint32_t address, void *buffer, size_t length);
static void _config_eeprom_write(uint32_t address, const void *buffer, size_t length);

void twr_config_init(uint64_t signature, void *config, size_t size, void *init_config)
{
    _twr_config.signature = signature;
    _twr_config.config = config;
    _twr_config.size = size;
    _twr_config.init_config = init_config;

    if (!twr_config_load())
    {
        twr_config_reset();
        twr_config_save();
    }
}

void twr_config_reset(void)
{
    if (!_twr_config.init_config)
    {
        // Initialize to zero if init_config is not set
        memset(_twr_config.config, 0, _twr_config.size);
    }
    else
    {
        memcpy(_twr_config.config, _twr_config.init_config, _twr_config.size);
    }
}

bool twr_config_load(void)
{
    config_header_t header;

    _config_eeprom_read(_CONFIG_ADDRESS_HEADER, &header, _CONFIG_SIZEOF_HEADER);

    if (header.signature != _twr_config.signature ||
        header.length != _twr_config.size)
    {
        return false;
    }

    _config_eeprom_read(_CONFIG_ADDRESS_CONFIG, _twr_config.config, _twr_config.size);

    static twr_sha256_t sha256;
    static uint8_t hash[32];

    twr_sha256_init(&sha256);
    twr_sha256_update(&sha256, _twr_config.config, _twr_config.size);
    twr_sha256_final(&sha256, hash, false);

    if (memcmp(header.hash, hash, sizeof(header.hash)) != 0)
    {
        return false;
    }

    return true;
}

bool twr_config_save(void)
{
    static twr_sha256_t sha256;
    static uint8_t hash[32];

    twr_sha256_init(&sha256);
    twr_sha256_update(&sha256, _twr_config.config, _twr_config.size);
    twr_sha256_final(&sha256, hash, false);

    config_header_t header;

    header.signature = _twr_config.signature;
    header.length = _twr_config.size;

    memcpy(header.hash, hash, sizeof(header.hash));

    _config_eeprom_write(_CONFIG_ADDRESS_HEADER, &header, _CONFIG_SIZEOF_HEADER);
    _config_eeprom_write(_CONFIG_ADDRESS_CONFIG, _twr_config.config, _twr_config.size);

    return true;
}

static void _config_eeprom_read(uint32_t address, void *buffer, size_t length)
{
    uint8_t *p = buffer;

    for (size_t i = 0; i < length; i++)
    {
        uint8_t a, b, c;

        uint32_t offset_bank_a = 0;
        uint32_t offset_bank_b = offset_bank_a + _CONFIG_SIZEOF_HEADER + _twr_config.size;
        uint32_t offset_bank_c = offset_bank_b + _CONFIG_SIZEOF_HEADER + _twr_config.size;

        if (!twr_eeprom_read(offset_bank_a + address + i, &a, 1) ||
            !twr_eeprom_read(offset_bank_b + address + i, &b, 1) ||
            !twr_eeprom_read(offset_bank_c + address + i, &c, 1))
        {
            //app_error(APP_ERROR);
        }

        *p++ = (a & b) | (a & c) | (b & c);
    }
}

static void _config_eeprom_write(uint32_t address, const void *buffer, size_t length)
{
    uint32_t offset_bank_a = 0;
    uint32_t offset_bank_b = offset_bank_a + _CONFIG_SIZEOF_HEADER + _twr_config.size;
    uint32_t offset_bank_c = offset_bank_b + _CONFIG_SIZEOF_HEADER + _twr_config.size;

    if (!twr_eeprom_write(offset_bank_a + address, buffer, length) ||
        !twr_eeprom_write(offset_bank_b + address, buffer, length) ||
        !twr_eeprom_write(offset_bank_c + address, buffer, length))
    {
        //app_error(APP_ERROR_EEPROM);
    }
}
