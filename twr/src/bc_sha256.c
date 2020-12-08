#include "twr_sha256.h"

#define _TWR_SHA256_RL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))
#define _TWR_SHA256_RR(a, b) (((a) >> (b)) | ((a) << (32 - (b))))
#define _TWR_SHA256_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define _TWR_SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define _TWR_SHA256_EP0(x) (_TWR_SHA256_RR(x, 2) ^ _TWR_SHA256_RR(x, 13) ^ _TWR_SHA256_RR(x, 22))
#define _TWR_SHA256_EP1(x) (_TWR_SHA256_RR(x, 6) ^ _TWR_SHA256_RR(x, 11) ^ _TWR_SHA256_RR(x, 25))
#define _TWR_SHA256_SIG0(x) (_TWR_SHA256_RR(x, 7) ^ _TWR_SHA256_RR(x, 18) ^ ((x) >> 3))
#define _TWR_SHA256_SIG1(x) (_TWR_SHA256_RR(x, 17) ^ _TWR_SHA256_RR(x, 19) ^ ((x) >> 10))

static const uint32_t _twr_sha256_k[64] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static void _twr_sha256_transform(twr_sha256_t *self, const uint8_t *buffer);

void twr_sha256_init(twr_sha256_t *self)
{
    memset(self, 0, sizeof(*self));

    self->_state[0] = 0x6a09e667;
    self->_state[1] = 0xbb67ae85;
    self->_state[2] = 0x3c6ef372;
    self->_state[3] = 0xa54ff53a;
    self->_state[4] = 0x510e527f;
    self->_state[5] = 0x9b05688c;
    self->_state[6] = 0x1f83d9ab;
    self->_state[7] = 0x5be0cd19;
}

void twr_sha256_update(twr_sha256_t *self, const void *buffer, size_t length)
{
    const uint8_t *p = buffer;

    for (size_t i = 0; i < length; i++)
    {
        self->_buffer[self->_length++] = p[i];

        if (self->_length == 64)
        {
            _twr_sha256_transform(self, self->_buffer);

            self->_bit_length += 512;
            self->_length = 0;
        }
    }
}

void twr_sha256_final(twr_sha256_t *self, uint8_t *hash, bool little_endian)
{
    size_t i = self->_length;

    if (i < 56)
    {
        self->_buffer[i++] = 0x80;

        while (i < 56)
        {
            self->_buffer[i++] = 0;
        }
    }
    else
    {
        self->_buffer[i++] = 0x80;

        while (i < 64)
        {
            self->_buffer[i++] = 0;
        }

        _twr_sha256_transform(self, self->_buffer);

        memset(self->_buffer, 0, 56);
    }

    self->_bit_length += (uint64_t) self->_length * 8;

    self->_buffer[63] = self->_bit_length;
    self->_buffer[62] = self->_bit_length >> 8;
    self->_buffer[61] = self->_bit_length >> 16;
    self->_buffer[60] = self->_bit_length >> 24;
    self->_buffer[59] = self->_bit_length >> 32;
    self->_buffer[58] = self->_bit_length >> 40;
    self->_buffer[57] = self->_bit_length >> 48;
    self->_buffer[56] = self->_bit_length >> 56;

    _twr_sha256_transform(self, self->_buffer);

    if (little_endian)
    {
        for (i = 0; i < 4; i++)
        {
            hash[i] = self->_state[7] >> i * 8;
            hash[i + 4] = self->_state[6] >> i * 8;
            hash[i + 8] = self->_state[5] >> i * 8;
            hash[i + 12] = self->_state[4] >> i * 8;
            hash[i + 16] = self->_state[3] >> i * 8;
            hash[i + 20] = self->_state[2] >> i * 8;
            hash[i + 24] = self->_state[1] >> i * 8;
            hash[i + 28] = self->_state[0] >> i * 8;
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            hash[i] = self->_state[0] >> (24 - i * 8);
            hash[i + 4] = self->_state[1] >> (24 - i * 8);
            hash[i + 8] = self->_state[2] >> (24 - i * 8);
            hash[i + 12] = self->_state[3] >> (24 - i * 8);
            hash[i + 16] = self->_state[4] >> (24 - i * 8);
            hash[i + 20] = self->_state[5] >> (24 - i * 8);
            hash[i + 24] = self->_state[6] >> (24 - i * 8);
            hash[i + 28] = self->_state[7] >> (24 - i * 8);
        }
    }
}

static void _twr_sha256_transform(twr_sha256_t *self, const uint8_t *buffer)
{
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];

    size_t i, j;

    for (i = 0, j = 0; i < 16; i++, j += 4)
    {
        m[i] = buffer[j] << 24 | buffer[j + 1] << 16 | buffer[j + 2] << 8 | buffer[j + 3];
    }

    for (; i < 64; i++)
    {
        m[i] = _TWR_SHA256_SIG1(m[i - 2]) + m[i - 7] + _TWR_SHA256_SIG0(m[i - 15]) + m[i - 16];
    }

    a = self->_state[0];
    b = self->_state[1];
    c = self->_state[2];
    d = self->_state[3];
    e = self->_state[4];
    f = self->_state[5];
    g = self->_state[6];
    h = self->_state[7];

    for (i = 0; i < 64; i++)
    {
        t1 = h + _TWR_SHA256_EP1(e) + _TWR_SHA256_CH(e, f, g) + _twr_sha256_k[i] + m[i];
        t2 = _TWR_SHA256_EP0(a) + _TWR_SHA256_MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    self->_state[0] += a;
    self->_state[1] += b;
    self->_state[2] += c;
    self->_state[3] += d;
    self->_state[4] += e;
    self->_state[5] += f;
    self->_state[6] += g;
    self->_state[7] += h;
}
