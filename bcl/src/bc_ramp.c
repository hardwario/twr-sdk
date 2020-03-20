#include <bc_ramp.h>

static float _bc_ramp_interpolate(bc_tick_t x, bc_tick_t x_min, bc_tick_t x_max, float y_min, float y_max);

void bc_ramp_init(bc_ramp_t *self, float start, float stop, bc_tick_t duration)
{
    memset(self, 0, sizeof(*self));

    self->_start = start;
    self->_stop = stop;
    self->_now = start;

    self->_duration = duration;
}

void bc_ramp_start(bc_ramp_t *self)
{
    self->_tick_start = bc_tick_get();
    self->_tick_end = self->_tick_start + self->_duration;

    self->_active = true;
}

float bc_ramp_get(bc_ramp_t *self)
{
    if (bc_tick_get() >= self->_tick_end)
    {
        self->_active = false;
    }

    if (!self->_active)
    {
        return self->_stop;
    }

    return _bc_ramp_interpolate(bc_tick_get(), self->_tick_start, self->_tick_end, self->_start, self->_stop);
}

static float _bc_ramp_interpolate(bc_tick_t x, bc_tick_t x_min, bc_tick_t x_max, float y_min, float y_max)
{
    if (x < x_min) { x = x_min; }
    if (x > x_max) { x = x_max; }

    return (float) (x - x_min) * (y_max - y_min) / (float) (x_max - x_min) + y_min;
}
