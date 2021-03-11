#ifndef TWR_TWR_INFO_H
#define TWR_TWR_INFO_H

#include <twr_common.h>

typedef enum
{
    TWR_INFO_PIB_VALID = 0,
    TWR_INFO_PIB_ERR_SIGNATURE = -1,
    TWR_INFO_PIB_ERR_VERSION = -2,
    TWR_INFO_PIB_ERR_SIZE = -3,
    TWR_INFO_PIB_ERR_CRC = -4,

} twr_info_pib_t;

void twr_info_init(void);

twr_info_pib_t
twr_info_pib_check(void);

uint32_t
twr_info_serial_number(void);

uint16_t
twr_info_hw_revision(void);

uint32_t
twr_info_hw_variant(void);

uint32_t
twr_info_fw_version(void);

const char *
twr_info_vendor_name(void);

const char *
twr_info_product_name(void);

#endif // TWR_TWR_INFO_H
