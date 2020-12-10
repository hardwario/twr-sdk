
#ifndef _TWR_IMAGE
#define _TWR_IMAGE

#include <twr_common.h>

 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
} twr_image_t;

#endif // _TWR_IMAGE
