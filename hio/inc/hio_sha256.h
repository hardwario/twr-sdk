#ifndef _HIO_SHA256_H
#define _HIO_SHA256_H

#include "hio_common.h"

//! @addtogroup hio_sha256 hio_sha256
//! @brief Library for SHA256 hash
//! @{

//! @cond

typedef struct
{
    uint8_t _buffer[64];
    size_t _length;
    uint64_t _bit_length;
    uint32_t _state[8];

} hio_sha256_t;

//! @endcond

//! @brief Initialize SHA256 structure
//! @param[in] self SHA256 instance

void hio_sha256_init(hio_sha256_t *self);

//! @brief Compute SHA256 from data
//! @param[in] self SHA256 instance
//! @param[in] buffer data
//! @param[in] length data length

void hio_sha256_update(hio_sha256_t *self, const void *buffer, size_t length);

//! @brief Finalize SHA256 computation and read the final hash
//! @param[in] self SHA256 instance
//! @param[in] hash pointer to the array where the hash will be copied
//! @param[in] little_endian endian format

void hio_sha256_final(hio_sha256_t *self, uint8_t *hash, bool little_endian);

//! @}

#endif
