

#ifndef _BC_FONT_COMMON
#define _BC_FONT_COMMON

#include <bc_common.h>

typedef struct
{
    const uint8_t *image;
    uint8_t width;
    uint8_t heigth;
} bc_font_image_t;

typedef struct  {
    uint16_t code;
    const bc_font_image_t *image;
} bc_font_char_t;

typedef struct  {
    uint16_t length;
    const bc_font_char_t *chars;
} bc_font_t;

//
// Put another generated fonts here
//
extern const bc_font_t bc_font_ubuntu_11;
extern const bc_font_t bc_font_ubuntu_13;
extern const bc_font_t bc_font_ubuntu_15;
extern const bc_font_t bc_font_ubuntu_24;
extern const bc_font_t bc_font_ubuntu_28;
extern const bc_font_t bc_font_ubuntu_33;

//_BC_FONT_COMMON
#endif
