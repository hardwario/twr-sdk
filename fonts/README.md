
Use this opensource multiplatform tool to regenerate fonts or add more.
http://www.riuson.com/lcd-image-converter

In configuration select the template_bc_font.tmpl so you can easily generate C files in the right format.
In  the Options you have to import Config_preset_monochrome_ISO8859-2.xml to set monochrome, codepage and other stuff.

Add your new fonts to the sdk/bcl/inc/bc_font_common.h:

extern const bc_font_t YourNewFontName;

The format for new fonts is
bc_font_[name][size][_bold|_italic]

The height of the font in the font name is the one used font size. The real height of the generated bitmap font could be higher.
