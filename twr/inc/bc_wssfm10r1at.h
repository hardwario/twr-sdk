#ifndef _TWR_WSSFM10R1AT_H
#define _TWR_WSSFM10R1AT_H

#include <twr_scheduler.h>
#include <twr_gpio.h>
#include <twr_uart.h>

//! @addtogroup twr_wssfm10r1at twr_wssfm10r1at
//! @brief Driver for WSSFM10R1AT SigFox modem
//! @{

//! @cond

#define TWR_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE 64
#define TWR_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE 64

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    TWR_WSSFM10R1AT_EVENT_READY = 0,

    //! @brief Error event
    TWR_WSSFM10R1AT_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    TWR_WSSFM10R1AT_EVENT_SEND_RF_FRAME_START = 2,

    //! @brief RF frame transmission finished event
    TWR_WSSFM10R1AT_EVENT_SEND_RF_FRAME_DONE = 3,

    //! @brief Device ID has been read event
    TWR_WSSFM10R1AT_EVENT_READ_DEVICE_ID = 4,

    //! @brief Device PAC has been read event
    TWR_WSSFM10R1AT_EVENT_READ_DEVICE_PAC = 5

} twr_wssfm10r1at_event_t;

//! @brief WSSFM10R1AT instance

typedef struct twr_wssfm10r1at_t twr_wssfm10r1at_t;

//! @cond

typedef enum
{
    TWR_WSSFM10R1AT_STATE_READY = 0,
    TWR_WSSFM10R1AT_STATE_ERROR = 1,
    TWR_WSSFM10R1AT_STATE_INITIALIZE = 2,
    TWR_WSSFM10R1AT_STATE_INITIALIZE_RESET_L = 3,
    TWR_WSSFM10R1AT_STATE_INITIALIZE_RESET_H = 4,
    TWR_WSSFM10R1AT_STATE_INITIALIZE_AT_COMMAND = 5,
    TWR_WSSFM10R1AT_STATE_INITIALIZE_AT_RESPONSE = 6,
    TWR_WSSFM10R1AT_STATE_SET_POWER_COMMAND = 7,
    TWR_WSSFM10R1AT_STATE_SET_POWER_RESPONSE = 8,
    TWR_WSSFM10R1AT_STATE_SEND_RF_FRAME_COMMAND = 9,
    TWR_WSSFM10R1AT_STATE_SEND_RF_FRAME_RESPONSE = 10,
    TWR_WSSFM10R1AT_STATE_READ_DEVICE_ID_COMMAND = 11,
    TWR_WSSFM10R1AT_STATE_READ_DEVICE_ID_RESPONSE = 12,
    TWR_WSSFM10R1AT_STATE_READ_DEVICE_PAC_COMMAND = 13,
    TWR_WSSFM10R1AT_STATE_READ_DEVICE_PAC_RESPONSE = 14,
    TWR_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_COMMAND = 15,
    TWR_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_RESPONSE = 16,
    TWR_WSSFM10R1AT_STATE_CONTINUOUS_WAVE = 17,
    TWR_WSSFM10R1AT_STATE_DEEP_SLEEP_COMMAND = 18,
    TWR_WSSFM10R1AT_STATE_DEEP_SLEEP_RESPONSE = 19,
    TWR_WSSFM10R1AT_STATE_DEEP_SLEEP = 20

} twr_wssfm10r1at_state_t;

struct twr_wssfm10r1at_t
{
    twr_scheduler_task_id_t _task_id;
    twr_gpio_channel_t _reset_signal;
    twr_uart_channel_t _uart_channel;
    twr_wssfm10r1at_state_t _state;
    twr_wssfm10r1at_state_t _state_after_sleep;
    bool _deep_sleep;
    twr_fifo_t _tx_fifo;
    twr_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[TWR_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[TWR_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(twr_wssfm10r1at_t *, twr_wssfm10r1at_event_t, void *);
    void *_event_param;
    char _command[TWR_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE];
    char _response[TWR_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE];
    uint8_t _message_buffer[12];
    size_t _message_length;
};

//! @endcond

//! @brief Initialize WSSFM10R1AT
//! @param[in] self Instance
//! @param[in] reset_signal GPIO channel where RST signal is connected
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void twr_wssfm10r1at_init(twr_wssfm10r1at_t *self, twr_gpio_channel_t reset_signal, twr_uart_channel_t uart_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_wssfm10r1at_set_event_handler(twr_wssfm10r1at_t *self, void (*event_handler)(twr_wssfm10r1at_t *, twr_wssfm10r1at_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool twr_wssfm10r1at_is_ready(twr_wssfm10r1at_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool twr_wssfm10r1at_send_rf_frame(twr_wssfm10r1at_t *self, const void *buffer, size_t length);

//! @brief Read device ID command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool twr_wssfm10r1at_read_device_id(twr_wssfm10r1at_t *self);

//! @brief Get device ID (can be called only in TWR_WSSFM10R1AT_EVENT_READ_DEVICE_ID event)
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] buffer_size Size of destination buffer
//! @return true If device ID was retrieved
//! @return false If device ID could not be retrieved

bool twr_wssfm10r1at_get_device_id(twr_wssfm10r1at_t *self, char *buffer, size_t buffer_size);

//! @brief Read device PAC command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool twr_wssfm10r1at_read_device_pac(twr_wssfm10r1at_t *self);

//! @brief Get device PAC (can be called only in TWR_WSSFM10R1AT_EVENT_READ_DEVICE_PAC event)
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] buffer_size Size of destination buffer
//! @return true If device PAC was retrieved
//! @return false If device PAC could not be retrieved

bool twr_wssfm10r1at_get_device_pac(twr_wssfm10r1at_t *self, char *buffer, size_t buffer_size);

//! @brief Generate continuous wave command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool twr_wssfm10r1at_continuous_wave(twr_wssfm10r1at_t *self);

//! @}

#endif // _TWR_WSSFM10R1AT_H
