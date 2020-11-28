#include <hio_ramp.h>

static float _hio_ramp_interpolate(hio_tick_t x, hio_tick_t x_min, hio_tick_t x_max, float y_min, float y_max);

void hio_ramp_init(hio_ramp_t *self, float start, float stop, hio_tick_t duration)
{
    memset(self, 0, sizeof(*self));

    self->_start = start;
    self->_stop = stop;
    self->_now = start;

    self->_duration = duration;
}

void hio_ramp_start(hio_ramp_t *self)
{
    self->_tick_start = hio_tick_get();
    self->_tick_end = self->_tick_start + self->_duration;

    self->_active = true;
}

float hio_ramp_get(hio_ramp_t *self)
{
    if (hio_tick_get() >= self->_tick_end)
    {
        self->_active = false;
    }

    if (!self->_active)
    {
        return self->_stop;
    }

    return _hio_ramp_interpolate(hio_tick_get(), self->_tick_start, self->_tick_end, self->_start, self->_stop);
}

static float _hio_ramp_interpolate(hio_tick_t x, hio_tick_t x_min, hio_tick_t x_max, float y_min, float y_max)
{
    if (x < x_min) { x = x_min; }
    if (x > x_max) { x = x_max; }

    return (float) (x - x_min) * (y_max - y_min) / (float) (x_max - x_min) + y_min;
}
