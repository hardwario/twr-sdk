#include <twr_tag_nfc.h>
#include <twr_timer.h>

#define _TWR_TAG_NFC_TNF_WELL_KNOWN 0x01

#define _TWR_TAG_NFC_BLOCK_SIZE 16

static bool _twr_tag_nfc_ndef_add_record_head(twr_tag_nfc_ndef_t *self, size_t payload_length);

bool twr_tag_nfc_init(twr_tag_nfc_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;

    self->_i2c_address = i2c_address;

    twr_timer_init();

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);

    twr_i2c_memory_transfer_t transfer;

    uint8_t config[_TWR_TAG_NFC_BLOCK_SIZE];

    transfer.device_address = self->_i2c_address;

    transfer.memory_address = 0x00;

    transfer.buffer = config;

    transfer.length = sizeof(config);

    if (!twr_i2c_memory_read(self->_i2c_channel, &transfer))
    {
        return false;
    }

    if ((config[12] | config[13] | config[14] | config[15]) == 0x00 ) //check lock tag
    {
        config[0] = self->_i2c_address << 1;

        config[12] = 0xE1;

        config[13] = 0x10;

        config[14] = 0x6D;

        config[15] = 0x00;

        if (!twr_i2c_memory_write(self->_i2c_channel, &transfer))
        {
            return false;
        }

        twr_timer_start();

        twr_timer_delay(5000);

        twr_timer_stop();
    }

    return true;
}

bool twr_tag_nfc_memory_read(twr_tag_nfc_t *self, void *buffer, size_t length)
{
    if ((length % _TWR_TAG_NFC_BLOCK_SIZE != 0) || (length > TWR_TAG_NFC_BUFFER_SIZE))
    {
        return false;
    }

    size_t read_length = 0;

    uint8_t address = 0x01;

    twr_i2c_memory_transfer_t transfer;

    transfer.device_address = self->_i2c_address;

    transfer.length = _TWR_TAG_NFC_BLOCK_SIZE;

    while ((read_length < length) && (address < 0x37))
    {
        transfer.memory_address = address;

        transfer.buffer = (uint8_t *) buffer + read_length;

        if (!twr_i2c_memory_read(self->_i2c_channel, &transfer))
        {
            return false;
        }

        address++;

        read_length += _TWR_TAG_NFC_BLOCK_SIZE;
    }

    return true;
}

bool twr_tag_nfc_memory_write(twr_tag_nfc_t *self, void *buffer, size_t length)
{
    if ((length % _TWR_TAG_NFC_BLOCK_SIZE != 0) || (length > TWR_TAG_NFC_BUFFER_SIZE))
    {
        return false;
    }

    size_t write_length = 0;

    uint8_t address = 0x01;

    twr_i2c_memory_transfer_t transfer;

    transfer.device_address = self->_i2c_address;

    transfer.length = _TWR_TAG_NFC_BLOCK_SIZE;

    while ((write_length < length) && (address < 0x37))
    {
        transfer.memory_address = address;

        transfer.buffer = (uint8_t *) buffer + write_length;

        if (!twr_i2c_memory_write(self->_i2c_channel, &transfer))
        {
            return false;
        }

        twr_timer_start();

        twr_timer_delay(5000);

        twr_timer_stop();

        address++;

        write_length += _TWR_TAG_NFC_BLOCK_SIZE;
    }

    return true;
}

bool twr_tag_nfc_memory_write_ndef(twr_tag_nfc_t *self, twr_tag_nfc_ndef_t *ndef)
{
    size_t length = ndef->_length;

    if (((length % _TWR_TAG_NFC_BLOCK_SIZE) != 0))
    {
        length = ((length / _TWR_TAG_NFC_BLOCK_SIZE) + 1) * _TWR_TAG_NFC_BLOCK_SIZE;
    }

    return twr_tag_nfc_memory_write(self, ndef->_buffer, length);
}

void twr_tag_nfc_ndef_init(twr_tag_nfc_ndef_t *self)
{
    self->_buffer[0] = 0x03;

    self->_buffer[1] = 0x00;

    self->_buffer[2] = 0xFE;

    self->_length = 3;

    self->_encoded_size = 0;

    self->_last_tnf_pos = 0;
}

bool twr_tag_nfc_ndef_add_text(twr_tag_nfc_ndef_t *self, const char *text, const char *encoding)
{
    size_t text_length = strlen(text);

    size_t encoding_length = strlen(encoding);

    if (!_twr_tag_nfc_ndef_add_record_head(self, encoding_length + 1 + text_length))
    {
        return false;
    }

    self->_buffer[self->_length++] = 0x54; // type RTD_TEXT

    self->_buffer[self->_length++] = encoding_length;

    memcpy(self->_buffer + self->_length, encoding, encoding_length);

    self->_length += encoding_length;

    memcpy(self->_buffer + self->_length, text, text_length);

    self->_length += text_length;

    self->_buffer[self->_length++] = 0xFE; // terminator

    return true;
}

bool twr_tag_nfc_ndef_add_uri(twr_tag_nfc_ndef_t *self, const char *uri)
{
    size_t uri_length = strlen(uri);

    if (!_twr_tag_nfc_ndef_add_record_head(self, 1 + uri_length))
    {
        return false;
    }
    self->_buffer[self->_length++] = 0x55; // type RTD_TEXT

    self->_buffer[self->_length++] = 0;

    memcpy(self->_buffer + self->_length, uri, uri_length);

    self->_length += uri_length;

    self->_buffer[self->_length++] = 0xFE; // terminator

    return true;
}

static bool _twr_tag_nfc_ndef_add_record_head(twr_tag_nfc_ndef_t *self, size_t payload_length)
{
    size_t head_length = 2 + (payload_length > 0xff ? 4 : 1) + 1; // tnf + type_length + type

    if ((head_length + payload_length > TWR_TAG_NFC_BUFFER_SIZE))
    {
        return false;
    }

    if (self->_last_tnf_pos != 0)
    {
        self->_buffer[self->_last_tnf_pos] &= ~0x40; // remove flag last record
    }

    if (self->_encoded_size == 0)
    {
        self->_buffer[1] = 0xff;

        self->_length = 4;
    }
    else
    {
        self->_length--;
    }

    self->_encoded_size += head_length + payload_length;

    self->_buffer[2] = self->_encoded_size >> 8;

    self->_buffer[3] = self->_encoded_size;

    self->_buffer[self->_length] = _TWR_TAG_NFC_TNF_WELL_KNOWN | 0x40;

    if (self->_length == 4)
    {
        self->_buffer[self->_length] |= 0x80;
    }

    if (payload_length <= 0xff)
    {
        self->_buffer[self->_length] |= 0x10;
    }

    self->_last_tnf_pos = self->_length;

    self->_length++;

    self->_buffer[self->_length++] = 1; // type length

    if (payload_length <= 0xFF) // short record
    {
        self->_buffer[self->_length++] = payload_length;
    }
    else // long format
    {
        self->_buffer[self->_length++] = payload_length >> 24;

        self->_buffer[self->_length++] = payload_length >> 16;

        self->_buffer[self->_length++] = payload_length >> 8;

        self->_buffer[self->_length++] = payload_length & 0xff;
    }

    return true;
}
