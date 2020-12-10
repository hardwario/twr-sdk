#ifndef _TWR_SHA256_H
#define _TWR_SHA256_H

#include "twr_common.h"

//! @addtogroup twr_sha256 twr_sha256
//! @brief Library for SHA256 hash
//! @{

//! @cond

typedef struct
{
    uint8_t _buffer[64];
    size_t _length;
    uint64_t _bit_length;
    uint32_t _state[8];

} twr_sha256_t;

//! @endcond

//! @brief Initialize SHA256 structure
//! @param[in] self SHA256 instance

void twr_sha256_init(twr_sha256_t *self);

//! @brief Compute SHA256 from data
//! @param[in] self SHA256 instance
//! @param[in] buffer data
//! @param[in] length data length

void twr_sha256_update(twr_sha256_t *self, const void *buffer, size_t length);

//! @brief Finalize SHA256 computation and read the final hash
//! @param[in] self SHA256 instance
//! @param[in] hash pointer to the array where the hash will be copied
//! @param[in] little_endian endian format

void twr_sha256_final(twr_sha256_t *self, uint8_t *hash, bool little_endian);

//! @}

#endif
