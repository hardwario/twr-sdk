#ifndef _BC_TAG_NFC_H
#define _BC_TAG_NFC_H

#include <bc_i2c.h>

//! @addtogroup bc_tag_nfc bc_tag_nfc
//! @brief Driver for BigClown NFC Tag
//! @{

//! @brief Default I2C address

#define BC_TAG_NFC_I2C_ADDRESS_DEFAULT 0x08

#define BC_TAG_NFC_BUFFER_SIZE 864

//! @brief Instance

typedef struct
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;

} bc_tag_nfc_t;

//! @brief NDEF Instance

typedef struct
{
    size_t _length;
    int _last_tnf_pos;
    uint16_t _encoded_size;
    uint8_t _buffer[BC_TAG_NFC_BUFFER_SIZE];

} bc_tag_nfc_ndef_t;

//! @brief Initialize NFC Tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool bc_tag_nfc_init(bc_tag_nfc_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Read from memory
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read, must by modulo 16, max 880
//! @return true On success
//! @return false On failure

bool bc_tag_nfc_memory_read(bc_tag_nfc_t *self, void *buffer, size_t length);

//! @brief Write to memory
//! @param[in] self Instance
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written, must by modulo 16, max 880
//! @return true On success
//! @return false On failure

bool bc_tag_nfc_memory_write(bc_tag_nfc_t *self, void *buffer, size_t length);

//! @brief Write to memory
//! @param[in] self Instance
//! @param[in] ndef Instance
//! @return true On success
//! @return false On failure

bool bc_tag_nfc_memory_write_ndef(bc_tag_nfc_t *self, bc_tag_nfc_ndef_t *ndef);

//! @brief Initialize NDEF
//! @param[in] self Instance

void bc_tag_nfc_ndef_init(bc_tag_nfc_ndef_t *self);

//! @brief Add ndef text record
//! @param[in] self Instance
//! @param[in] *text String to be added
//! @param[in] *encoding Encoding for example "en"

bool bc_tag_nfc_ndef_add_text(bc_tag_nfc_ndef_t *self, const char *text, const char *encoding);

//! @brief Add ndef uri record
//! @param[in] self Instance
//! @param[in] *uri URI

bool bc_tag_nfc_ndef_add_uri(bc_tag_nfc_ndef_t *self, const char *uri);

//! @}

#endif // _BC_TAG_NFC_H
