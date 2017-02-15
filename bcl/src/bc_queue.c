#include <bc_queue.h>

void bc_queue_init(bc_queue_t *queue, void *buffer, size_t size)
{
    memset(queue, 0, sizeof(*queue));

    queue->_buffer = buffer;
    queue->_size = size;
}

bool bc_queue_put(bc_queue_t *queue, const void *buffer, size_t length)
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

bool bc_queue_get(bc_queue_t *queue, void *buffer, size_t *length)
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
