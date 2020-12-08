#include <twr_base64.h>

const char twr_b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static uint8_t twr_base64_lookup(char c);

bool twr_base64_encode(char *output, size_t *output_length, uint8_t *input, size_t input_length)
{
    size_t i = 0, j = 0;
    size_t encode_length = 0;
    uint8_t a3[3];
    uint8_t a4[4];

    for (; input_length != 0; input_length--)
    {
        a3[i++] = *(input++);
        if (i == 3)
        {
            a4[0] = (uint8_t) ((a3[0] & 0xfc) >> 2);
            a4[1] = (uint8_t) (((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4));
            a4[2] = (uint8_t) (((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6));
            a4[3] = (uint8_t) (a3[2] & 0x3f);

            for (i = 0; i < 4; i++)
            {
                output[encode_length++] = twr_b64_alphabet[a4[i]];
                if (encode_length > *output_length)
                {
                    return false;
                }
            }

            i = 0;
        }
    }

    if (i != 0)
    {
        for (j = i; j < 3; j++)
        {
            a3[j] = 0x00;
        }

        a4[0] = (uint8_t) ((a3[0] & 0xfc) >> 2);
        a4[1] = (uint8_t) (((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4));
        a4[2] = (uint8_t) (((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6));
        a4[3] = (uint8_t) (a3[2] & 0x3f);

        for (j = 0; j < (i + 1); j++)
        {
            output[encode_length++] = twr_b64_alphabet[a4[j]];
            if (encode_length > *output_length)
            {
                return false;
            }

        }

        while (i++ < 3)
        {
            output[encode_length++] = '=';
            if (encode_length > *output_length)
            {
                return false;
            }
        }
    }

    output[encode_length] = 0x00;

    *output_length = encode_length;

    return true;
}

bool twr_base64_decode(uint8_t *output, size_t *output_length, char *input, size_t input_length)
{
    size_t i = 0, j = 0;
    size_t decode_length = 0;
    uint8_t a3[3];
    uint8_t a4[4];

    for (; input_length != 0; input_length--)
    {
        if (*input == '=')
        {
            break;
        }

        a4[i++] = (uint8_t) *(input++);
        if (i == 4)
        {
            for (i = 0; i < 4; i++)
            {
                a4[i] = twr_base64_lookup(a4[i]);
            }

            a3[0] = (uint8_t) ((a4[0] << 2) + ((a4[1] & 0x30) >> 4));
            a3[1] = (uint8_t) (((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2));
            a3[2] = (uint8_t) (((a4[2] & 0x3) << 6) + a4[3]);

            for (i = 0; i < 3; i++)
            {
                output[decode_length++] = a3[i];
                if (decode_length > *output_length)
                {
                    return false;
                }
            }

            i = 0;
        }
    }

    if (i != 0)
    {
        for (j = i; j < 4; j++)
        {
            a4[j] = 0x00;
        }

        for (j = 0; j < 4; j++)
        {
            a4[j] = twr_base64_lookup(a4[j]);
        }

        a3[0] = (uint8_t) ((a4[0] << 2) + ((a4[1] & 0x30) >> 4));
        a3[1] = (uint8_t) (((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2));
        a3[2] = (uint8_t) (((a4[2] & 0x3) << 6) + a4[3]);

        for (j = 0; j < (i - 1); j++)
        {
            output[decode_length++] = a3[j];
            if (decode_length > *output_length)
            {
                return false;
            }
        }
    }

    output[decode_length] = 0x00;

    *output_length = decode_length;

    return true;
}

size_t twr_base64_calculate_encode_length(size_t length)
{
    size_t n = (int) length;
    return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

size_t twr_base64_calculate_decode_length(char *input, size_t length)
{
    size_t i = 0;
    size_t num_eq = 0;

    for (i = length - 1; input[i] == '='; i--)
    {
        num_eq++;
    }

    return ((6 * length) / 8) - num_eq;
}

static uint8_t twr_base64_lookup(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return (uint8_t) (c - 'A');
    }
    if (c >= 'a' && c <= 'z')
    {
        return (uint8_t) (c - 71);
    }
    if (c >= '0' && c <= '9')
    {
        return (uint8_t) (c + 4);
    }
    if (c == '+')
    {
        return 62;
    }
    if (c == '/')
    {
        return 63;
    }
    return (uint8_t) -1;
}
