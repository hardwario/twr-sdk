#ifndef _HIO_QUEUE_H
#define _HIO_QUEUE_H

#include <hio_common.h>

//! @addtogroup hio_queue hio_queue
//! @brief Queue handling functions
//! @{

//! @cond

typedef struct
{
    void *_buffer;
    size_t _size;
    size_t _length;

} hio_queue_t;

//! @endcond

//! @brief Initialize queue
//! @param[in] queue Instance
//! @param[in] buffer Buffer to store the queue
//! @param[in] size Buffer size

void hio_queue_init(hio_queue_t *queue, void *buffer, size_t size);

//! @brief Put buffer to queue
//! @param[in] queue Instance
//! @param[in] buffer Buffer to be copied to queue
//! @param[in] length Length of buffer
//! @return true On success
//! @return false On failure

bool hio_queue_put(hio_queue_t *queue, const void *buffer, size_t length);

//! @brief Get queue to buffer
//! @param[in] queue Instance
//! @param[in] buffer Buffer to be copied from the queue
//! @param[in] length Length of buffer
//! @return true On success
//! @return false On failure

bool hio_queue_get(hio_queue_t *queue, void *buffer, size_t *length);

//! @}

#endif // _HIO_QUEUE_H
