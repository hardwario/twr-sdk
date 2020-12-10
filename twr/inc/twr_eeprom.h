#ifndef _TWR_EEPROM_H
#define _TWR_EEPROM_H

#include <twr_common.h>

//! @addtogroup twr_eeprom twr_eeprom
//! @brief Driver for internal EEPROM memory
//! @{

typedef enum
{
    //! @brief EEPROM event sync write error
    TWR_EEPROM_EVENT_ASYNC_WRITE_ERROR = 0,

    //! @brief EEPROM event sync write done
    TWR_EEPROM_EVENT_ASYNC_WRITE_DONE = 1

} twr_eepromc_event_t;

//! @brief Write buffer to EEPROM area and verify it
//! @param[in] address EEPROM start address (starts at 0)
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return true On success
//! @return false On failure

bool twr_eeprom_write(uint32_t address, const void *buffer, size_t length);

//! @brief Async write buffer to EEPROM area and verify it
//! @param[in] address EEPROM start address (starts at 0)
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)
//! @return true On success start
//! @return false On failure start

bool twr_eeprom_async_write(uint32_t address, const void *buffer, size_t length, void (*event_handler)(twr_eepromc_event_t, void *), void *event_param);

//! @brief Cancel async write

void twr_eeprom_async_cancel(void);

//! @brief Read buffer from EEPROM area
//! @param[in] address EEPROM start address (starts at 0)
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @return true On success
//! @return false On failure

bool twr_eeprom_read(uint32_t address, void *buffer, size_t length);

//! @brief Return size of EEPROM area
//! @return Size of EEPROM area in bytes

size_t twr_eeprom_get_size(void);

//! @}

#endif // _TWR_EEPROM_H
