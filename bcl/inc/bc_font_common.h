

#ifndef _BC_FONT_COMMON
#define _BC_FONT_COMMON

#include <bc_common.h>

typedef struct
{
    const uint8_t *image;
    uint8_t width;
    uint8_t heigth;
} tImage;

typedef struct  {
    uint16_t code;
    const tImage *image;
} tChar;

typedef struct  {
    uint16_t length;
    const tChar *chars;
} tFont;

//
// Put another generated fonts here
//
extern const tFont bc_font_ubuntu8;
extern const tFont bc_font_ubuntu12;
extern const tFont bc_font_ubuntu14;
extern const tFont bc_font_ubuntu24;

// Defines for backward compatibility
#define FontBig bc_font_ubuntu24
#define Font bc_font_ubuntu14

//_BC_FONT_COMMON
#endif
