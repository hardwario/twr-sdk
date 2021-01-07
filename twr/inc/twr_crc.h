#ifndef _TWR_CRC_H
#define _TWR_CRC_H

#include <twr_common.h>

//! @addtogroup twr_crc twr_crc
//! @brief Calculate crc
//! @{

//! @brief Calculate CRC8
//! @param[in] polynomial
//! @param[in] buffer Data buffer
//! @param[in] length Data buffer length
//! @param[in] initialization data
//! @return crc

uint8_t twr_crc8(const uint8_t polynomial, const void *buffer, size_t length, const uint8_t initialization);

//! @}

#endif // _TWR_CRC_H
