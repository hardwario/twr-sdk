#ifndef _BC_UART_H
#define _BC_UART_H

#include <bc_tick.h>
#include <bc_fifo.h>

//! @addtogroup bc_uart bc_uart
//! @brief Driver for UART (universal asynchronous receiver/transmitter)
//! @{

//! @cond

#define _BC_UART_DATA_BITS_8 0x00000000
#define _BC_UART_PARITY_NONE 0x00000000
#define _BC_UART_STOP_BITS_1 0x00000000

// Todo remove
#define BC_UART_CONFIG_9600_8N1 BC_UART_BAUDRATE_9600, BC_UART_SETTINGS_8N1

//! @endcond

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

//! @brief UART baudrate

typedef enum
{
	BC_UART_BAUDRATE_9600 = 0,
	BC_UART_BAUDRATE_115200 = 1

} bc_uart_baudrate_t;

//! @brief UART setting

typedef enum
{
	BC_UART_SETTINGS_8N1

} bc_uart_setting_t;

//! @brief Callback events

typedef enum
{
    //! @brief Event is writting done
    BC_UART_EVENT_ASYNC_WRITE_DONE = 0,

    //! @brief Event is reading done
    BC_UART_EVENT_ASYNC_READ_DATA = 1,

    //! @brief Event is timeout
    BC_UART_EVENT_ASYNC_READ_TIMEOUT = 2

} bc_uart_event_t;

//! @brief Initialize UART channel
//! @param[in] channel UART channel
//! @param[in] config UART configuration

void bc_uart_init(bc_uart_channel_t channel, bc_uart_baudrate_t baudrate, bc_uart_setting_t setting);

//! @brief Write data to UART channel (blocking call)
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return Number of bytes written

size_t bc_uart_write(bc_uart_channel_t channel, const void *buffer, size_t length);

//! @brief Read data from UART channel (blocking call)
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @param[in] timeout Read operation timeout in ticks
//! @return Number of bytes read

size_t bc_uart_read(bc_uart_channel_t channel, void *buffer, size_t length, bc_tick_t timeout);

//! @brief Set callback function
//! @param[in] channel UART channel
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_uart_set_event_handler(bc_uart_channel_t channel, void (*event_handler)(bc_uart_channel_t, bc_uart_event_t, void *), void *event_param);

//! @brief Set buffers for async transfers
//! @param[in] channel UART channel
//! @param[in] write_fifo Pointer to writing fifo
//! @param[in] read_fifo Pointer to reader fifo

void bc_uart_set_async_fifo(bc_uart_channel_t channel, bc_fifo_t *write_fifo, bc_fifo_t *read_fifo);

//! @brief Add data to be transmited in async mode
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to buffer
//! @param[in] length Length of data to be added
//! @return Number of bytes added

size_t bc_uart_async_write(bc_uart_channel_t channel, const void *buffer, size_t length);

//! @brief Start async reading
//! @param[in] channel UART channel
//! @param[in] timeout Maximum timeout in ms
//! @return true On success
//! @return false On failure

bool bc_uart_async_read_start(bc_uart_channel_t channel, bc_tick_t timeout);

//! @brief Cancel async reading
//! @param[in] channel UART channel
//! @return true On success
//! @return false On failure

bool bc_uart_async_read_cancel(bc_uart_channel_t channel);

//! @brief Get data that has been received in async mode
//! @param[in] channel UART channel
//! @param[in] buffer Pointer to buffer
//! @param[in] length Maximum length of received data
//! @return Number of received bytes

size_t bc_uart_async_read(bc_uart_channel_t channel, void *buffer, size_t length);

//! @}

#endif // _BC_UART_H
