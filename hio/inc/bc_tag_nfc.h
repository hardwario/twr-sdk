#ifndef _HIO_TAG_NFC_H
#define _HIO_TAG_NFC_H

#include <hio_i2c.h>

//! @addtogroup hio_tag_nfc hio_tag_nfc
//! @brief Driver for HARDWARIO NFC Module
//! @{

//! @brief Default I2C address

#define HIO_TAG_NFC_I2C_ADDRESS_DEFAULT 0x08

#define HIO_TAG_NFC_BUFFER_SIZE 864

//! @brief Instance

typedef struct
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;

} hio_tag_nfc_t;

//! @brief NDEF Instance

typedef struct
{
    size_t _length;
    int _last_tnf_pos;
    uint16_t _encoded_size;
    uint8_t _buffer[HIO_TAG_NFC_BUFFER_SIZE];

} hio_tag_nfc_ndef_t;

//! @brief Initialize NFC Tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool hio_tag_nfc_init(hio_tag_nfc_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Read from memory
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read, must by modulo 16, max 880
//! @return true On success
//! @return false On failure

bool hio_tag_nfc_memory_read(hio_tag_nfc_t *self, void *buffer, size_t length);

//! @brief Write to memory
//! @param[in] self Instance
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written, must by modulo 16, max 880
//! @return true On success
//! @return false On failure

bool hio_tag_nfc_memory_write(hio_tag_nfc_t *self, void *buffer, size_t length);

//! @brief Write to memory
//! @param[in] self Instance
//! @param[in] ndef Instance
//! @return true On success
//! @return false On failure

bool hio_tag_nfc_memory_write_ndef(hio_tag_nfc_t *self, hio_tag_nfc_ndef_t *ndef);

//! @brief Initialize NDEF
//! @param[in] self Instance

void hio_tag_nfc_ndef_init(hio_tag_nfc_ndef_t *self);

//! @brief Add ndef text record
//! @param[in] self Instance
//! @param[in] *text String to be added
//! @param[in] *encoding Encoding for example "en"

bool hio_tag_nfc_ndef_add_text(hio_tag_nfc_ndef_t *self, const char *text, const char *encoding);

//! @brief Add ndef uri record
//! @param[in] self Instance
//! @param[in] *uri URI

bool hio_tag_nfc_ndef_add_uri(hio_tag_nfc_ndef_t *self, const char *uri);

//! @}

#endif // _HIO_TAG_NFC_H
