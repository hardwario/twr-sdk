#include <twr_queue.h>

void twr_queue_init(twr_queue_t *queue, void *buffer, size_t size)
{
    memset(queue, 0, sizeof(*queue));

    queue->_buffer = buffer;
    queue->_size = size;
}

bool twr_queue_put(twr_queue_t *queue, const void *buffer, size_t length)
{
    if (length == 0)
    {
        return true;
    }

    if (sizeof(length) + length > queue->_size - queue->_length)
    {
        return false;
    }

    uint8_t *p = queue->_buffer;

    p += queue->_length;

    memcpy(p, &length, sizeof(length));

    p += sizeof(length);

    queue->_length += sizeof(length) + length;

    if (buffer != NULL)
    {
        memcpy(p, buffer, length);
    }
    else
    {
        memset(p, 0, length);
    }

    return true;
}

bool twr_queue_get(twr_queue_t *queue, void *buffer, size_t *length)
{
    if (queue->_length == 0)
    {
        return false;
    }

    uint8_t *p = queue->_buffer;

    memcpy(length, p, sizeof(*length));

    p += sizeof(*length);

    queue->_length -= sizeof(*length) + *length;

    if (buffer != NULL)
    {
        memcpy(buffer, p, *length);
    }

    memmove(queue->_buffer, p + *length, queue->_length);

    return true;
}

void twr_queue_clear(twr_queue_t *queue)
{
    queue->_length = 0;
}
