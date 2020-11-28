#ifndef _HIO_FIFO_H
#define _HIO_FIFO_H

#include <hio_common.h>

//! @addtogroup hio_fifo hio_fifo
//! @brief FIFO buffer implementation
//! @{

//! @brief Structure of FIFO instance

typedef struct
{
    //! @brief Pointer to buffer where FIFO holds data
    void *buffer;

    //! @brief Size of buffer where FIFO holds data
    size_t size;

    //! @brief Position of FIFO's head
    size_t head;

    //! @brief Position of FIFO's tail
    size_t tail;

} hio_fifo_t;

//! @brief Initialize FIFO buffer
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer where FIFO holds data
//! @param[in] size Size of buffer where FIFO holds data

void hio_fifo_init(hio_fifo_t *fifo, void *buffer, size_t size);

//! @brief Purge FIFO buffer
//! @param[in] fifo FIFO instance

void hio_fifo_purge(hio_fifo_t *fifo);

//! @brief Write data to FIFO
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer from which data will be written
//! @param[in] length Number of requested bytes to be written
//! @return Number of bytes written

size_t hio_fifo_write(hio_fifo_t *fifo, const void *buffer, size_t length);

//! @brief Read data from FIFO
//! @param[in] fifo FIFO instance
//! @param[out] buffer Pointer to buffer where data will be read
//! @param[in] length Number of requested bytes to be read
//! @return Number of bytes read

size_t hio_fifo_read(hio_fifo_t *fifo, void *buffer, size_t length);

//! @brief Write data to FIFO from interrupt
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer from which data will be written
//! @param[in] length Number of requested bytes to be written
//! @return Number of bytes written

size_t hio_fifo_irq_write(hio_fifo_t *fifo, const void *buffer, size_t length);

//! @brief Read data from FIFO from interrupt
//! @param[in] fifo FIFO instance
//! @param[out] buffer Pointer to buffer where data will be read
//! @param[in] length Number of requested bytes to be read
//! @return Number of bytes read

size_t hio_fifo_irq_read(hio_fifo_t *fifo, void *buffer, size_t length);

//! @brief Is empty
//! @param[in] fifo FIFO instance
//! @return true When is empty
//! @return false When is not empty

bool hio_fifo_is_empty(hio_fifo_t *fifo);

//! @}

#endif // _HIO_FIFO_H
