#ifndef _BC_CMWX1ZZABZ_H
#define _BC_CMWX1ZZABZ_H

#include <bc_scheduler.h>
#include <bc_gpio.h>
#include <bc_uart.h>

//! @addtogroup bc_cmwx1zzabz bc_cmwx1zzabz
//! @brief Driver for CMWX1ZZABZ SigFox modem
//! @{

//! @cond

#define BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE 64
#define BC_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE 64

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    BC_CMWX1ZZABZ_EVENT_READY = 0,

    //! @brief Error event
    BC_CMWX1ZZABZ_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    BC_CMWX1ZZABZ_EVENT_SEND_RF_FRAME_START = 2,

    //! @brief RF frame transmission finished event
    BC_CMWX1ZZABZ_EVENT_SEND_RF_FRAME_DONE = 3,
} bc_cmwx1zzabz_event_t;

//! @brief CMWX1ZZABZ instance

typedef struct bc_cmwx1zzabz_t bc_cmwx1zzabz_t;

//! @cond

typedef enum
{
    BC_CMWX1ZZABZ_STATE_READY = 0,
    BC_CMWX1ZZABZ_STATE_ERROR = 1,
    BC_CMWX1ZZABZ_STATE_INITIALIZE = 2,
    BC_CMWX1ZZABZ_STATE_IDLE = 3,
    BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND = 5,
    BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_RESPONSE = 6,
    BC_CMWX1ZZABZ_STATE_SEND_RF_FRAME_COMMAND = 9,
    BC_CMWX1ZZABZ_STATE_SEND_RF_FRAME_RESPONSE = 10,
} bc_cmwx1zzabz_state_t;

struct bc_cmwx1zzabz_t
{
    bc_scheduler_task_id_t _task_id;
    bc_uart_channel_t _uart_channel;
    bc_cmwx1zzabz_state_t _state;
    bc_cmwx1zzabz_state_t _state_after_sleep;
    bool _deep_sleep;
    bc_fifo_t _tx_fifo;
    bc_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[BC_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(bc_cmwx1zzabz_t *, bc_cmwx1zzabz_event_t, void *);
    void *_event_param;
    char _command[BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE];
    char _response[BC_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE];
    uint8_t _message_buffer[12];
    size_t _message_length;

    uint8_t init_command_index;
};

//! @endcond

//! @brief Initialize CMWX1ZZABZ
//! @param[in] self Instance
//! @param[in] reset_signal GPIO channel where RST signal is connected
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void bc_cmwx1zzabz_init(bc_cmwx1zzabz_t *self, bc_uart_channel_t uart_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_cmwx1zzabz_set_event_handler(bc_cmwx1zzabz_t *self, void (*event_handler)(bc_cmwx1zzabz_t *, bc_cmwx1zzabz_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool bc_cmwx1zzabz_is_ready(bc_cmwx1zzabz_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool bc_cmwx1zzabz_send_rf_frame(bc_cmwx1zzabz_t *self, const void *buffer, size_t length);

//! @brief Read device ID command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

//! @}

#endif // _BC_CMWX1ZZABZ_H
