#include "base64.h"

// TODO Support padding check (aka strict mode)
// TODO Check input length constraints (blocks of four bytes)
// TODO Re-implement base64_char_index to lookup table (optional for speed improvement)

static const char *base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static uint8_t base64_char_index(char c);

bool base64_encode(const uint8_t *input, uint32_t input_length, char *output, uint32_t *output_length)
{
    uint32_t i;

    uint8_t byte_array_3[3] = { 0, 0, 0 };
    uint8_t byte_array_4[4] = { 0, 0, 0, 0 };

    *output_length = 0;

    for (i = 0; input_length != 0; i++, input_length--, input++)
    {
        byte_array_3[i] = *input;

        if (i == 3)
        {
            byte_array_4[0] = (byte_array_3[0] & 0xfc) >> 2;
            byte_array_4[1] = ((byte_array_3[0] & 0x03) << 4) | ((byte_array_3[1] & 0xf0) >> 4);
            byte_array_4[2] = ((byte_array_3[1] & 0x0f) << 2) | ((byte_array_3[2] & 0xc0) >> 6);
            byte_array_4[3] = byte_array_3[2] & 0x3f;

            output[(*output_length)++] = base64_chars[byte_array_4[0]];
            output[(*output_length)++] = base64_chars[byte_array_4[1]];
            output[(*output_length)++] = base64_chars[byte_array_4[2]];
            output[(*output_length)++] = base64_chars[byte_array_4[3]];

            i = 0;
        }
    }

    if (i != 0)
    {
        uint32_t j;

        for (j = i; j < 3; j++)
        {
            byte_array_3[j] = 0;
        }

        byte_array_4[0] = (byte_array_3[0] & 0xfc) >> 2;
        byte_array_4[1] = ((byte_array_3[0] & 0x03) << 4) | ((byte_array_3[1] & 0xf0) >> 4);
        byte_array_4[2] = ((byte_array_3[1] & 0x0f) << 2) | ((byte_array_3[2] & 0xc0) >> 6);
        byte_array_4[3] = byte_array_3[2] & 0x3f;

        for (j = 0; j < (i + 1); j++)
        {
            output[(*output_length)++] = base64_chars[byte_array_4[j]];
        }

        while (i++ < 3)
        {
            output[(*output_length)++] = '=';
        }
    }

    return true;
}

bool base64_decode(const char *input, uint32_t input_length, uint8_t *output, uint32_t *output_length)
{
    uint32_t i = 0;

    uint8_t byte_array_3[3] =
    { 0, 0, 0 };
    uint8_t byte_array_4[4] =
    { 0, 0, 0, 0 };

    *output_length = 0;

    while (input_length-- && (*input != '=') && (isalnum(*input) || (*input == '+') || (*input == '/')))
    {
        byte_array_4[i++] = (uint8_t) *(input++);

        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                byte_array_4[i] = base64_char_index(byte_array_4[i]);
            }

            byte_array_3[0] = (byte_array_4[0] << 2) + ((byte_array_4[1] & 0x30) >> 4);
            byte_array_3[1] = ((byte_array_4[1] & 0x0f) << 4) + ((byte_array_4[2] & 0x3c) >> 2);
            byte_array_3[2] = ((byte_array_4[2] & 0x03) << 6) + byte_array_4[3];

            output[(*output_length)++] = byte_array_3[0];
            output[(*output_length)++] = byte_array_3[1];
            output[(*output_length)++] = byte_array_3[2];

            i = 0;
        }
    }

    if (i != 0)
    {
        uint32_t j;

        for (j = i; j < 4; j++)
        {
            byte_array_4[j] = 0;
        }

        byte_array_4[0] = base64_char_index(byte_array_4[0]);
        byte_array_4[1] = base64_char_index(byte_array_4[1]);
        byte_array_4[2] = base64_char_index(byte_array_4[2]);
        byte_array_4[3] = base64_char_index(byte_array_4[3]);

        byte_array_3[0] = (byte_array_4[0] << 2) | ((byte_array_4[1] & 0x30) >> 4);
        byte_array_3[1] = ((byte_array_4[1] & 0xf) << 4) | ((byte_array_4[2] & 0x3c) >> 2);
        byte_array_3[2] = ((byte_array_4[2] & 0x3) << 6) | byte_array_4[3];

        for (j = 0; j < (i - 1); j++)
        {
            output[(*output_length)++] = byte_array_3[j];
        }
    }

    return true;
}

static uint8_t base64_char_index(char c)
{
    uint8_t i;

    const char *p;

    for (i = 0, p = base64_chars; *p != '\0'; i++, p++)
    {
        if (*p == c)
        {
            return i;
        }
    }

    return 0xff;
}

size_t base64_calculate_encode_length(size_t length)
{
    size_t n = (int) length;
    return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

size_t base64_calculate_decode_length(const char *input, size_t length)
{
    size_t i = 0;
    size_t num_eq = 0;

    for (i = length - 1; input[i] == '='; i--)
    {
        num_eq++;
    }

    return ((6 * length) / 8) - num_eq;
}
