#ifndef _BC_UART_H
#define _BC_UART_H

#include <bc_tick.h>
#include <bc_fifo.h>

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

//! @brief UART baudrate

typedef enum
{
    //! @brief UART baudrat 9600 bps
    BC_UART_BAUDRATE_9600 = 0,

    //! @brief UART baudrat 19200 bps
    BC_UART_BAUDRATE_19200 = 1,

    //! @brief UART baudrat 38400 bps
    BC_UART_BAUDRATE_38400 = 2,

    //! @brief UART baudrat 57600 bps
    BC_UART_BAUDRATE_57600 = 3,

    //! @brief UART baudrat 115200 bps
    BC_UART_BAUDRATE_115200 = 4,

    //! @brief UART baudrat 921600 bps
    BC_UART_BAUDRATE_921600 = 5

} bc_uart_baudrate_t;

//! @brief UART setting

//! @cond

#define _BC_UART_SETTING_DATA_BITS_7   0x07 << 4
#define _BC_UART_SETTING_DATA_BITS_8   0x08 << 4

#define _BC_UART_SETTING_PARITY_NONE   0x00 << 2
#define _BC_UART_SETTING_PARITY_EVEN   0x02 << 2
#define _BC_UART_SETTING_PARITY_ODD    0x03 << 2

#define _BC_UART_SETTING_STOP_BIT_1   0x00
#define _BC_UART_SETTING_STOP_BIT_2   0x02
#define _BC_UART_SETTING_STOP_BIT_15  0x03

//! @endcond

typedef enum
{
    //! @brief 8N1: 8 data bits, none parity bit, 1 stop bit
    BC_UART_SETTING_8N1 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_1,

    //! @brief 8E1: 8 data bits, even parity bit, 1 stop bit
    BC_UART_SETTING_8E1 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_EVEN | _BC_UART_SETTING_STOP_BIT_1,

    //! @brief 8O1: 8 data bits, odd parity bit, 1 stop bit
    BC_UART_SETTING_8O1 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_ODD | _BC_UART_SETTING_STOP_BIT_1,

    //! @brief 8N2: 8 data bits, none parity bit, 2 stop bits
    BC_UART_SETTING_8N2 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_2,

    //! @brief 8E2: 8 data bits, even parity bit, 2 stop bit
    BC_UART_SETTING_8E2 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_EVEN | _BC_UART_SETTING_STOP_BIT_2,

    //! @brief 8O2: 8 data bits, odd parity bit, 2 stop bit
    BC_UART_SETTING_8O2 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_2,

    //! @brief 8N1_5: 8 data bits, none parity bit, 1.5 stop bits
    BC_UART_SETTING_8N1_5 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_15,

    //! @brief 8E1_5: 8 data bits, even parity bit, 1.5 stop bit
    BC_UART_SETTING_8E1_5 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_EVEN | _BC_UART_SETTING_STOP_BIT_15,

    //! @brief 8O1_5: 8 data bits, odd parity bit, 1.5 stop bit
    BC_UART_SETTING_8O1_5 = _BC_UART_SETTING_DATA_BITS_8 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_15,

    //! @brief 7N1: 7 data bits, none parity bit, 1 stop bit
    BC_UART_SETTING_7N1 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_1,

    //! @brief 7E1: 7 data bits, even parity bit, 1 stop bit
    BC_UART_SETTING_7E1 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_EVEN | _BC_UART_SETTING_STOP_BIT_1,

    //! @brief 7O1: 7 data bits, odd parity bit, 1 stop bit
    BC_UART_SETTING_7O1 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_ODD | _BC_UART_SETTING_STOP_BIT_1,

    //! @brief 7N2: 7 data bits, none parity bit, 2 stop bits
    BC_UART_SETTING_7N2 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_2,

    //! @brief 7E2: 7 data bits, even parity bit, 2 stop bit
    BC_UART_SETTING_7E2 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_EVEN | _BC_UART_SETTING_STOP_BIT_2,

    //! @brief 7O2: 7 data bits, odd parity bit, 2 stop bit
    BC_UART_SETTING_7O2 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_2,

    //! @brief 7N1_5: 7 data bits, none parity bit, 1.5 stop bits
    BC_UART_SETTING_7N1_5 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_15,

    //! @brief 7E1_5: 7 data bits, even parity bit, 1.5 stop bit
    BC_UART_SETTING_7E1_5 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_EVEN | _BC_UART_SETTING_STOP_BIT_15,

    //! @brief 7O1_5: 7 data bits, odd parity bit, 1.5 stop bit
    BC_UART_SETTING_7O1_5 = _BC_UART_SETTING_DATA_BITS_7 | _BC_UART_SETTING_PARITY_NONE | _BC_UART_SETTING_STOP_BIT_15

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

//! @brief Deinitialize UART channel
//! @param[in] channel UART channel

void bc_uart_deinit(bc_uart_channel_t channel);

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
