#include <twr_crc.h>

uint8_t twr_crc8(const uint8_t polynomial, const void *buffer, size_t length, const uint8_t initialization)
{
    uint8_t crc = initialization;
    uint8_t *_buffer = (uint8_t *) buffer;

    while (length--)
    {
        crc ^= *_buffer++;

        for ( int i = 8; i; --i )
        {
            crc = ( crc & 0x80 )
            ? (crc << 1) ^ polynomial
            : (crc << 1);
        }
    }
    return crc;
}
