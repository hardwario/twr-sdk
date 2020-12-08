#ifndef _TWR_AES_H
#define _TWR_AES_H

#include <twr_common.h>

//! @addtogroup twr_aes twr_aes
//! @brief Driver for AES
//! @{

#define TWR_AES_KEYLEN 128
#define TWR_AES_IVLEN 128

//! @brief AES 128-bit Key

typedef uint32_t twr_aes_key_t[TWR_AES_KEYLEN/8/4];

//! @brief AES 128-bit Initialization vector

typedef uint32_t twr_aes_iv_t[TWR_AES_IVLEN/8/4];

//! @brief Initialize AES

void twr_aes_init(void);

//! @brief AES derivation decryption key from encryption key
//! @param[out] key 128-bit decryption key
//! @param[in] key 128-bit encryption key
//! @return true On success
//! @return false On failure

bool twr_aes_key_derivation(twr_aes_key_t decryption_key, const twr_aes_key_t key);

//! @brief AES encryption Electronic CodeBook (ECB)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit encryption key
//! @return true On success
//! @return false On failure

bool twr_aes_ecb_encrypt(void *buffer_out, const void *buffer_in, const size_t length, const twr_aes_key_t key);

//! @brief AES decryption Electronic CodeBook (ECB)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit decryption key
//! @return true On success
//! @return false On failure

bool twr_aes_ecb_decrypt(void *buffer_out, const void *buffer_in, size_t length, twr_aes_key_t key);

//! @brief AES Cipher block chaining (CBC)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit encryption key
//! @param[in] iv 128-bit Initialization vector
//! @return true On success
//! @return false On failure

bool twr_aes_ctwr_encrypt(void *buffer_out, const void *buffer_in, size_t length, twr_aes_key_t key, twr_aes_iv_t iv);

//! @brief AES Cipher block chaining (CBC)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit decryption key
//! @param[in] iv 128-bit Initialization vector
//! @return true On success
//! @return false On failure

bool twr_aes_ctwr_decrypt(void *buffer_out, const void *buffer_in, size_t length, twr_aes_key_t key, twr_aes_iv_t iv);

//! @brief Create key from uint8 array
//! @param[out] key key 128-bit encryption key
//! @param[in] buffer Pointer to source buffer

void twr_aes_key_from_uint8(twr_aes_key_t key, const uint8_t *buffer);

//! @brief Create Initialization vector from uint8 array
//! @param[out] iv 128-bit Initialization vector
//! @param[in] buffer Pointer to source buffer

void twr_aes_iv_from_uint8(twr_aes_iv_t iv, const uint8_t *buffer);

//! @}

#endif // _TWR_AES_H
