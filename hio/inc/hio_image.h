
#ifndef _HIO_IMAGE
#define _HIO_IMAGE

#include <hio_common.h>

 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
} hio_image_t;

#endif // _HIO_IMAGE
