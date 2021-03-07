#ifndef _TWR_QUEUE_H
#define _TWR_QUEUE_H

#include <twr_common.h>

//! @addtogroup twr_queue twr_queue
//! @brief Queue handling functions
//! @{

//! @cond

typedef struct
{
    void *_buffer;
    size_t _size;
    size_t _length;

} twr_queue_t;

//! @endcond

//! @brief Initialize queue
//! @param[in] queue Instance
//! @param[in] buffer Buffer to store the queue
//! @param[in] size Buffer size

void twr_queue_init(twr_queue_t *queue, void *buffer, size_t size);

//! @brief Put buffer to queue
//! @param[in] queue Instance
//! @param[in] buffer Buffer to be copied to queue
//! @param[in] length Length of buffer
//! @return true On success
//! @return false On failure

bool twr_queue_put(twr_queue_t *queue, const void *buffer, size_t length);

//! @brief Get queue to buffer
//! @param[in] queue Instance
//! @param[in] buffer Buffer to be copied from the queue
//! @param[in] length Length of buffer
//! @return true On success
//! @return false On failure

bool twr_queue_get(twr_queue_t *queue, void *buffer, size_t *length);

//! @brief Clear queue
//! @param[in] queue Instance

void twr_queue_clear(twr_queue_t *queue);

//! @}

#endif // _TWR_QUEUE_H
