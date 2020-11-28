

#ifndef _HIO_FONT_COMMON
#define _HIO_FONT_COMMON

#include <hio_common.h>

typedef struct
{
    const uint8_t *image;
    uint8_t width;
    uint8_t heigth;
} hio_font_image_t;

typedef struct  {
    uint16_t code;
    const hio_font_image_t *image;
} hio_font_char_t;

typedef struct  {
    uint16_t length;
    const hio_font_char_t *chars;
} hio_font_t;

//
// Put another generated fonts here
//
extern const hio_font_t hio_font_ubuntu_11;
extern const hio_font_t hio_font_ubuntu_13;
extern const hio_font_t hio_font_ubuntu_15;
extern const hio_font_t hio_font_ubuntu_24;
extern const hio_font_t hio_font_ubuntu_28;
extern const hio_font_t hio_font_ubuntu_33;

//_HIO_FONT_COMMON
#endif
