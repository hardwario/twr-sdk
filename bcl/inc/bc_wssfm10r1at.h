#ifndef _BC_WSSFM10R1AT_H
#define _BC_WSSFM10R1AT_H

#include <bc_scheduler.h>
#include <bc_gpio.h>
#include <bc_uart.h>

//! @addtogroup bc_wssfm10r1at bc_wssfm10r1at
//! @brief Driver for WSSFM10R1AT SigFox modem
//! @{

//! @cond

#define BC_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE 64
#define BC_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE 64

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    BC_WSSFM10R1AT_EVENT_READY = 0,

    //! @brief Error event
    BC_WSSFM10R1AT_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    BC_WSSFM10R1AT_EVENT_SEND_RF_FRAME_START = 2,

    //! @brief RF frame transmission finished event
    BC_WSSFM10R1AT_EVENT_SEND_RF_FRAME_DONE = 3,

    //! @brief Device ID has been read event
    BC_WSSFM10R1AT_EVENT_READ_DEVICE_ID = 4,

    //! @brief Device PAC has been read event
    BC_WSSFM10R1AT_EVENT_READ_DEVICE_PAC = 5

} bc_wssfm10r1at_event_t;

//! @brief WSSFM10R1AT instance

typedef struct bc_wssfm10r1at_t bc_wssfm10r1at_t;

//! @cond

typedef enum
{
    BC_WSSFM10R1AT_STATE_READY = 0,
    BC_WSSFM10R1AT_STATE_ERROR = 1,
    BC_WSSFM10R1AT_STATE_INITIALIZE = 2,
    BC_WSSFM10R1AT_STATE_INITIALIZE_RESET_L = 3,
    BC_WSSFM10R1AT_STATE_INITIALIZE_RESET_H = 4,
    BC_WSSFM10R1AT_STATE_INITIALIZE_AT_COMMAND = 5,
    BC_WSSFM10R1AT_STATE_INITIALIZE_AT_RESPONSE = 6,
    BC_WSSFM10R1AT_STATE_SET_POWER_COMMAND = 7,
    BC_WSSFM10R1AT_STATE_SET_POWER_RESPONSE = 8,
    BC_WSSFM10R1AT_STATE_SEND_RF_FRAME_COMMAND = 9,
    BC_WSSFM10R1AT_STATE_SEND_RF_FRAME_RESPONSE = 10,
    BC_WSSFM10R1AT_STATE_READ_DEVICE_ID_COMMAND = 11,
    BC_WSSFM10R1AT_STATE_READ_DEVICE_ID_RESPONSE = 12,
    BC_WSSFM10R1AT_STATE_READ_DEVICE_PAC_COMMAND = 13,
    BC_WSSFM10R1AT_STATE_READ_DEVICE_PAC_RESPONSE = 14,
    BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_COMMAND = 15,
    BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_RESPONSE = 16,
    BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE = 17,
    BC_WSSFM10R1AT_STATE_DEEP_SLEEP_COMMAND = 18,
    BC_WSSFM10R1AT_STATE_DEEP_SLEEP_RESPONSE = 19,
    BC_WSSFM10R1AT_STATE_DEEP_SLEEP = 20

} bc_wssfm10r1at_state_t;

struct bc_wssfm10r1at_t
{
    bc_scheduler_task_id_t _task_id;
    bc_gpio_channel_t _reset_signal;
    bc_uart_channel_t _uart_channel;
    bc_wssfm10r1at_state_t _state;
    bc_wssfm10r1at_state_t _state_after_sleep;
    bool _deep_sleep;
    bc_fifo_t _tx_fifo;
    bc_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[BC_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[BC_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(bc_wssfm10r1at_t *, bc_wssfm10r1at_event_t, void *);
    void *_event_param;
    char _command[BC_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE];
    char _response[BC_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE];
    uint8_t _message_buffer[12];
    size_t _message_length;
};

//! @endcond

//! @brief Initialize WSSFM10R1AT
//! @param[in] self Instance
//! @param[in] reset_signal GPIO channel where RST signal is connected
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void bc_wssfm10r1at_init(bc_wssfm10r1at_t *self, bc_gpio_channel_t reset_signal, bc_uart_channel_t uart_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_wssfm10r1at_set_event_handler(bc_wssfm10r1at_t *self, void (*event_handler)(bc_wssfm10r1at_t *, bc_wssfm10r1at_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true if ready
//! @return false if not ready

bool bc_wssfm10r1at_is_ready(bc_wssfm10r1at_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_wssfm10r1at_send_rf_frame(bc_wssfm10r1at_t *self, const void *buffer, size_t length);

//! @brief Read device ID command
//! @param[in] self Instance
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_wssfm10r1at_read_device_id(bc_wssfm10r1at_t *self);

//! @brief Read device PAC command
//! @param[in] self Instance
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_wssfm10r1at_read_device_pac(bc_wssfm10r1at_t *self);

//! @brief Generate continuous wave command
//! @param[in] self Instance
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_wssfm10r1at_continuous_wave(bc_wssfm10r1at_t *self);

//! @}

#endif // _BC_WSSFM10R1AT_H
