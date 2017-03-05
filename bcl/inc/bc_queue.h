#ifndef _BC_QUEUE_H
#define _BC_QUEUE_H

#include <bc_common.h>

typedef struct
{
    void *_buffer;
    size_t _size;
    size_t _length;

} bc_queue_t;

void bc_queue_init(bc_queue_t *queue, void *buffer, size_t size);
bool bc_queue_put(bc_queue_t *queue, const void *buffer, size_t length);
bool bc_queue_get(bc_queue_t *queue, void *buffer, size_t *length);

#endif // _BC_QUEUE_H
