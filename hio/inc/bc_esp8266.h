#ifndef _HIO_ESP8266_H
#define _HIO_ESP8266_H

#include <hio_scheduler.h>
#include <hio_gpio.h>
#include <hio_uart.h>

//! @addtogroup hio_esp8266 hio_esp8266
//! @brief Driver for ESP8266 WiFi module
//! @{

//! @cond

#define HIO_ESP8266_TX_MAX_PACKET_SIZE 1024

#define HIO_ESP8266_TX_FIFO_BUFFER_SIZE HIO_ESP8266_TX_MAX_PACKET_SIZE
#define HIO_ESP8266_RX_FIFO_BUFFER_SIZE 2048

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    HIO_ESP8266_EVENT_READY = 0,

    //! @brief Error event
    HIO_ESP8266_EVENT_ERROR = 1,

    HIO_ESP8266_EVENT_WIFI_CONNECT_SUCCESS = 2,
    HIO_ESP8266_EVENT_WIFI_CONNECT_ERROR = 3,
    HIO_ESP8266_EVENT_SOCKET_CONNECT_SUCCESS = 4,
    HIO_ESP8266_EVENT_SOCKET_CONNECT_ERROR = 5,
    HIO_ESP8266_EVENT_SOCKET_SEND_SUCCESS = 6,
    HIO_ESP8266_EVENT_SOCKET_SEND_ERROR = 7,
    HIO_ESP8266_EVENT_DATA_RECEIVED = 8,
    HIO_ESP8266_EVENT_AP_AVAILABILITY_RESULT = 9,
    HIO_ESP8266_EVENT_DISCONNECTED = 10

} hio_esp8266_event_t;

//! @brief ESP8266 instance

typedef struct hio_esp8266_t hio_esp8266_t;

typedef enum
{
    HIO_ESP8266_STATE_READY = 0,
    HIO_ESP8266_STATE_ERROR = 1,
    HIO_ESP8266_STATE_INITIALIZE = 2,
    HIO_ESP8266_STATE_IDLE = 3,
    HIO_ESP8266_STATE_RECEIVE = 4,
    HIO_ESP8266_STATE_INITIALIZE_COMMAND_SEND = 5,
    HIO_ESP8266_STATE_INITIALIZE_COMMAND_RESPONSE = 6,
    HIO_ESP8266_STATE_WIFI_CONNECT_COMMAND = 7,
    HIO_ESP8266_STATE_SOCKET_CONNECT_COMMAND = 8,
    HIO_ESP8266_STATE_SOCKET_SEND_COMMAND = 9,
    HIO_ESP8266_STATE_SOCKET_SEND_DATA = 10,
    HIO_ESP8266_STATE_WIFI_CONNECT_RESPONSE = 11,
    HIO_ESP8266_STATE_SOCKET_CONNECT_RESPONSE = 12,
    HIO_ESP8266_STATE_SOCKET_SEND_RESPONSE = 13,
    HIO_ESP8266_STATE_WIFI_CONNECT_ERROR = 14,
    HIO_ESP8266_STATE_SOCKET_CONNECT_ERROR = 15,
    HIO_ESP8266_STATE_SOCKET_SEND_ERROR = 16,
    HIO_ESP8266_STATE_SOCKET_RECEIVE = 17,
    HIO_ESP8266_STATE_DISCONNECTED = 18,
    HIO_ESP8266_STATE_SNTP_CONFIG_COMMAND = 19,
    HIO_ESP8266_STATE_SNTP_CONFIG_RESPONSE = 20,
    HIO_ESP8266_STATE_SNTP_TIME_COMMAND = 21,
    HIO_ESP8266_STATE_SNTP_TIME_RESPONSE = 22,
    HIO_ESP8266_STATE_AP_AVAILABILITY_OPT_COMMAND = 23,
    HIO_ESP8266_STATE_AP_AVAILABILITY_OPT_RESPONSE = 24,
    HIO_ESP8266_STATE_AP_AVAILABILITY_COMMAND = 25,
    HIO_ESP8266_STATE_AP_AVAILABILITY_RESPONSE = 26

} hio_esp8266_state_t;

typedef enum
{
    HIO_ESP8266_CONFIG_MODE_STATION = 0,
    HIO_ESP8266_CONFIG_MODE_AP = 1
} hio_esp8266_config_mode_t;

typedef struct
{
    hio_esp8266_config_mode_t mode;
    char ssid[64];
    char password[64];
    uint8_t sntp_enabled;
    int sntp_timezone;
    char sntp_server1[128];
    char sntp_server2[128];
    char sntp_server3[128];
} hio_esp8266_config;

struct hio_esp8266_t
{
    hio_scheduler_task_id_t _task_id;
    hio_uart_channel_t _uart_channel;
    hio_esp8266_state_t _state;
    hio_esp8266_state_t _state_after_init;
    hio_fifo_t _tx_fifo;
    hio_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[HIO_ESP8266_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[HIO_ESP8266_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(hio_esp8266_t *, hio_esp8266_event_t, void *);
    void *_event_param;
    char _command[HIO_ESP8266_TX_FIFO_BUFFER_SIZE];
    char _response[HIO_ESP8266_RX_FIFO_BUFFER_SIZE];
    uint8_t _message_buffer[HIO_ESP8266_TX_MAX_PACKET_SIZE];
    size_t _message_length;
    size_t _message_part_length;
    uint8_t _init_command_index;
    uint8_t _timeout_cnt;
    hio_esp8266_config _config;
    bool _ap_available;
    int _rssi;
};

//! @endcond

//! @brief Initialize ESP8266
//! @param[in] self Instance
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void hio_esp8266_init(hio_esp8266_t *self, hio_uart_channel_t uart_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_esp8266_set_event_handler(hio_esp8266_t *self, void (*event_handler)(hio_esp8266_t *, hio_esp8266_event_t, void *), void *event_param);

//! @brief Set station mode
//! @param[in] self Instance
//! @param[in] ssid SSID
//! @param[in] password Password

void hio_esp8266_set_station_mode(hio_esp8266_t *self, char *ssid, char *password);

//! @brief Enable SNTP and set time zone
//! @param[in] self Instance
//! @param[in] timezone Time zone, from -11 to 13

void hio_esp8266_set_sntp(hio_esp8266_t *self, int timezone);

//! @brief Enable SNTP and set configuration
//! @param[in] self Instance
//! @param[in] timezone Time zone, from -11 to 13
//! @param[in] sntp_server1 First SNTP server
//! @param[in] sntp_server2 Second SNTP server
//! @param[in] sntp_server3 Third SNTP server

void hio_esp8266_set_sntp_with_servers(hio_esp8266_t *self, int timezone, char *sntp_server1, char *sntp_server2, char *sntp_server3);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool hio_esp8266_is_ready(hio_esp8266_t *self);

//! @brief Enable ESP8266 and connect to WiFi
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_esp8266_connect(hio_esp8266_t *self);

//! @brief Disable ESP8266 and disconnect
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_esp8266_disconnect(hio_esp8266_t *self);

//! @brief Establish TCP Connection
//! @param[in] self Instance
//! @param[in] host Remote host
//! @param[in] port Remote port
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_esp8266_tcp_connect(hio_esp8266_t *self, const char *host, uint16_t port);

//! @brief Establish UDP Connection
//! @param[in] self Instance
//! @param[in] host Remote host
//! @param[in] port Remote port
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_esp8266_udp_connect(hio_esp8266_t *self, const char *host, uint16_t port);

//! @brief Establish SSL Connection
//! @param[in] self Instance
//! @param[in] host Remote host
//! @param[in] port Remote port
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_esp8266_ssl_connect(hio_esp8266_t *self, const char *host, uint16_t port);

//! @brief Send data
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_esp8266_send_data(hio_esp8266_t *self, const void *buffer, size_t length);

//! @brief Get length of the received message
//! @param[in] self Instance
//! @return length

uint32_t hio_esp8266_get_received_message_length(hio_esp8266_t *self);

//! @brief Get received message data
//! @param[in] self Instance
//! @param[in] buffer Destination buffer for received data
//! @param[in] buffer_size Size of the destination buffer
//! @return Length of the received message. Zero if the destination buffer is not big enough.

uint32_t hio_esp8266_get_received_message_data(hio_esp8266_t *self, uint8_t *buffer, uint32_t buffer_size);

//! @brief Check AP availability
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_esp8266_check_ap_availability(hio_esp8266_t *self);

//! @brief Get AP availability result
//! @param[in] self Instance
//! @param[in] available If AP is available
//! @param[in] rssi RSSI

void hio_esp8266_get_ap_availability(hio_esp8266_t *self, bool *available, int *rssi);

//! @brief Get SSID
//! @param[in] self Instance
//! @param[in] ssid Pointer to at least 64 character string

void hio_esp8266_get_ssid(hio_esp8266_t *self, char *ssid);

//! @brief Set SSID
//! @param[in] self Instance
//! @param[in] ssid Pointer to 64 character string

void hio_esp8266_set_ssid(hio_esp8266_t *self, char *ssid);

//! @brief Get password
//! @param[in] self Instance
//! @param[in] password Pointer to at least 64 character string

void hio_esp8266_get_password(hio_esp8266_t *self, char *password);

//! @brief Set password
//! @param[in] self Instance
//! @param[in] password Pointer to 64 character string

void hio_esp8266_set_password(hio_esp8266_t *self, char *password);

//! @}

#endif // _HIO_ESP8266_H
