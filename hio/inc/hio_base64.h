#ifndef _HIO_BASE64_H
#define _HIO_BASE64_H

#include "hio_common.h"

//! @addtogroup hio_base64 hio_base64
//! @brief BASE64
//! @{

//! @brief BASE64 encode
//! @param[out] output Pointer to destination buffer
//! @param[in,out] output_length Size of destination buffer, Number of used bytes
//! @param[in] input Pointer to source buffer
//! @param[in] input_length Number of bytes
//! @return true On success
//! @return false On failure

bool hio_base64_encode(char *output, size_t *output_length, uint8_t *input, size_t input_length);

//! @brief BASE64 decode
//! @param[out] output Pointer to destination buffer
//! @param[in,out] output_length Size of destination buffer, Number of used bytes
//! @param[in] input Pointer to source buffer
//! @param[in] input_length Number of bytes
//! @return true On success
//! @return false On failure

bool hio_base64_decode(uint8_t *output, size_t *output_length, char *input, size_t input_length);

//! @brief BASE64 Calculate encode length
//! @param[in] length Number of bytes

size_t hio_base64_calculate_encode_length(size_t length);

//! @brief BASE64 Calculate decode length
//! @param[in] input Pointer to source buffer
//! @param[in] length Number of bytes

size_t hio_base64_calculate_decode_length(char *input, size_t length);

//! @}

#endif // _HIO_BASE64_H
