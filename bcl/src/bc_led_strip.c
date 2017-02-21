#include <bc_led_strip.h>

static void _bc_led_strip_task(void *param);
uint32_t _bc_led_strip_wheel(int position);

void bc_led_strip_init(bc_led_strip_t *self, const bc_led_strip_driver_t *driver, const bc_led_strip_buffer_t *buffer)
{
    memset(self, 0x00, sizeof(bc_led_strip_t));
    self->_buffer = buffer;
    self->_driver = driver;
    self->_task_id = bc_scheduler_register(_bc_led_strip_task, self, BC_TICK_INFINITY);

    self->_driver->init(self->_buffer);
}

void bc_led_strip_set_event_handler(bc_led_strip_t *self, void (*event_handler)(bc_led_strip_t *, bc_led_strip_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

int bc_led_strip_get_pixel_count(bc_led_strip_t *self)
{
    return self->_buffer->count;
}

bc_led_strip_type_t bc_led_strip_get_strip_type(bc_led_strip_t *self)
{
    return self->_buffer->type;
}

void bc_led_strip_set_pixel(bc_led_strip_t *self, int position, uint32_t color)
{
    self->_driver->set_pixel(position, color);
}

void bc_led_strip_set_pixel_rgbw(bc_led_strip_t *self, int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    self->_driver->set_pixel_rgbw(position, r, g, b, w);
}

bool bc_led_strip_set_rgbw_framebuffer(bc_led_strip_t *self, const uint8_t *framebuffer, size_t length)
{
    if (length > (size_t) (self->_buffer->type * self->_buffer->count))
    {
        return false;
    }

    int position = 0;

    if (self->_buffer->type == BC_LED_STRIP_TYPE_RGBW)
    {
        for (int i = 0; i < length; i += self->_buffer->type)
        {
            self->_driver->set_pixel_rgbw(position++, framebuffer[i], framebuffer[i + 1], framebuffer[i + 2], framebuffer[i + 3]);
        }
    }
    else
    {
        for (int i = 0; i < length; i += self->_buffer->type)
        {
            self->_driver->set_pixel_rgbw(position++, framebuffer[i], framebuffer[i + 1], framebuffer[i + 2], 0);
        }
    }

    return true;
}

void bc_led_strip_fill(bc_led_strip_t *self, uint32_t color)
{
    for (size_t i = 0; i < self->_buffer->count; i++)
    {
        self->_driver->set_pixel(i, color);
    }
}

bool bc_led_strip_write(bc_led_strip_t *self)
{
    return self->_driver->write();
}

void bc_led_strip_test(bc_led_strip_t *self)
{
    bc_scheduler_plan_absolute(self->_task_id, BC_TICK_INFINITY);

    self->_effect.led = 0;
    self->_effect.round = 0;
    self->_effect.wait = 10;
    self->_effect.name = BC_LED_STRIP_EFFECT_TEST;

    bc_led_strip_fill(self, 0x00000000);

    bc_scheduler_plan_now(self->_task_id);
}

void bc_led_strip_effect_rainbow_cycle(bc_led_strip_t *self, bc_tick_t wait)
{
    bc_scheduler_plan_absolute(self->_task_id, BC_TICK_INFINITY);

    self->_effect.round = 0;
    self->_effect.wait = wait;
    self->_effect.name = BC_LED_STRIP_EFFECT_RAINBOW_CYCLE;

    bc_scheduler_plan_now(self->_task_id);
}

void bc_led_strip_effect_color_wipe(bc_led_strip_t *self, uint32_t color, bc_tick_t wait)
{
    bc_scheduler_plan_absolute(self->_task_id, BC_TICK_INFINITY);

    self->_effect.led = 0;
    self->_effect.wait = wait;
    self->_effect.color = color;
    self->_effect.name = BC_LED_STRIP_EFFECT_COLOR_WIPE;

    bc_scheduler_plan_now(self->_task_id);
}

static void _bc_led_strip_task(void *param)
{
    bc_led_strip_t *self = (bc_led_strip_t *)param;

    if (self->_effect.name == BC_LED_STRIP_EFFECT_NONE)
    {
        bc_scheduler_plan_current_absolute(BC_TICK_INFINITY);

        if (self->_event_handler != NULL)
        {
            self->_event_handler(self, BC_LED_STRIP_EVENT_EFFECT_DONE, self->_event_param);
        }

        return;
    }

    if (self->_effect.name == BC_LED_STRIP_EFFECT_TEST)
    {
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
            if (self->_buffer->type == BC_LED_STRIP_TYPE_RGBW)
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

            if (self->_effect.round == 5)
            {
                self->_effect.name = BC_LED_STRIP_EFFECT_NONE;
            }
        }
    }
    else if (self->_effect.name == BC_LED_STRIP_EFFECT_RAINBOW_CYCLE)
    {

        for(int i = 0; i< self->_buffer->count; i++) {
            self->_driver->set_pixel(i, _bc_led_strip_wheel(((i * 256 / self->_buffer->count) + self->_effect.round) & 255));
        }

        self->_effect.round++;
    }
    else if (self->_effect.name == BC_LED_STRIP_EFFECT_COLOR_WIPE)
    {

        self->_driver->set_pixel(self->_effect.led++, self->_effect.color);

        if (self->_effect.led == self->_buffer->count)
        {
            self->_effect.name = BC_LED_STRIP_EFFECT_NONE;
        }

    }

    self->_driver->write();


    bc_scheduler_plan_current_relative(self->_effect.wait);

}

uint32_t _bc_led_strip_wheel(int position) {
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


