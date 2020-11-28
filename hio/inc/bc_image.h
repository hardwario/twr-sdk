
#ifndef _BC_IMAGE
#define _BC_IMAGE

#include <bc_common.h>

 typedef struct {
     const uint8_t *data;
     uint16_t width;
     uint16_t height;
     uint8_t dataSize;
} bc_image_t;

#endif // _BC_IMAGE
