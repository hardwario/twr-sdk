#ifndef _HIO_AES_H
#define _HIO_AES_H

#include <hio_common.h>

//! @addtogroup hio_aes hio_aes
//! @brief Driver for AES
//! @{

#define HIO_AES_KEYLEN 128
#define HIO_AES_IVLEN 128

//! @brief AES 128-bit Key

typedef uint32_t hio_aes_key_t[HIO_AES_KEYLEN/8/4];

//! @brief AES 128-bit Initialization vector

typedef uint32_t hio_aes_iv_t[HIO_AES_IVLEN/8/4];

//! @brief Initialize AES

void hio_aes_init(void);

//! @brief AES derivation decryption key from encryption key
//! @param[out] key 128-bit decryption key
//! @param[in] key 128-bit encryption key
//! @return true On success
//! @return false On failure

bool hio_aes_key_derivation(hio_aes_key_t decryption_key, const hio_aes_key_t key);

//! @brief AES encryption Electronic CodeBook (ECB)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit encryption key
//! @return true On success
//! @return false On failure

bool hio_aes_ecb_encrypt(void *buffer_out, const void *buffer_in, const size_t length, const hio_aes_key_t key);

//! @brief AES decryption Electronic CodeBook (ECB)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit decryption key
//! @return true On success
//! @return false On failure

bool hio_aes_ecb_decrypt(void *buffer_out, const void *buffer_in, size_t length, hio_aes_key_t key);

//! @brief AES Cipher block chaining (CBC)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit encryption key
//! @param[in] iv 128-bit Initialization vector
//! @return true On success
//! @return false On failure

bool hio_aes_chio_encrypt(void *buffer_out, const void *buffer_in, size_t length, hio_aes_key_t key, hio_aes_iv_t iv);

//! @brief AES Cipher block chaining (CBC)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit decryption key
//! @param[in] iv 128-bit Initialization vector
//! @return true On success
//! @return false On failure

bool hio_aes_chio_decrypt(void *buffer_out, const void *buffer_in, size_t length, hio_aes_key_t key, hio_aes_iv_t iv);

//! @brief Create key from uint8 array
//! @param[out] key key 128-bit encryption key
//! @param[in] buffer Pointer to source buffer

void hio_aes_key_from_uint8(hio_aes_key_t key, const uint8_t *buffer);

//! @brief Create Initialization vector from uint8 array
//! @param[out] iv 128-bit Initialization vector
//! @param[in] buffer Pointer to source buffer

void hio_aes_iv_from_uint8(hio_aes_iv_t iv, const uint8_t *buffer);

//! @}

#endif // _HIO_AES_H
