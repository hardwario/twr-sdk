#ifndef _BC_QUEUE_H
#define _BC_QUEUE_H

#include <bc_common.h>

//! @addtogroup bc_queue bc_queue
//! @brief Queue handling functions
//! @{

//! @cond

typedef struct
{
    void *_buffer;
    size_t _size;
    size_t _length;

} bc_queue_t;

//! @endcond

//! @brief Initialize queue
//! @param[in] queue Instance
//! @param[in] buffer Buffer to store the queue
//! @param[in] size Buffer size

void bc_queue_init(bc_queue_t *queue, void *buffer, size_t size);

//! @brief Put buffer to queue
//! @param[in] queue Instance
//! @param[in] buffer Buffer to be copied to queue
//! @param[in] length Length of buffer
//! @return true On success
//! @return false On failure

bool bc_queue_put(bc_queue_t *queue, const void *buffer, size_t length);

//! @brief Get queue to buffer
//! @param[in] queue Instance
//! @param[in] buffer Buffer to be copied from the queue
//! @param[in] length Length of buffer
//! @return true On success
//! @return false On failure

bool bc_queue_get(bc_queue_t *queue, void *buffer, size_t *length);

//! @}

#endif // _BC_QUEUE_H
