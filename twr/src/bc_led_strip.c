#include <twr_led_strip.h>

#define TWR_LED_STRIP_NULL_TASK TWR_SCHEDULER_MAX_TASKS + 1

static uint32_t _twr_led_strip_wheel(int position);
static void _twr_led_strip_get_heat_map_color(float value, float *red, float *green, float *blue);

void twr_led_strip_init(twr_led_strip_t *self, const twr_led_strip_driver_t *driver, const twr_led_strip_buffer_t *buffer)
{
    memset(self, 0x00, sizeof(twr_led_strip_t));
    self->_buffer = buffer;
    self->_driver = driver;
    self->_effect.task_id = TWR_LED_STRIP_NULL_TASK;
    self->_driver->init(self->_buffer);
    self->_brightness = 255;
}

void twr_led_strip_set_event_handler(twr_led_strip_t *self, void (*event_handler)(twr_led_strip_t *, twr_led_strip_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

int twr_led_strip_get_pixel_count(twr_led_strip_t *self)
{
    return self->_buffer->count;
}

twr_led_strip_type_t twr_led_strip_get_strip_type(twr_led_strip_t *self)
{
    return self->_buffer->type;
}

void twr_led_strip_set_pixel(twr_led_strip_t *self, int position, uint32_t color)
{
    if (position < 0 || position >= self->_buffer->count)
    {
        return;
    }

    if (self->_brightness != 255)
    {
        twr_led_strip_set_pixel_rgbw(self, position, color >> 24, color >> 16, color >> 8, color);
    }
    else
    {
        self->_driver->set_pixel(position, color);
    }
}

void twr_led_strip_set_pixel_rgbw(twr_led_strip_t *self, int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    if (position < 0 || position >= self->_buffer->count)
    {
        return;
    }

    if (self->_brightness != 255)
    {
        r = ((uint16_t) r * self->_brightness) >> 8;
        g = ((uint16_t) g * self->_brightness) >> 8;
        b = ((uint16_t) b * self->_brightness) >> 8;
        w = ((uint16_t) w * self->_brightness) >> 8;
    }
    self->_driver->set_pixel_rgbw(position, r, g, b, w);
}

bool twr_led_strip_set_rgbw_framebuffer(twr_led_strip_t *self, uint8_t *framebuffer, size_t length)
{
    if (length > (size_t) (self->_buffer->type * self->_buffer->count))
    {
        return false;
    }

    int position = 0;

    if (self->_buffer->type == TWR_LED_STRIP_TYPE_RGBW)
    {
        for (size_t i = 0; i < length; i += self->_buffer->type)
        {
            self->_driver->set_pixel_rgbw(position++, framebuffer[i], framebuffer[i + 1], framebuffer[i + 2], framebuffer[i + 3]);
        }
    }
    else
    {
        for (size_t i = 0; i < length; i += self->_buffer->type)
        {
            self->_driver->set_pixel_rgbw(position++, framebuffer[i], framebuffer[i + 1], framebuffer[i + 2], 0);
        }
    }

    return true;
}

void twr_led_strip_fill(twr_led_strip_t *self, uint32_t color)
{
    for (int i = 0; i < self->_buffer->count; i++)
    {
        twr_led_strip_set_pixel(self, i, color);
    }
}

bool twr_led_strip_write(twr_led_strip_t *self)
{
    return self->_driver->write();
}

bool twr_led_strip_is_ready(twr_led_strip_t *self)
{
    return self->_driver->is_ready();
}

void twr_led_strip_set_brightness(twr_led_strip_t *self, uint8_t brightness)
{
    self->_brightness = brightness;
}

void twr_led_strip_effect_stop(twr_led_strip_t *self)
{
    if (self->_effect.task_id != TWR_LED_STRIP_NULL_TASK)
    {
        twr_scheduler_unregister(self->_effect.task_id);

        self->_effect.task_id = TWR_LED_STRIP_NULL_TASK;
    }
}

static void _twr_led_strip_effect_done(twr_led_strip_t *self)
{
    twr_led_strip_effect_stop(self);

    if (self->_event_handler != NULL)
    {
        self->_event_handler(self, TWR_LED_STRIP_EVENT_EFFECT_DONE, self->_event_param);
    }
}

static void _twr_led_strip_effect_test_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    uint8_t intensity = 255 * (self->_effect.led + 1) / (self->_buffer->count + 1);

    if (self->_effect.round == 0)
    {
        self->_driver->set_pixel_rgbw(self->_effect.led, intensity, 0, 0, 0);
    }
    else if (self->_effect.round == 1)
    {
        self->_driver->set_pixel_rgbw(self->_effect.led, 0, intensity, 0, 0);
    }
    else if (self->_effect.round == 2)
    {
        self->_driver->set_pixel_rgbw(self->_effect.led, 0, 0, intensity, 0);
    }
    else if (self->_effect.round == 3)
    {
        if (self->_buffer->type == TWR_LED_STRIP_TYPE_RGBW)
        {
            self->_driver->set_pixel_rgbw(self->_effect.led, 0, 0, 0, intensity);
        }
        else
        {
            self->_driver->set_pixel_rgbw(self->_effect.led, intensity, intensity, intensity, 0);
        }
    }
    else
    {
        self->_driver->set_pixel_rgbw(self->_effect.led, 0, 0, 0, 0);
    }

    self->_effect.led++;

    if (self->_effect.led == self->_buffer->count)
    {
        self->_effect.led = 0;

        self->_effect.round++;
    }

    self->_driver->write();

    if (self->_effect.round == 5)
    {
        _twr_led_strip_effect_done(self);
        return;
    }

    twr_scheduler_plan_current_relative(self->_effect.wait);
}

void twr_led_strip_effect_test(twr_led_strip_t *self)
{
    twr_led_strip_effect_stop(self);

    self->_effect.led = 0;
    self->_effect.round = 0;
    self->_effect.wait = 2000 / self->_buffer->count;

    twr_led_strip_fill(self, 0x00000000);

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_test_task, self, 0);
}

static void _twr_led_strip_effect_rainbow_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    for(int i = 0; i< self->_buffer->count; i++) {
        twr_led_strip_set_pixel(self, i, _twr_led_strip_wheel((i + self->_effect.round) & 255));
    }

    self->_effect.round++;

    self->_driver->write();

    twr_scheduler_plan_current_relative(self->_effect.wait);
}

void twr_led_strip_effect_rainbow(twr_led_strip_t *self, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.round = 0;
    self->_effect.wait = wait;

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_rainbow_task, self, 0);
}

static void _twr_led_strip_effect_rainbow_cycle_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    for(int i = 0; i< self->_buffer->count; i++) {
        twr_led_strip_set_pixel(self, i, _twr_led_strip_wheel(((i * 256 / self->_buffer->count) + self->_effect.round) & 255));
    }

    self->_effect.round++;

    self->_driver->write();

    twr_scheduler_plan_current_relative(self->_effect.wait);
}

void twr_led_strip_effect_rainbow_cycle(twr_led_strip_t *self, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.round = 0;
    self->_effect.wait = wait;

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_rainbow_cycle_task, self, 0);
}

static void _twr_led_strip_effect_color_wipe_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    twr_led_strip_set_pixel(self, self->_effect.led++, self->_effect.color);

    if (self->_effect.led == self->_buffer->count)
    {
        _twr_led_strip_effect_done(self);
        return;
    }

    self->_driver->write();

    twr_scheduler_plan_current_relative(self->_effect.wait);

}

void twr_led_strip_effect_color_wipe(twr_led_strip_t *self, uint32_t color, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.led = 0;
    self->_effect.wait = wait;
    self->_effect.color = color;

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_color_wipe_task, self, 0);
}

static void _twr_led_strip_effect_theater_chase_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    for (int i = self->_effect.led; i < self->_buffer->count; i += 3) {
        self->_driver->set_pixel(i, 0);    //turn every third pixel off
    }

    self->_effect.led++;

    if (self->_effect.led == 3)
    {
        self->_effect.led = 0;
    }

    for (int i = self->_effect.led ; i < self->_buffer->count; i += 3) {
        twr_led_strip_set_pixel(self, i, self->_effect.color);    //turn every third pixel on
    }

    self->_driver->write();

    twr_scheduler_plan_current_relative(self->_effect.wait);

}

void twr_led_strip_effect_theater_chase(twr_led_strip_t *self, uint32_t color, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.led = 0;
    self->_effect.round = 0;
    self->_effect.color = color;
    self->_effect.wait = wait;

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_theater_chase_task, self, 0);
}

static void _twr_led_strip_effect_theater_chase_rainbow_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    for (int i = self->_effect.led; i < self->_buffer->count; i += 3) {
        self->_driver->set_pixel(i, 0);    //turn every third pixel off
    }

    self->_effect.led++;

    if (self->_effect.led == 3)
    {
        self->_effect.led = 0;
    }

    for (int i = self->_effect.led; i < self->_buffer->count; i += 3) {
        twr_led_strip_set_pixel(self, i, _twr_led_strip_wheel((i + self->_effect.round) % 255) );    //turn every third pixel on
    }

    self->_driver->write();

    self->_effect.round++;

    twr_scheduler_plan_current_relative(self->_effect.wait);

}

void twr_led_strip_effect_theater_chase_rainbow(twr_led_strip_t *self, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.led = 0;
    self->_effect.round = 0;
    self->_effect.wait = wait;

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_theater_chase_rainbow_task, self, 0);
}

static void twr_led_strip_effect_stroboscope_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    uint32_t color = (self->_effect.round & 1) == 0 ? self->_effect.color : 0;

    twr_led_strip_fill(self, color);

    twr_led_strip_write(self);

    self->_effect.round++;

    twr_scheduler_plan_current_relative(self->_effect.wait);
}

void twr_led_strip_effect_stroboscope(twr_led_strip_t *self, uint32_t color, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.wait = wait;

    self->_effect.round = 0;

    self->_effect.color = color;

    self->_effect.task_id = twr_scheduler_register(twr_led_strip_effect_stroboscope_task, self, 0);
}

void _twr_led_strip_effect_icicle_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    const int length = 10;

    for (int i = self->_effect.led; (i < self->_effect.led + length) && i < self->_buffer->count; i++) {

        if (i < 0)
        {
            continue;
        }

        twr_led_strip_set_pixel(self, i, 0x00);
    }

    self->_effect.led++;

    if (self->_effect.led == self->_buffer->count)
    {
        self->_effect.led = -length;
    }


    uint8_t r, g, b, w, dr, dg, db, dw;

    dr = ((uint8_t) (self->_effect.color >> 24)) / length;
    dg = ((uint8_t) (self->_effect.color >> 16)) / length;
    db = ((uint8_t) (self->_effect.color >> 8)) / length;
    dw = ((uint8_t) (self->_effect.color)) / length;

    r = dr;
    g = dg;
    b = db;
    w = dw;

    for (int i = self->_effect.led; (i < self->_effect.led + length) && i < self->_buffer->count; i++) {

        if (i < 0)
        {
            continue;
        }

        twr_led_strip_set_pixel_rgbw(self, i, r, g, b, w); // 0x20000000

        r += dr;
        g += dg;
        b += db;
        w += dw;
    }

    twr_led_strip_write(self);

    twr_scheduler_plan_current_relative(self->_effect.wait);
}

void twr_led_strip_effect_icicle(twr_led_strip_t *self, uint32_t color, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.color = color;

    self->_effect.wait = wait;

    self->_effect.led = -10;

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_icicle_task, self, 0);
}

static void _twr_led_strip_effect_pulse_color_task(void *param)
{
    twr_led_strip_t *self = (twr_led_strip_t *)param;

    if (!self->_driver->is_ready())
    {
        twr_scheduler_plan_current_now();

        return;
    }

    uint8_t r = self->_effect.color >> 24;
    uint8_t g = self->_effect.color >> 16;
    uint8_t b = self->_effect.color >> 8;
    uint8_t w = self->_effect.color;

    uint8_t brightness = (abs(19 - self->_effect.round) + 1) * (255 / 20);

    if (++self->_effect.round == 38)
    {
        self->_effect.round = 0;
    }

    r = ((uint16_t) r * brightness) >> 8;
    g = ((uint16_t) g * brightness) >> 8;
    b = ((uint16_t) b * brightness) >> 8;
    w = ((uint16_t) w * brightness) >> 8;

    for (int i = 0; i < self->_buffer->count; i++)
    {
        twr_led_strip_set_pixel_rgbw(self, i, r, g, b, w);
    }

    self->_driver->write();

    twr_scheduler_plan_current_relative(self->_effect.wait);
}

void twr_led_strip_effect_pulse_color(twr_led_strip_t *self, uint32_t color, twr_tick_t wait)
{
    twr_led_strip_effect_stop(self);

    self->_effect.round = 0;
    self->_effect.wait = wait;
    self->_effect.color = color;

    self->_effect.task_id = twr_scheduler_register(_twr_led_strip_effect_pulse_color_task, self, 0);
}

void twr_led_strip_thermometer(twr_led_strip_t *self, float temperature, float min, float max, uint8_t white_dots, float set_point, uint32_t color)
{
    temperature -= min;

    int max_i = ((float)self->_buffer->count / (float)(fabs(max) + fabs(min))) * temperature;

    if (max_i > self->_buffer->count)
    {
        max_i = self->_buffer->count;
    }

    if (max_i < 0)
    {
        max_i = 0;
    }

    float red;
    float green;
    float blue;

    for (int i = 0; i < max_i; i++)
    {
        _twr_led_strip_get_heat_map_color((float)i / self->_buffer->count, &red, &green, &blue);

        self->_driver->set_pixel_rgbw(i, self->_brightness * red, self->_brightness * green, self->_brightness * blue, 0);
    }

    if (self->_buffer->type == TWR_LED_STRIP_TYPE_RGBW)
    {
        for (int i = max_i; i < self->_buffer->count; i++)
        {
            self->_driver->set_pixel_rgbw(i, 0, 0, 0, white_dots);
        }
    }
    else
    {
        for (int i = max_i; i < self->_buffer->count; i++)
        {
            self->_driver->set_pixel_rgbw(i, white_dots, white_dots, white_dots, 0);
        }
    }

    if ((min < set_point) && (max > set_point))
    {
        set_point -= min;

        int color_i = ((float)self->_buffer->count / (float)(fabs(max) + fabs(min))) * set_point;

        self->_driver->set_pixel(color_i, color);
    }

    twr_led_strip_write(self);
}

static uint32_t _twr_led_strip_wheel(int position) {
    if(position < 85)
    {
      return ((position * 3) << 24) | ((255 - position * 3) << 16);
    }
    else if (position < 170)
    {
      position -= 85;
      return ((255 - position * 3) << 24) | ((position * 3) << 8);
    }
    else
    {
      position -= 170;
      return ((position * 3) << 16) | ((255 - position * 3) << 8);
    }
}

static void _twr_led_strip_get_heat_map_color(float value, float *red, float *green, float *blue)
{
    const int NUM_COLORS = 4;
    const float color[4][3] = { {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0} };

    int idx1;        // Our desired color will be between these two indexes in "color".
    int idx2;
    float fractBetween = 0; // Fraction between "idx1" and "idx2" where our value is.

    if(value <= 0)
    {
        idx1 = idx2 = 0;
    }
    else if(value >= 1)
    {
        idx1 = idx2 = NUM_COLORS - 1;
    }
    else
    {
        value = value * (NUM_COLORS - 1);      // Will multiply value by 3.
        idx1  = floor(value);                  // Our desired color will be after this index.
        idx2  = idx1 + 1;                      // ... and before this index (inclusive).
        fractBetween = value - (float)idx1;    // Distance between the two indexes (0-1).
    }

    *red   = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
    *green = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
    *blue  = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];
}
