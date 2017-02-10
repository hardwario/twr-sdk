#ifndef _BC_EEPROM_H
#define _BC_EEPROM_H

#include <bc_common.h>

//! @addtogroup bc_eeprom bc_eeprom
//! @brief Driver for internal EEPROM memory
//! @{

//! @brief Write buffer to EEPROM area and verify it
//! @param[in] address EEPROM start address (starts at 0)
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return true on success
//! @return false on failure

bool bc_eeprom_write(uint32_t address, const void *buffer, size_t length);

//! @brief Read buffer from EEPROM area
//! @param[in] address EEPROM start address (starts at 0)
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @return true on success
//! @return false on failure

bool bc_eeprom_read(uint32_t address, void *buffer, size_t length);

//! @brief Return size of EEPROM area
//! @return Size of EEPROM area in bytes

size_t bc_eeprom_get_size(void);

//! @}

#endif // _BC_EEPROM_H
