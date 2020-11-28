#ifndef _HIO_WSSFM10R1AT_H
#define _HIO_WSSFM10R1AT_H

#include <hio_scheduler.h>
#include <hio_gpio.h>
#include <hio_uart.h>

//! @addtogroup hio_wssfm10r1at hio_wssfm10r1at
//! @brief Driver for WSSFM10R1AT SigFox modem
//! @{

//! @cond

#define HIO_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE 64
#define HIO_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE 64

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    HIO_WSSFM10R1AT_EVENT_READY = 0,

    //! @brief Error event
    HIO_WSSFM10R1AT_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    HIO_WSSFM10R1AT_EVENT_SEND_RF_FRAME_START = 2,

    //! @brief RF frame transmission finished event
    HIO_WSSFM10R1AT_EVENT_SEND_RF_FRAME_DONE = 3,

    //! @brief Device ID has been read event
    HIO_WSSFM10R1AT_EVENT_READ_DEVICE_ID = 4,

    //! @brief Device PAC has been read event
    HIO_WSSFM10R1AT_EVENT_READ_DEVICE_PAC = 5

} hio_wssfm10r1at_event_t;

//! @brief WSSFM10R1AT instance

typedef struct hio_wssfm10r1at_t hio_wssfm10r1at_t;

//! @cond

typedef enum
{
    HIO_WSSFM10R1AT_STATE_READY = 0,
    HIO_WSSFM10R1AT_STATE_ERROR = 1,
    HIO_WSSFM10R1AT_STATE_INITIALIZE = 2,
    HIO_WSSFM10R1AT_STATE_INITIALIZE_RESET_L = 3,
    HIO_WSSFM10R1AT_STATE_INITIALIZE_RESET_H = 4,
    HIO_WSSFM10R1AT_STATE_INITIALIZE_AT_COMMAND = 5,
    HIO_WSSFM10R1AT_STATE_INITIALIZE_AT_RESPONSE = 6,
    HIO_WSSFM10R1AT_STATE_SET_POWER_COMMAND = 7,
    HIO_WSSFM10R1AT_STATE_SET_POWER_RESPONSE = 8,
    HIO_WSSFM10R1AT_STATE_SEND_RF_FRAME_COMMAND = 9,
    HIO_WSSFM10R1AT_STATE_SEND_RF_FRAME_RESPONSE = 10,
    HIO_WSSFM10R1AT_STATE_READ_DEVICE_ID_COMMAND = 11,
    HIO_WSSFM10R1AT_STATE_READ_DEVICE_ID_RESPONSE = 12,
    HIO_WSSFM10R1AT_STATE_READ_DEVICE_PAC_COMMAND = 13,
    HIO_WSSFM10R1AT_STATE_READ_DEVICE_PAC_RESPONSE = 14,
    HIO_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_COMMAND = 15,
    HIO_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_RESPONSE = 16,
    HIO_WSSFM10R1AT_STATE_CONTINUOUS_WAVE = 17,
    HIO_WSSFM10R1AT_STATE_DEEP_SLEEP_COMMAND = 18,
    HIO_WSSFM10R1AT_STATE_DEEP_SLEEP_RESPONSE = 19,
    HIO_WSSFM10R1AT_STATE_DEEP_SLEEP = 20

} hio_wssfm10r1at_state_t;

struct hio_wssfm10r1at_t
{
    hio_scheduler_task_id_t _task_id;
    hio_gpio_channel_t _reset_signal;
    hio_uart_channel_t _uart_channel;
    hio_wssfm10r1at_state_t _state;
    hio_wssfm10r1at_state_t _state_after_sleep;
    bool _deep_sleep;
    hio_fifo_t _tx_fifo;
    hio_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[HIO_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[HIO_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(hio_wssfm10r1at_t *, hio_wssfm10r1at_event_t, void *);
    void *_event_param;
    char _command[HIO_WSSFM10R1AT_TX_FIFO_BUFFER_SIZE];
    char _response[HIO_WSSFM10R1AT_RX_FIFO_BUFFER_SIZE];
    uint8_t _message_buffer[12];
    size_t _message_length;
};

//! @endcond

//! @brief Initialize WSSFM10R1AT
//! @param[in] self Instance
//! @param[in] reset_signal GPIO channel where RST signal is connected
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void hio_wssfm10r1at_init(hio_wssfm10r1at_t *self, hio_gpio_channel_t reset_signal, hio_uart_channel_t uart_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_wssfm10r1at_set_event_handler(hio_wssfm10r1at_t *self, void (*event_handler)(hio_wssfm10r1at_t *, hio_wssfm10r1at_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool hio_wssfm10r1at_is_ready(hio_wssfm10r1at_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_wssfm10r1at_send_rf_frame(hio_wssfm10r1at_t *self, const void *buffer, size_t length);

//! @brief Read device ID command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_wssfm10r1at_read_device_id(hio_wssfm10r1at_t *self);

//! @brief Get device ID (can be called only in HIO_WSSFM10R1AT_EVENT_READ_DEVICE_ID event)
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] buffer_size Size of destination buffer
//! @return true If device ID was retrieved
//! @return false If device ID could not be retrieved

bool hio_wssfm10r1at_get_device_id(hio_wssfm10r1at_t *self, char *buffer, size_t buffer_size);

//! @brief Read device PAC command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_wssfm10r1at_read_device_pac(hio_wssfm10r1at_t *self);

//! @brief Get device PAC (can be called only in HIO_WSSFM10R1AT_EVENT_READ_DEVICE_PAC event)
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] buffer_size Size of destination buffer
//! @return true If device PAC was retrieved
//! @return false If device PAC could not be retrieved

bool hio_wssfm10r1at_get_device_pac(hio_wssfm10r1at_t *self, char *buffer, size_t buffer_size);

//! @brief Generate continuous wave command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_wssfm10r1at_continuous_wave(hio_wssfm10r1at_t *self);

//! @}

#endif // _HIO_WSSFM10R1AT_H
