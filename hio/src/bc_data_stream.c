#include <hio_data_stream.h>

static int _hio_data_stream_compare_float(const void * a, const void * b);
static int _hio_data_stream_compare_int(const void * a, const void * b);
//
void hio_data_stream_init(hio_data_stream_t *self, int min_number_of_samples, hio_data_stream_buffer_t *buffer)
{
    memset(self, 0, sizeof(*self));
    self->_buffer = buffer;
    self->_counter = 0;
    self->_feed_head = self->_buffer->number_of_samples - 1;
    self->_min_number_of_samples = min_number_of_samples;
}

void hio_data_stream_feed(hio_data_stream_t *self, void *data)
{
    if (data == NULL)
    {
        hio_data_stream_reset(self);
        return;
    }

    if (++self->_feed_head == self->_buffer->number_of_samples)
    {
       self->_feed_head = 0;
    }

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            if (isnan(*(float *) data) || isinf(*(float *) data))
            {
                hio_data_stream_reset(self);

                return;
            }

            *((float *) self->_buffer->feed + self->_feed_head) = *(float *) data;

            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            *((int *) self->_buffer->feed + self->_feed_head) = *(int *) data;

            break;
        }
        default:
        {
            break;
        }
    }

    self->_counter++;
}

void hio_data_stream_reset(hio_data_stream_t *self)
{
    self->_counter = 0;
    self->_feed_head = self->_buffer->number_of_samples - 1;
}

int hio_data_stream_get_counter(hio_data_stream_t *self)
{
    return self->_counter;
}

int hio_data_stream_get_length(hio_data_stream_t *self)
{
    return self->_counter > self->_buffer->number_of_samples ? self->_buffer->number_of_samples : self->_counter;
}

hio_data_stream_type_t hio_data_stream_get_type(hio_data_stream_t *self)
{
    return self->_buffer->type;
}

int hio_data_stream_get_number_of_samples(hio_data_stream_t *self)
{
    return self->_buffer->number_of_samples;
}

bool hio_data_stream_get_average(hio_data_stream_t *self, void *result)
{
    if (self->_counter < self->_min_number_of_samples)
    {
        return false;
    }

    int length = self->_counter > self->_buffer->number_of_samples ? self->_buffer->number_of_samples : self->_counter;

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            float sum = 0;
            float *buffer = (float *) self->_buffer->feed;


            for (int i = 0; i < length; i ++)
            {
                sum += buffer[i];
            }

            *(float *) result = sum / length;
            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            int64_t sum = 0;
            int *buffer = (int *) self->_buffer->feed;

            for (int i = 0; i < length; i ++)
            {
                sum += buffer[i];
            }

            *(int *) result = sum / length;
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool hio_data_stream_get_median(hio_data_stream_t *self, void *result)
{
    if (self->_counter < self->_min_number_of_samples)
    {
        return false;
    }

    int length = self->_counter > self->_buffer->number_of_samples ? self->_buffer->number_of_samples : self->_counter;

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            memcpy(self->_buffer->sort, self->_buffer->feed, length * sizeof(float));
            qsort(self->_buffer->sort, length, sizeof(float), _hio_data_stream_compare_float);

            float *buffer = (float *) self->_buffer->sort;

            if (length % 2 == 0)
            {
                *(float *) result = (buffer[(length - 2) / 2] + buffer[length / 2]) / 2;
            }
            else
            {
                *(float *) result = buffer[(length - 1) / 2];
            }
            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            memcpy(self->_buffer->sort, self->_buffer->feed, length * sizeof(int));
            qsort(self->_buffer->sort, length, sizeof(int), _hio_data_stream_compare_int);

            int *buffer = (int *) self->_buffer->sort;

            if (length % 2 == 0)
            {
                *(int *) result = (buffer[(length - 2) / 2] + buffer[length / 2]) / 2;
            }
            else
            {
                *(int *) result = buffer[(length - 1) / 2];
            }
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool hio_data_stream_get_first(hio_data_stream_t *self, void *result)
{
    if (self->_counter == 0)
    {
        return false;
    }

    int position = self->_counter < self->_buffer->number_of_samples ? 0 : self->_feed_head + 1;

    if (position == self->_buffer->number_of_samples)
    {
        position = 0;
    }

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            *(float *) result = *((float *) self->_buffer->feed + position);
            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            *(int *) result = *((int *) self->_buffer->feed + position);
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool hio_data_stream_get_last(hio_data_stream_t *self, void *result)
{
    if (self->_counter == 0)
    {
        return false;
    }

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            *(float *) result = *((float *) self->_buffer->feed + self->_feed_head);
            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            *(int *) result = *((int *) self->_buffer->feed + self->_feed_head);
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool hio_data_stream_get_nth(hio_data_stream_t *self, int n, void *result)
{
    int position;

    if (n < 0)
    {
        if (self->_counter + n < 0)
        {
            return false;
        }

        if (self->_buffer->number_of_samples + n < 0)
        {
            return false;
        }

        position = self->_counter < self->_buffer->number_of_samples ? self->_counter : self->_feed_head + 1 + self->_buffer->number_of_samples;
    }
    else
    {
        if (self->_counter < n)
        {
            return false;
        }

        if (self->_buffer->number_of_samples > n)
        {
            return false;
        }

        position = self->_counter < self->_buffer->number_of_samples ? 0 : self->_feed_head + 1;
    }

    position = (position + n) % self->_buffer->number_of_samples;

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            *(float *) result = *((float *) self->_buffer->feed + position);
            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            *(int *) result = *((int *) self->_buffer->feed + position);
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool hio_data_stream_get_max(hio_data_stream_t *self, void *result)
{
    if (self->_counter < self->_min_number_of_samples)
    {
        return false;
    }

    int length = hio_data_stream_get_length(self);

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            float *buffer = (float *) self->_buffer->feed;

            float max = buffer[0];

            for (int i = 1; i < length; i ++)
            {
                if (buffer[i] > max)
                {
                    max = buffer[i];
                }
            }

            *(float *) result = max;

            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            int *buffer = (int *) self->_buffer->feed;

            int max = buffer[0];

            for (int i = 1; i < length; i ++)
            {
                if (buffer[i] > max)
                {
                    max = buffer[i];
                }
            }

            *(int *) result = max;

            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool hio_data_stream_get_min(hio_data_stream_t *self, void *result)
{
    if (self->_counter < self->_min_number_of_samples)
    {
        return false;
    }

    int length = hio_data_stream_get_length(self);

    switch (self->_buffer->type)
    {
        case HIO_DATA_STREAM_TYPE_FLOAT:
        {
            float *buffer = (float *) self->_buffer->feed;

            float min = buffer[0];

            for (int i = 1; i < length; i ++)
            {
                if (buffer[i] < min)
                {
                    min = buffer[i];
                }
            }

            *(float *) result = min;

            break;
        }
        case HIO_DATA_STREAM_TYPE_INT:
        {
            int *buffer = (int *) self->_buffer->feed;

            int min = buffer[0];

            for (int i = 1; i < length; i ++)
            {
                if (buffer[i] < min)
                {
                    min = buffer[i];
                }
            }

            *(int *) result = min;

            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

static int _hio_data_stream_compare_float(const void *a, const void *b)
{
    return *(float *) a - *(float *) b;
}

static int _hio_data_stream_compare_int(const void *a, const void *b)
{
    return *(int *) a - *(int *) b;
}
