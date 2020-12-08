#include <twr_ramp.h>

static float _twr_ramp_interpolate(twr_tick_t x, twr_tick_t x_min, twr_tick_t x_max, float y_min, float y_max);

void twr_ramp_init(twr_ramp_t *self, float start, float stop, twr_tick_t duration)
{
    memset(self, 0, sizeof(*self));

    self->_start = start;
    self->_stop = stop;
    self->_now = start;

    self->_duration = duration;
}

void twr_ramp_start(twr_ramp_t *self)
{
    self->_tick_start = twr_tick_get();
    self->_tick_end = self->_tick_start + self->_duration;

    self->_active = true;
}

float twr_ramp_get(twr_ramp_t *self)
{
    if (twr_tick_get() >= self->_tick_end)
    {
        self->_active = false;
    }

    if (!self->_active)
    {
        return self->_stop;
    }

    return _twr_ramp_interpolate(twr_tick_get(), self->_tick_start, self->_tick_end, self->_start, self->_stop);
}

static float _twr_ramp_interpolate(twr_tick_t x, twr_tick_t x_min, twr_tick_t x_max, float y_min, float y_max)
{
    if (x < x_min) { x = x_min; }
    if (x > x_max) { x = x_max; }

    return (float) (x - x_min) * (y_max - y_min) / (float) (x_max - x_min) + y_min;
}
