#include <twr_gfx.h>

void twr_gfx_init(twr_gfx_t *self, void *display, const twr_gfx_driver_t *driver)
{
    memset(self, 0, sizeof(*self));
    self->_display = display;
    self->_driver = driver;

    self->_caps = driver->get_caps(self->_display);
}

bool twr_gfx_display_is_ready(twr_gfx_t *self)
{
    return self->_driver->is_ready(self->_display);
}

twr_gfx_caps_t twr_gfx_get_caps(twr_gfx_t *self)
{
    return self->_caps;
}

void twr_gfx_clear(twr_gfx_t *self)
{
    self->_driver->clear(self->_display);
}

void twr_gfx_set_font(twr_gfx_t *self, const twr_font_t *font)
{
    self->_font = font;
}

void twr_gfx_set_rotation(twr_gfx_t *self, twr_gfx_rotation_t rotation)
{
    self->_rotation = rotation;
}

twr_gfx_rotation_t twr_gfx_get_rotation(twr_gfx_t *self)
{
    return self->_rotation;
}

void twr_gfx_draw_pixel(twr_gfx_t *self, int x, int y, uint32_t color)
{
    if (x >= self->_caps.width || y >= self->_caps.height || x < 0 || y < 0)
    {
        return;
    }

    int tmp;

    switch (self->_rotation)
    {
        case TWR_GFX_ROTATION_90:
        {
            tmp = x;
            x = self->_caps.height - 1 - y;
            y = tmp;
            break;
        }
        case TWR_GFX_ROTATION_180:
        {
            x = self->_caps.width - 1 - x;
            y = self->_caps.height - 1 - y;
            break;
        }
        case TWR_GFX_ROTATION_270:
        {
            tmp = y;
            y = self->_caps.width - 1 - x;
            x = tmp;
            break;
        }
        case TWR_GFX_ROTATION_0:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    self->_driver->draw_pixel(self->_display, x, y, color);
}

int twr_gfx_draw_char(twr_gfx_t *self, int left, int top, uint8_t ch, uint32_t color)
{
    if (!self->_font)
    {
        return 0;
    }

    const twr_font_t *font = self->_font;

    int w = 0;
    uint8_t h = 0;
    uint16_t i;
    uint16_t x;
    uint16_t y;
    uint8_t bytes;

    for (i = 0; i < font->length; i++)
    {
        if (font->chars[i].code == ch)
        {
            w = font->chars[i].image->width;
            h = font->chars[i].image->heigth;

            bytes = (w + 7) / 8;

            for (y = 0; y < h; y++)
            {
                for (x = 0; x < w; x++)
                {
                    uint32_t byteIndex = x / 8;
                    byteIndex += y * bytes;

                    uint8_t bitMask = 1 << (7 - (x % 8));

                    if ((font->chars[i].image->image[byteIndex] & bitMask) == 0)
                    {
                        twr_gfx_draw_pixel(self, left + x, top + y, color);
                    }
                }
            }
        }
    }

    return w;
}

int twr_gfx_calc_char_width(twr_gfx_t *self, uint8_t ch)
{
    if (!self->_font)
    {
        return 0;
    }

    const twr_font_t *font = self->_font;

    for (int i = 0; i < font->length; i++)
    {
        if (font->chars[i].code == ch)
        {
            return font->chars[i].image->width;
        }
    }

    return 0;
}

int twr_gfx_draw_string(twr_gfx_t *self, int left, int top, char *str, uint32_t color)
{
    while(*str)
    {
        left += twr_gfx_draw_char(self, left, top, *str, color);
        str++;
    }
    return left;
}

int twr_gfx_calc_string_width(twr_gfx_t *self,  char *str)
{
    int width = 0;
    while(*str)
    {
        width += twr_gfx_calc_char_width(self, *str);
        str++;
    }
    return width;
}

int twr_gfx_printf(twr_gfx_t *self, int left, int top, uint32_t color, char *format, ...)
{
    va_list ap;

    char buffer[32];

    va_start(ap, format);

    vsnprintf(buffer, sizeof(buffer), format, ap);

    va_end(ap);

    return twr_gfx_draw_string(self, left, top, buffer, color);
}

void twr_gfx_draw_line(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color)
{
    int tmp;

    if (y0 == y1)
    {
        if (x0 > x1)
        {
            tmp = x0;
            x0 = x1;
            x1 = tmp;
        }

        for (; x0 <= x1; x0++)
        {
            twr_gfx_draw_pixel(self, x0, y0, color);
        }

        return;
    }
    else if (x0 == x1)
    {
        if (y0 > y1)
        {
            tmp = y0;
            y0 = y1;
            y1 = tmp;
        }

        for (; y0 <= y1; y0++)
        {
            twr_gfx_draw_pixel(self, x0, y0, color);
        }

        return;
    }

    int16_t step = abs(y1 - y0) > abs(x1 - x0);

    if (step)
    {
        tmp = x0;
        x0 = y0;
        y0 = tmp;

        tmp = x1;
        x1 = y1;
        y1 = tmp;
    }

    if (x0 > x1)
    {
        tmp = x0;
        x0 = x1;
        x1 = tmp;

        tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    ystep = y0 < y1 ? 1 : -1;

    for (; x0 <= x1; x0++)
    {
        if (step)
        {
            twr_gfx_draw_pixel(self, y0, x0, color);
        }
        else
        {
            twr_gfx_draw_pixel(self, x0, y0, color);
        }

        err -= dy;

        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}

void twr_gfx_draw_rectangle(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color)
{
    twr_gfx_draw_line(self, x0, y0, x0, y1, color);
    twr_gfx_draw_line(self, x0, y1, x1, y1, color);
    twr_gfx_draw_line(self, x1, y0, x1, y1, color);
    twr_gfx_draw_line(self, x1, y0, x0, y0, color);
}

void twr_gfx_draw_fill_rectangle(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color)
{
    int y;
    for (; x0 <= x1; x0++)
    {
        for (y = y0; y <= y1; y++)
        {
            twr_gfx_draw_pixel(self, x0, y, color);
        }
    }
}

void twr_gfx_draw_fill_rectangle_dithering(twr_gfx_t *self, int x0, int y0, int x1, int y1, uint32_t color)
{
    int y;
    for (; x0 <= x1; x0++)
    {
        for (y = y0; y <= y1; y++)
        {
            uint8_t dx = x0 % 4;
            uint8_t dy = y % 4;
            uint32_t d_color = color & (1 << (dx + 4*dy));
            twr_gfx_draw_pixel(self, x0, y, d_color);
        }
    }
}

void twr_gfx_draw_circle(twr_gfx_t *self, int x0, int y0, int radius, uint32_t color)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {

        twr_gfx_draw_pixel(self, x0 - y, y0 + x, color);
        twr_gfx_draw_pixel(self, x0 - x, y0 + y, color);
        twr_gfx_draw_pixel(self, x0 - x, y0 - y, color);
        twr_gfx_draw_pixel(self, x0 - y, y0 - x, color);
        twr_gfx_draw_pixel(self, x0 + y, y0 - x, color);
        twr_gfx_draw_pixel(self, x0 + x, y0 - y, color);
        twr_gfx_draw_pixel(self, x0 + x, y0 + y, color);
        twr_gfx_draw_pixel(self, x0 + y, y0 + x, color);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }
}

void twr_gfx_draw_fill_circle(twr_gfx_t *self, int x0, int y0, int radius, uint32_t color)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        twr_gfx_draw_line(self, x0 - y, y0 - x, x0 + y, y0 - x, color);
        twr_gfx_draw_line(self, x0 - x, y0 - y, x0 + x, y0 - y, color);
        twr_gfx_draw_line(self, x0 - x, y0 + y, x0 + x, y0 + y, color);
        twr_gfx_draw_line(self, x0 - y, y0 + x, x0 + y, y0 + x, color);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }
}

void twr_gfx_draw_round_corner(twr_gfx_t *self, int x0, int y0, int radius, twr_gfx_round_corner_t corner, uint32_t color)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        if (corner & TWR_GFX_ROUND_CORNER_RIGHT_TOP)
        {
            twr_gfx_draw_pixel(self, x0 + y, y0 - x, color);
            twr_gfx_draw_pixel(self, x0 + x, y0 - y, color);
        }

        if (corner & TWR_GFX_ROUND_CORNER_RIGHT_BOTTOM)
        {
            twr_gfx_draw_pixel(self, x0 + x, y0 + y, color);
            twr_gfx_draw_pixel(self, x0 + y, y0 + x, color);
        }

        if (corner & TWR_GFX_ROUND_CORNER_LEFT_BOTTOM)
        {
            twr_gfx_draw_pixel(self, x0 - y, y0 + x, color);
            twr_gfx_draw_pixel(self, x0 - x, y0 + y, color);
        }

        if (corner & TWR_GFX_ROUND_CORNER_LEFT_TOP)
        {
            twr_gfx_draw_pixel(self, x0 - x, y0 - y, color);
            twr_gfx_draw_pixel(self, x0 - y, y0 - x, color);
        }

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }
}

void twr_gfx_draw_fill_round_corner(twr_gfx_t *self, int x0, int y0, int radius, twr_gfx_round_corner_t corner, uint32_t color)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        if (corner & TWR_GFX_ROUND_CORNER_RIGHT_TOP)
        {
            twr_gfx_draw_line(self, x0, y0 - x, x0 + y, y0 - x, color);
            twr_gfx_draw_line(self, x0, y0 - y, x0 + x, y0 - y, color);
        }

        if (corner & TWR_GFX_ROUND_CORNER_RIGHT_BOTTOM)
        {
            twr_gfx_draw_line(self, x0, y0 + y, x0 + x, y0 + y, color);
            twr_gfx_draw_line(self, x0, y0 + x, x0 + y, y0 + x, color);
        }

        if (corner & TWR_GFX_ROUND_CORNER_LEFT_BOTTOM)
        {
            twr_gfx_draw_line(self, x0 - y, y0 + x, x0, y0 + x, color);
            twr_gfx_draw_line(self, x0 - x, y0 + y, x0, y0 + y, color);
        }

        if (corner & TWR_GFX_ROUND_CORNER_LEFT_TOP)
        {
            twr_gfx_draw_line(self, x0 - x, y0 - y, x0, y0 - y, color);
            twr_gfx_draw_line(self, x0 - y, y0 - x, x0, y0 - x, color);
        }

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0)
        {
            x--;
            dx += 2;
            err += (-radius << 1) + dx;
        }
    }
}

bool twr_gfx_update(twr_gfx_t *self)
{
    return self->_driver->update(self->_display);
}
