#ifndef _BC_UART_H
#define _BC_UART_H

#include <bc_fifo.h>
#include <bc_tick.h>

//! @addtogroup bc_uart bc_uart
//! @brief Driver for UART (universal asynchronous receiver/transmitter)
//! @{

//! @cond

#define _BC_UART_DATA_BITS_8 0x00000000
#define _BC_UART_PARITY_NONE 0x00000000
#define _BC_UART_STOP_BITS_1 0x00000000

//! @endcond

//! @brief UART configurations

typedef enum
{
    BC_UART_CONFIG_9600_8N1 = 9600 | _BC_UART_DATA_BITS_8 | _BC_UART_PARITY_NONE | _BC_UART_STOP_BITS_1

} bc_uart_config_t;

//! @brief UART channels

typedef enum
{
    //! @brief UART channel UART1
    BC_UART_UART1 = 1

} bc_uart_channel_t;

//! @brief Initialize UART channel
//! @param[in] channel UART channel
//! @param[in] config UART configuration

void bc_uart_init(bc_uart_channel_t channel, bc_uart_config_t config);

//! @brief Write data to UART channel
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return Number of bytes written

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length);

//! @brief Read data from UART channel
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @param[in] timeout Read operation timeout in ticks
//! @return Number of bytes read

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout);

//! @}

#endif // _BC_UART_H
