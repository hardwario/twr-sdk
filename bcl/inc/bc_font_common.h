

#ifndef _BC_FONT_COMMON
#define _BC_FONT_COMMON

#include <bc_common.h>

typedef struct
{
    const uint8_t *image;
    uint8_t width;
    uint8_t heigth;
    /*uint8_t dont_know;*/
} tImage;

 typedef struct  {
     uint16_t code;
     const tImage *image;
     } tChar;

 typedef struct  {
     uint16_t length;
     const tChar *chars;
     } tFont;

//_BC_FONT_COMMON
#endif
