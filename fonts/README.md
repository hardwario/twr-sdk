
Use this opensource multiplatform tool to regenerate fonts or add more.
http://www.riuson.com/lcd-image-converter

In configuration select the template_bc_font.tmpl so you can easily generate C files in the right format.

Add your new fonts to the bc_font_common.h:

extern const tFont YourNewFontName;

