#ifndef _BC_UART_H
#define _BC_UART_H

#include <bc_fifo.h>
#include <bc_tick.h>

//! @addtogroup bc_uart bc_uart
//! @brief Driver for UART (universal asynchronous receiver/transmitter)
//! @{

//! @brief UART channels

typedef enum
{
    //! @brief UART channel UART0
    BC_UART_UART0 = 0,

    //! @brief UART channel UART1
    BC_UART_UART1 = 1,

    //! @brief UART channel UART2
    BC_UART_UART2 = 2

} bc_uart_channel_t;

//! @brief UART baudrates

typedef enum
{
    //! @brief UART baudrate 9600 Bd
    BC_UART_BAUDRATE_9600_BD = 0,

    //! @brief UART baudrate 115200 Bd
    BC_UART_BAUDRATE_115200_BD = 1

} bc_uart_baudrate_t;

//! @brief UART initialization parameters

typedef struct
{
    bc_uart_baudrate_t baudrate;

} bc_uart_param_t;

//! @brief Initialize UART channel
//! @param[in] channel UART channel
//! @param[in] param Initialization parameters
//! @param[in] tx_fifo TX FIFO to be used for transmit operation (has to be initialized by user)
//! @param[in] rx_fifo RX FIFO to be used for receive operation (has to be initialized by user)

void bc_uart_init(bc_uart_channel_t channel, bc_uart_param_t *param, bc_fifo_t *tx_fifo, bc_fifo_t *rx_fifo);

//! @brief Write data to UART channel
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @param[in] timeout Write operation timeout
//! @return Number of bytes written

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length, bc_tick_t timeout);

//! @brief Read data from UART channel
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @param[in] timeout Write operation timeout in ticks
//! @return Number of bytes read

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout);

//! @}

#endif // _BC_UART_H
