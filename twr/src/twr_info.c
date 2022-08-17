#include <twr_info.h>
#include <twr_log.h>

#ifndef VERSION_MAJOR
#define VERSION_MAJOR 0
#endif

#ifndef VERSION_MINOR
#define VERSION_MINOR 0
#endif

#ifndef VERSION_PATCH
#define VERSION_PATCH 0
#endif

static uint32_t signature
    __attribute__((section(".pib_signature")));

static uint8_t version
    __attribute__((section(".pib_version")));

static uint16_t size
    __attribute__((section(".pib_size")));

static uint32_t serial_number
    __attribute__((section(".pib_serial_number")));

static uint16_t hw_revision
    __attribute__((section(".pib_hw_revision")));

static uint32_t hw_variant
    __attribute__((section(".pib_hw_variant")));

static char vendor_name[32]
    __attribute__((section(".pib_vendor_name")));

static char product_name[32]
    __attribute__((section(".pib_product_name")));

static int16_t rf_offset
    __attribute__((section(".pib_rf_offset")));

static uint32_t crc
    __attribute__((section(".pib_crc")));

static bool pib_valid;

static void
calc_crc(uint32_t *crc, const uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        *crc ^= buf[i];

        for (int j = 0; j < 8; j++)
        {
            *crc = (*crc >> 1) ^ (0xedb88320 & ~((*crc & 1) - 1));
        }
    }

    *crc = ~*crc;
}

void twr_info_init(void)
{
    twr_info_pib_t res = twr_info_pib_check();

    char *type = "";

    switch (res)
    {
    case TWR_INFO_PIB_VALID:
        return;
    case TWR_INFO_PIB_ERR_SIGNATURE:
        type = "signature";
        break;
    case TWR_INFO_PIB_ERR_VERSION:
        type = "version";
        break;
    case TWR_INFO_PIB_ERR_SIZE:
        type = "size";
        break;
    case TWR_INFO_PIB_ERR_CRC:
        type = "crc";
        break;
    default:
        break;
    }

    if(type[0] == '\0')
    {
        return;
    }

    twr_log_error("APP: Integrity check for PIB failed %s", type);
}

twr_info_pib_t
twr_info_pib_check(void)
{
    pib_valid = false;

    if (signature != 0xbabecafe)
        return TWR_INFO_PIB_ERR_SIGNATURE;

    if (version != 1)
        return TWR_INFO_PIB_ERR_VERSION;

    if (size != 88 + 4 + 4)
        return TWR_INFO_PIB_ERR_SIZE;

    uint32_t crc_ = 0xffffffff;

    calc_crc(&crc_, (const uint8_t *)&signature, sizeof(signature));
    calc_crc(&crc_, (const uint8_t *)&version, sizeof(version));
    calc_crc(&crc_, (const uint8_t *)&size, sizeof(size));
    calc_crc(&crc_, (const uint8_t *)&serial_number, sizeof(serial_number));
    calc_crc(&crc_, (const uint8_t *)&hw_revision, sizeof(hw_revision));
    calc_crc(&crc_, (const uint8_t *)&hw_variant, sizeof(hw_variant));
    calc_crc(&crc_, (const uint8_t *)&vendor_name, sizeof(vendor_name));
    calc_crc(&crc_, (const uint8_t *)&product_name, sizeof(product_name));
    calc_crc(&crc_, (const uint8_t *)&rf_offset, sizeof(rf_offset));

    // twr_log_debug("crc 0x%08x", crc_);
    if (crc != crc_)
    {
        return TWR_INFO_PIB_ERR_CRC;
    }

    pib_valid = true;

    return TWR_INFO_PIB_VALID;
}

uint32_t
twr_info_serial_number(void)
{
    return pib_valid ? serial_number : 0;
}

uint16_t
twr_info_hw_revision(void)
{
    return pib_valid ? hw_revision : 0;
}

uint32_t
twr_info_hw_variant(void)
{
    return pib_valid ? hw_variant : 0;
}

uint32_t
twr_info_fw_version(void)
{
    if (pib_valid)
    {
        return VERSION_MAJOR << 24 | VERSION_MINOR << 16 | VERSION_PATCH << 8;
    }
    else
    {
        return 0;
    }
}

const char *
twr_info_vendor_name(void)
{
    if (pib_valid)
    {
        return vendor_name;
    }
    else
    {
        return "";
    }
}

const char *
twr_info_product_name(void)
{
    if (pib_valid)
    {
        return product_name;
    }
    else
    {
        return "";
    }
}
