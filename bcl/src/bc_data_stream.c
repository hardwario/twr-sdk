#include <bc_data_stream.h>

static int _bc_data_stream_compare_float(const void * a, const void * b);
static int _bc_data_stream_compare_int(const void * a, const void * b);

void bc_data_stream_init(bc_data_stream_t *self, bc_data_stream_type_t type, void *buffer, size_t buffer_size)
{
    memset(self, 0, sizeof(*self));

    self->_buffer = buffer;
    self->_buffer_size = buffer_size;
    self->_type = type;
    self->_fifo_head = buffer;
    self->_counter = 0;
    self->_temp_head = (uint8_t *)buffer + (buffer_size / 2);
    self->_number_of_samples = buffer_size / 2;

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            self->_number_of_samples /= sizeof(float);
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            self->_number_of_samples /= sizeof(int);
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
            if ((*(float *)data == NAN) || (*(float *)data == INFINITY))
            {
                bc_data_stream_reset(self);
                return;
            }
            *(float *) self->_fifo_head = *(float *) data;
            self->_fifo_head = (uint8_t *)self->_fifo_head + sizeof(float);
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            *(int *) self->_fifo_head = *(int *) data;
            self->_fifo_head = (uint8_t *)self->_fifo_head + sizeof(int);
            break;
        }
        default:
        {
            break;
        }
    }

    if (self->_fifo_head == self->_temp_head)
    {
        self->_fifo_head = self->_buffer;
    }

    ++self->_counter;
}

void bc_data_stream_reset(bc_data_stream_t *self)
{
    self->_fifo_head = self->_buffer;
    self->_counter = 0;
}

bool bc_data_stream_get_average(bc_data_stream_t *self, void *result)
{
    if (self->_counter == 0)
    {
        return false;
    }

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            float sum = 0;
            float *buffer = (float *) self->_buffer;

            int length = self->_counter > self->_number_of_samples ? self->_number_of_samples : self->_counter;

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
            int *buffer = (int *) self->_buffer;

            int length = self->_counter > self->_number_of_samples ? self->_number_of_samples : self->_counter;

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
    if (self->_counter == 0)
    {
        return false;
    }

    size_t length = self->_counter > self->_number_of_samples ? self->_number_of_samples : self->_counter;

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            memcpy(self->_temp_head, self->_buffer, length * sizeof(float));
            qsort(self->_temp_head, length, sizeof(float), _bc_data_stream_compare_float);

            float *buffer = (float *) self->_temp_head;

            if (length % 2 == 0)
            {
                *(float *) result = (buffer[(length - 2) / 2] + buffer[length / 2]) / 2;
            }else
            {
                *(float *) result = buffer[(length - 1) / 2];
            }
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            memcpy(self->_temp_head, self->_buffer, length * sizeof(int));
            qsort(self->_temp_head, length, sizeof(int), _bc_data_stream_compare_int);

            int *buffer = (int *) self->_temp_head;

            if (length % 2 == 0)
            {
                *(int *) result = (buffer[(length - 2) / 2] + buffer[length / 2]) / 2;
            }else
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

    void *pointer = self->_counter > self->_number_of_samples ? self->_fifo_head : self->_buffer;

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

    if (self->_fifo_head == self->_buffer)
    {
        pointer = self->_temp_head;
    }

    switch (self->_type)
    {
        case BC_DATA_STREAM_TYPE_FLOAT:
        {
            pointer = (uint8_t *)pointer - sizeof(float);
            *(float *) result = *(float *) pointer;
            break;
        }
        case BC_DATA_STREAM_TYPE_INT:
        {
            pointer = (uint8_t *)pointer - sizeof(int);
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

static int _bc_data_stream_compare_float(const void * a, const void * b)
{
  return *(float*) a - *(float*) b;
}

static int _bc_data_stream_compare_int(const void * a, const void * b)
{
  return *(int*) a - *(int*) b;
}
