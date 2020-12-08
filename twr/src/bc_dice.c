#include <twr_dice.h>

#define _TWR_DICE_THRESHOLD 0.4f

const int8_t _twr_dice_vectors[][3] =
{
    [TWR_DICE_FACE_UNKNOWN] = { 0, 0, 0 },

    [TWR_DICE_FACE_1] = { 0, 0, 1 },
    [TWR_DICE_FACE_2] = { 1, 0, 0 },
    [TWR_DICE_FACE_3] = { 0, 1, 0 },
    [TWR_DICE_FACE_4] = { 0, -1, 0 },
    [TWR_DICE_FACE_5] = { -1, 0, 0 },
    [TWR_DICE_FACE_6] = { 0, 0, -1 }
};

void twr_dice_init(twr_dice_t *self, twr_dice_face_t start)
{
    memset(self, 0, sizeof(*self));

    self->_face = start;

    self->_threshold = _TWR_DICE_THRESHOLD;
}

void twr_dice_set_threshold(twr_dice_t *self, float threshold)
{
    self->_threshold = threshold;
}

void twr_dice_feed_vectors(twr_dice_t *self, float x_axis, float y_axis, float z_axis)
{
    int8_t vector_x = _twr_dice_vectors[self->_face][0];
    int8_t vector_y = _twr_dice_vectors[self->_face][1];
    int8_t vector_z = _twr_dice_vectors[self->_face][2];

    bool update = false;

    if ((vector_x == 0 && (x_axis < -self->_threshold || x_axis > self->_threshold)) ||
        (vector_x == 1 && (x_axis < 1.f - self->_threshold)) ||
        (vector_x == -1 && (x_axis > -1.f + self->_threshold)))
    {
        update = true;
    }

    if ((vector_y == 0 && (y_axis < -self->_threshold || y_axis > self->_threshold)) ||
        (vector_y == 1 && (y_axis < 1.f - self->_threshold)) ||
        (vector_y == -1 && (y_axis > -1.f + self->_threshold)))
    {
        update = true;
    }

    if ((vector_z == 0 && (z_axis < -self->_threshold || z_axis > self->_threshold)) ||
        (vector_z == 1 && (z_axis < 1.f - self->_threshold)) ||
        (vector_z == -1 && (z_axis > -1.f + self->_threshold)))
    {
        update = true;
    }

    if (update)
    {
        for (size_t i = 1; i <= 6; i++)
        {
            float delta_x = _twr_dice_vectors[i][0] - x_axis;
            float delta_y = _twr_dice_vectors[i][1] - y_axis;
            float delta_z = _twr_dice_vectors[i][2] - z_axis;

            if (delta_x < 0.f) { delta_x = -delta_x; }
            if (delta_y < 0.f) { delta_y = -delta_y; }
            if (delta_z < 0.f) { delta_z = -delta_z; }

            if (delta_x < 1.f - self->_threshold &&
                delta_y < 1.f - self->_threshold &&
                delta_z < 1.f - self->_threshold)
            {
                self->_face = i;
            }
        }
    }
}

twr_dice_face_t twr_dice_get_face(twr_dice_t *self)
{
    return self->_face;
}
