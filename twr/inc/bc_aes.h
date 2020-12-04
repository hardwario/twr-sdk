#ifndef _BC_AES_H
#define _BC_AES_H

#include <bc_common.h>

//! @addtogroup bc_aes bc_aes
//! @brief Driver for AES
//! @{

#define BC_AES_KEYLEN 128
#define BC_AES_IVLEN 128

//! @brief AES 128-bit Key

typedef uint32_t bc_aes_key_t[BC_AES_KEYLEN/8/4];

//! @brief AES 128-bit Initialization vector

typedef uint32_t bc_aes_iv_t[BC_AES_IVLEN/8/4];

//! @brief Initialize AES

void bc_aes_init(void);

//! @brief AES derivation decryption key from encryption key
//! @param[out] key 128-bit decryption key
//! @param[in] key 128-bit encryption key
//! @return true On success
//! @return false On failure

bool bc_aes_key_derivation(bc_aes_key_t decryption_key, const bc_aes_key_t key);

//! @brief AES encryption Electronic CodeBook (ECB)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit encryption key
//! @return true On success
//! @return false On failure

bool bc_aes_ecb_encrypt(void *buffer_out, const void *buffer_in, const size_t length, const bc_aes_key_t key);

//! @brief AES decryption Electronic CodeBook (ECB)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit decryption key
//! @return true On success
//! @return false On failure

bool bc_aes_ecb_decrypt(void *buffer_out, const void *buffer_in, size_t length, bc_aes_key_t key);

//! @brief AES Cipher block chaining (CBC)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit encryption key
//! @param[in] iv 128-bit Initialization vector
//! @return true On success
//! @return false On failure

bool bc_aes_cbc_encrypt(void *buffer_out, const void *buffer_in, size_t length, bc_aes_key_t key, bc_aes_iv_t iv);

//! @brief AES Cipher block chaining (CBC)
//! @param[out] buffer_out Pointer to destination buffer
//! @param[in] buffer_in Pointer to source buffer
//! @param[in] length Number of bytes
//! @param[in] key 128-bit decryption key
//! @param[in] iv 128-bit Initialization vector
//! @return true On success
//! @return false On failure

bool bc_aes_cbc_decrypt(void *buffer_out, const void *buffer_in, size_t length, bc_aes_key_t key, bc_aes_iv_t iv);

//! @brief Create key from uint8 array
//! @param[out] key key 128-bit encryption key
//! @param[in] buffer Pointer to source buffer

void bc_aes_key_from_uint8(bc_aes_key_t key, const uint8_t *buffer);

//! @brief Create Initialization vector from uint8 array
//! @param[out] iv 128-bit Initialization vector
//! @param[in] buffer Pointer to source buffer

void bc_aes_iv_from_uint8(bc_aes_iv_t iv, const uint8_t *buffer);

//! @}

#endif // _BC_AES_H
