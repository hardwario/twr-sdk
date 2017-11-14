#ifndef _BC_FIFO_H
#define _BC_FIFO_H

#include <bc_common.h>

//! @addtogroup bc_fifo bc_fifo
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

} bc_fifo_t;

//! @brief Initialize FIFO buffer
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer where FIFO holds data
//! @param[in] size Size of buffer where FIFO holds data

void bc_fifo_init(bc_fifo_t *fifo, void *buffer, size_t size);

//! @brief Purge FIFO buffer
//! @param[in] fifo FIFO instance

void bc_fifo_purge(bc_fifo_t *fifo);

//! @brief Write data to FIFO
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer from which data will be written
//! @param[in] length Number of requested bytes to be written
//! @return Number of bytes written

size_t bc_fifo_write(bc_fifo_t *fifo, const void *buffer, size_t length);

//! @brief Read data from FIFO
//! @param[in] fifo FIFO instance
//! @param[out] buffer Pointer to buffer where data will be read
//! @param[in] length Number of requested bytes to be read
//! @return Number of bytes read

size_t bc_fifo_read(bc_fifo_t *fifo, void *buffer, size_t length);

//! @brief Write data to FIFO from interrupt
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer from which data will be written
//! @param[in] length Number of requested bytes to be written
//! @return Number of bytes written

size_t bc_fifo_irq_write(bc_fifo_t *fifo, const void *buffer, size_t length);

//! @brief Read data from FIFO from interrupt
//! @param[in] fifo FIFO instance
//! @param[out] buffer Pointer to buffer where data will be read
//! @param[in] length Number of requested bytes to be read
//! @return Number of bytes read

size_t bc_fifo_irq_read(bc_fifo_t *fifo, void *buffer, size_t length);

//! @brief Is empty
//! @param[in] fifo FIFO instance
//! @return true When is empty
//! @return false When is not empty

bool bc_fifo_is_empty(bc_fifo_t *fifo);

//! @}

#endif // _BC_FIFO_H
