#ifndef _TWR_FIFO_H
#define _TWR_FIFO_H

#include <twr_common.h>

//! @addtogroup twr_fifo twr_fifo
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

} twr_fifo_t;

//! @brief Initialize FIFO buffer
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer where FIFO holds data
//! @param[in] size Size of buffer where FIFO holds data

void twr_fifo_init(twr_fifo_t *fifo, void *buffer, size_t size);

//! @brief Purge FIFO buffer
//! @param[in] fifo FIFO instance

void twr_fifo_purge(twr_fifo_t *fifo);

//! @brief Write data to FIFO
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer from which data will be written
//! @param[in] length Number of requested bytes to be written
//! @return Number of bytes written

size_t twr_fifo_write(twr_fifo_t *fifo, const void *buffer, size_t length);

//! @brief Read data from FIFO
//! @param[in] fifo FIFO instance
//! @param[out] buffer Pointer to buffer where data will be read
//! @param[in] length Number of requested bytes to be read
//! @return Number of bytes read

size_t twr_fifo_read(twr_fifo_t *fifo, void *buffer, size_t length);

//! @brief Write data to FIFO from interrupt
//! @param[in] fifo FIFO instance
//! @param[in] buffer Pointer to buffer from which data will be written
//! @param[in] length Number of requested bytes to be written
//! @return Number of bytes written

size_t twr_fifo_irq_write(twr_fifo_t *fifo, const void *buffer, size_t length);

//! @brief Read data from FIFO from interrupt
//! @param[in] fifo FIFO instance
//! @param[out] buffer Pointer to buffer where data will be read
//! @param[in] length Number of requested bytes to be read
//! @return Number of bytes read

size_t twr_fifo_irq_read(twr_fifo_t *fifo, void *buffer, size_t length);

//! @brief Is empty
//! @param[in] fifo FIFO instance
//! @return true When is empty
//! @return false When is not empty

bool twr_fifo_is_empty(twr_fifo_t *fifo);

//! @}

#endif // _TWR_FIFO_H
