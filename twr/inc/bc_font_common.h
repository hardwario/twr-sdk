

#ifndef _TWR_FONT_COMMON
#define _TWR_FONT_COMMON

#include <twr_common.h>

typedef struct
{
    const uint8_t *image;
    uint8_t width;
    uint8_t heigth;
} twr_font_image_t;

typedef struct  {
    uint16_t code;
    const twr_font_image_t *image;
} twr_font_char_t;

typedef struct  {
    uint16_t length;
    const twr_font_char_t *chars;
} twr_font_t;

//
// Put another generated fonts here
//
extern const twr_font_t twr_font_ubuntu_11;
extern const twr_font_t twr_font_ubuntu_13;
extern const twr_font_t twr_font_ubuntu_15;
extern const twr_font_t twr_font_ubuntu_24;
extern const twr_font_t twr_font_ubuntu_28;
extern const twr_font_t twr_font_ubuntu_33;

//_TWR_FONT_COMMON
#endif
