#ifndef _BASE64_H
#define _BASE64_H

#include <bc_common.h>

bool base64_encode(const uint8_t *input, uint32_t input_length, char *output, uint32_t *output_length);
bool base64_decode(const char *input, uint32_t input_length, uint8_t *output, uint32_t *output_length);
size_t base64_calculate_encode_length(size_t length);
size_t base64_calculate_decode_length(const char *input, size_t length);

#endif
