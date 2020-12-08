#ifndef _TWR_TD1207R_H
#define _TWR_TD1207R_H

#include <twr_scheduler.h>
#include <twr_gpio.h>
#include <twr_uart.h>

//! @addtogroup twr_td1207r twr_td1207r
//! @brief Driver for TD1207R SigFox modem
//! @{

//! @cond

#define TWR_TD1207R_TX_FIFO_BUFFER_SIZE 64
#define TWR_TD1207R_RX_FIFO_BUFFER_SIZE 64

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    TWR_TD1207R_EVENT_READY = 0,

    //! @brief Error event
    TWR_TD1207R_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    TWR_TD1207R_EVENT_SEND_RF_FRAME_START = 2,

    //! @brief RF frame transmission finished event
    TWR_TD1207R_EVENT_SEND_RF_FRAME_DONE = 3

} twr_td1207r_event_t;

//! @brief TD1207R instance

typedef struct twr_td1207r_t twr_td1207r_t;

//! @cond

typedef enum
{
    TWR_TD1207R_STATE_READY = 0,
    TWR_TD1207R_STATE_ERROR = 1,
    TWR_TD1207R_STATE_INITIALIZE = 2,
    TWR_TD1207R_STATE_INITIALIZE_RESET_L = 3,
    TWR_TD1207R_STATE_INITIALIZE_RESET_H = 4,
    TWR_TD1207R_STATE_INITIALIZE_AT_COMMAND = 5,
    TWR_TD1207R_STATE_INITIALIZE_AT_RESPONSE = 6,
    TWR_TD1207R_STATE_SEND_RF_FRAME_COMMAND = 7,
    TWR_TD1207R_STATE_SEND_RF_FRAME_RESPONSE = 8

} twr_td1207r_state_t;

struct twr_td1207r_t
{
    twr_scheduler_task_id_t _task_id;
    twr_gpio_channel_t _reset_signal;
    twr_uart_channel_t _uart_channel;
    twr_td1207r_state_t _state;
    twr_fifo_t _tx_fifo;
    twr_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[TWR_TD1207R_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[TWR_TD1207R_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(twr_td1207r_t *, twr_td1207r_event_t, void *);
    void *_event_param;
    char _command[TWR_TD1207R_TX_FIFO_BUFFER_SIZE];
    char _response[TWR_TD1207R_RX_FIFO_BUFFER_SIZE];
    uint8_t _message_buffer[12];
    size_t _message_length;
};

//! @endcond

//! @brief Initialize TD1207R
//! @param[in] self Instance
//! @param[in] reset_signal GPIO channel where RST signal is connected
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void twr_td1207r_init(twr_td1207r_t *self, twr_gpio_channel_t reset_signal, twr_uart_channel_t uart_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_td1207r_set_event_handler(twr_td1207r_t *self, void (*event_handler)(twr_td1207r_t *, twr_td1207r_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool twr_td1207r_is_ready(twr_td1207r_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool twr_td1207r_send_rf_frame(twr_td1207r_t *self, const void *buffer, size_t length);

//! @}

#endif // _TWR_TD1207R_H
