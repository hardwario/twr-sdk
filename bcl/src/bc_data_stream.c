#include <bc_data_stream.h>

static int _bc_data_stream_compare_float(const void * a, const void * b);
static int _bc_data_stream_compare_int(const void * a, const void * b);
//
void bc_data_stream_init(bc_data_stream_t *self, bc_data_stream_type_t type, int min_number_of_samples, bc_data_stream_buffer_t *buffer)
{
    memset(self, 0, sizeof(*self));
    self->_buffer = buffer;
    self->_type = type;
    self->_counter = 0;
    self->_min_number_of_samples = min_number_of_samples;

    self->_fifo_head = buffer->feed;

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            self->_fifo_end = (float *) self->_fifo_head + self->_buffer->number_of_samples;
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            self->_fifo_end = (int *) self->_fifo_head + self->_buffer->number_of_samples;
            break;
        }
        default:
        {
            break;
        }
    }
}

void bc_data_stream_feed(bc_data_stream_t *self, void *data)
{
    if (data == NULL)
    {
        bc_data_stream_reset(self);
        return;
    }

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            if ((*(float *) data == NAN) || (*(float *) data == INFINITY))
            {
                bc_data_stream_reset(self);
                return;
            }
            *(float *) self->_fifo_head = *(float *) data;
            self->_fifo_head = (float *) self->_fifo_head + 1;
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            *(int *) self->_fifo_head = *(int *) data;
            self->_fifo_head = (int *) self->_fifo_head + 1;
            break;
        }
        default:
        {
            break;
        }
    }

    if (self->_fifo_head == self->_fifo_end)
    {
        self->_fifo_head = self->_buffer->feed;
    }

    ++self->_counter;
}

void bc_data_stream_reset(bc_data_stream_t *self)
{
    self->_fifo_head = self->_buffer->feed;
    self->_counter = 0;
}

bool bc_data_stream_get_average(bc_data_stream_t *self, void *result)
{
    if (self->_counter < self->_min_number_of_samples)
    {
        return false;
    }

    int length = self->_counter > self->_buffer->number_of_samples ? self->_buffer->number_of_samples : self->_counter;

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
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
        case BC_DATA_STREAM_TYPE_INT:
        {
            int sum = 0;
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

bool bc_data_stream_get_median(bc_data_stream_t *self, void *result)
{
    if (self->_counter < self->_min_number_of_samples)
    {
        return false;
    }

     size_t length = self->_counter > self->_buffer->number_of_samples ? self->_buffer->number_of_samples : self->_counter;

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            memcpy(self->_buffer->sort, self->_buffer->feed, length * sizeof(float));
            qsort(self->_buffer->sort, length, sizeof(float), _bc_data_stream_compare_float);

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
        case BC_DATA_STREAM_TYPE_INT:
        {
            memcpy(self->_buffer->sort, self->_buffer->feed, length * sizeof(int));
            qsort(self->_buffer->sort, length, sizeof(int), _bc_data_stream_compare_int);

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

bool bc_data_stream_get_first(bc_data_stream_t *self, void *result)
{
    if (self->_counter == 0)
    {
        return false;
    }

    void *pointer = self->_counter > self->_buffer->number_of_samples ? self->_fifo_head : self->_buffer->feed;

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            *(float *) result = *(float *) pointer;
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            *(int *) result = *(int *) pointer;
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

bool bc_data_stream_get_last(bc_data_stream_t *self, void *result)
{
    if (self->_counter == 0)
    {
        return false;
    }

    void *pointer = self->_fifo_head;
    int move = -1;

    if (self->_fifo_head == self->_buffer->feed)
    {
        move = self->_buffer->number_of_samples - 1;
    }

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            pointer = (float *) pointer + move;
            *(float *) result = *(float *) pointer;
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            pointer = (int *) pointer + move;
            *(int *) result = *(int *) pointer;
            break;
        }
        default:
        {
            return false;
        }
    }

    return true;
}

static int _bc_data_stream_compare_float(const void *a, const void *b)
{
    return *(float *) a - *(float *) b;
}

static int _bc_data_stream_compare_int(const void *a, const void *b)
{
    return *(int *) a - *(int *) b;
}
