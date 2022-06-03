#ifndef _TWR_CMWX1ZZABZ_H
#define _TWR_CMWX1ZZABZ_H

#include <twr_scheduler.h>
#include <twr_gpio.h>
#include <twr_uart.h>

//! @addtogroup twr_cmwx1zzabz twr_cmwx1zzabz
//! @brief Driver for CMWX1ZZABZ muRata LoRa modem
//! @{

//! @cond

#define TWR_CMWX1ZZABZ_TX_MAX_PACKET_SIZE 230

#define TWR_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE (TWR_CMWX1ZZABZ_TX_MAX_PACKET_SIZE + 25)
#define TWR_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE 220
#define TWR_CMWX1ZZABZ_CUSTOM_COMMAND_BUFFER_SIZE 32
#define TWR_CMWX1ZZABZ_FW_VERSION_BUFFER_SIZE 64


//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    TWR_CMWX1ZZABZ_EVENT_READY = 0,

    //! @brief Error event
    TWR_CMWX1ZZABZ_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    TWR_CMWX1ZZABZ_EVENT_SEND_MESSAGE_START = 2,

    //! @brief RF frame transmission finished event
    TWR_CMWX1ZZABZ_EVENT_SEND_MESSAGE_DONE = 3,

    //! @brief Configuration save done
    TWR_CMWX1ZZABZ_EVENT_CONFIG_SAVE_DONE = 4,

    //! @brief OTAA join success
    TWR_CMWX1ZZABZ_EVENT_JOIN_SUCCESS = 5,

    //! @brief OTAA join error
    TWR_CMWX1ZZABZ_EVENT_JOIN_ERROR = 6,

    //! @brief Received message
    TWR_CMWX1ZZABZ_EVENT_MESSAGE_RECEIVED = 7,

    //! @brief Retransmission of the confirmed message
    TWR_CMWX1ZZABZ_EVENT_MESSAGE_RETRANSMISSION = 8,

    //! @brief Sent message confirmed
    TWR_CMWX1ZZABZ_EVENT_MESSAGE_CONFIRMED = 9,

    //! @brief Sent message not confirmed
    TWR_CMWX1ZZABZ_EVENT_MESSAGE_NOT_CONFIRMED = 10,

    //! @brief RF quality response
    TWR_CMWX1ZZABZ_EVENT_RFQ = 11,

    //! @brief Frame counter response
    TWR_CMWX1ZZABZ_EVENT_FRAME_COUNTER = 12,

    TWR_CMWX1ZZABZ_EVENT_LINK_CHECK_OK = 13,

    TWR_CMWX1ZZABZ_EVENT_LINK_CHECK_NOK = 14,

    TWR_CMWX1ZZABZ_EVENT_MODEM_FACTORY_RESET = 15,

    TWR_CMWX1ZZABZ_EVENT_CUSTOM_AT = 16

} twr_cmwx1zzabz_event_t;

//! @brief CMWX1ZZABZ instance

typedef struct twr_cmwx1zzabz_t twr_cmwx1zzabz_t;

//! @brief LoRa mode ABP/OTAA

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_MODE_ABP = 0,
    TWR_CMWX1ZZABZ_CONFIG_MODE_OTAA = 1

} twr_cmwx1zzabz_config_mode_t;

//! @brief Frequency modes and standards

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_BAND_AS923 = 0,
    TWR_CMWX1ZZABZ_CONFIG_BAND_AU915 = 1,
    TWR_CMWX1ZZABZ_CONFIG_BAND_EU868 = 5,
    TWR_CMWX1ZZABZ_CONFIG_BAND_KR920 = 6,
    TWR_CMWX1ZZABZ_CONFIG_BAND_IN865 = 7,
    TWR_CMWX1ZZABZ_CONFIG_BAND_US915 = 8

} twr_cmwx1zzabz_config_band_t;

//! @brief LoRa device class A or C

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_CLASS_A = 0,
    TWR_CMWX1ZZABZ_CONFIG_CLASS_C = 2

} twr_cmwx1zzabz_config_class_t;

//! @brief Datarate for AS923

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF12_125KHZ = 0,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF11_125KHZ = 1,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF10_125KHZ = 2,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF9_125KHZ = 3,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF8_125KHZ = 4,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF7_125KHZ = 5,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF7_250KHZ = 6,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AS923_FSK_50KBPS = 7,

} twr_cmwx1zzabz_config_datarate_as923_t;

//! @brief Datarate for AU915

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF12_125KHZ = 0,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF11_125KHZ = 1,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF10_125KHZ = 2,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF9_125KHZ = 3,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF8_125KHZ = 4,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF7_125KHZ = 5,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF8_500KHZ = 6,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF12_500KHZ = 8,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF11_500KHZ = 9,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF10_500KHZ = 10,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF9_500KHZ = 11,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF8_500KHZ_2 = 12,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF7_500KHZ = 13

} twr_cmwx1zzabz_config_datarate_au915_t;


//! @brief Datarate for EU868

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF12_125KHZ = 0,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF11_125KHZ = 1,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF10_125KHZ = 2,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF9_125KHZ = 3,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF8_125KHZ = 4,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF7_125KHZ = 5,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF7_250KHZ = 6,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_EU868_FSK_50KBPS = 7

} twr_cmwx1zzabz_config_datarate_eu868_t;

//! @brief Datarate for KR920

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF12_125KHZ = 0,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF11_125KHZ = 1,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF10_125KHZ = 2,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF9_125KHZ = 3,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF8_125KHZ = 4,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF7_125KHZ = 5

} twr_cmwx1zzabz_config_datarate_kr920_t;

//! @brief Datarate for US915

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF10_125KHZ = 0,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF9_125KHZ = 1,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF8_125KHZ = 2,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF7_125KHZ = 3,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF8_500KHZ = 4,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF12_500KHZ = 8,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF11_500KHZ = 9,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF10_500KHZ = 10,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF9_500KHZ = 11,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF8_500KHZ_2 = 12,
    TWR_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF7_500KHZ = 13

} twr_cmwx1zzabz_config_datarate_us915_t;

//! @cond

typedef enum
{
    TWR_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR = 0,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI = 1,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_APPEUI = 2,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY = 3,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY = 4,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_APPKEY = 5,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_BAND = 6,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_MODE = 7,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_CLASS = 8,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_RX2 = 9,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_NWK = 10,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_ADAPTIVE_DATARATE = 11,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_DATARATE = 12,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_REP = 13,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_RTYNUM = 14,
    TWR_CMWX1ZZABZ_CONFIG_INDEX_LAST_ITEM

} twr_cmwx1zzabz_config_index_t;

typedef enum
{
    TWR_CMWX1ZZABZ_STATE_READY,
    TWR_CMWX1ZZABZ_STATE_ERROR,
    TWR_CMWX1ZZABZ_STATE_INITIALIZE,
    TWR_CMWX1ZZABZ_STATE_INITIALIZE_AT_RESPONSE,
    TWR_CMWX1ZZABZ_STATE_IDLE,
    TWR_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND,
    TWR_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_RESPONSE,

    TWR_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND,
    TWR_CMWX1ZZABZ_STATE_CONFIG_SAVE_RESPONSE,

    TWR_CMWX1ZZABZ_STATE_SEND_MESSAGE_COMMAND,
    TWR_CMWX1ZZABZ_STATE_SEND_MESSAGE_CONFIRMED_COMMAND,
    TWR_CMWX1ZZABZ_STATE_SEND_MESSAGE_RESPONSE,

    TWR_CMWX1ZZABZ_STATE_JOIN_SEND,
    TWR_CMWX1ZZABZ_STATE_JOIN_RESPONSE,

    TWR_CMWX1ZZABZ_STATE_CUSTOM_COMMAND_SEND,
    TWR_CMWX1ZZABZ_STATE_CUSTOM_COMMAND_RESPONSE,

    TWR_CMWX1ZZABZ_STATE_LINK_CHECK_SEND,
    TWR_CMWX1ZZABZ_STATE_LINK_CHECK_RESPONSE,
    TWR_CMWX1ZZABZ_STATE_LINK_CHECK_RESPONSE_ANS,

    TWR_CMWX1ZZABZ_STATE_RECEIVE,
    TWR_CMWX1ZZABZ_STATE_RECOVER_BAUDRATE_UART,
    TWR_CMWX1ZZABZ_STATE_RECOVER_BAUDRATE_REBOOT

} twr_cmwx1zzabz_state_t;

typedef struct
{
    twr_cmwx1zzabz_config_band_t band;
    twr_cmwx1zzabz_config_mode_t mode;
    twr_cmwx1zzabz_config_class_t class;
    char devaddr[8 + 1];
    char deveui[16 + 1];
    char appeui[16 + 1];
    char nwkskey[32 + 1];
    char appskey[32 + 1];
    char appkey[32 + 1];
    uint32_t rx2_frequency;
    uint8_t rx2_datarate;
    uint8_t nwk_public;
    bool adaptive_datarate;
    uint8_t datarate;
    uint8_t repetition_unconfirmed;
    uint8_t repetition_confirmed;

} twr_cmwx1zzabz_config;

struct twr_cmwx1zzabz_t
{
    twr_scheduler_task_id_t _task_id;
    twr_uart_channel_t _uart_channel;
    twr_cmwx1zzabz_state_t _state;
    twr_cmwx1zzabz_state_t _state_after_sleep;
    twr_fifo_t _tx_fifo;
    twr_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[TWR_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[TWR_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(twr_cmwx1zzabz_t *, twr_cmwx1zzabz_event_t, void *);
    void *_event_param;
    char _command[TWR_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE];
    char _response[TWR_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE];
    uint8_t _response_length;
    uint8_t _message_buffer[TWR_CMWX1ZZABZ_TX_MAX_PACKET_SIZE];
    size_t _message_length;
    uint8_t _message_port;
    uint8_t _init_command_index;
    uint8_t _save_command_index;
    bool _save_flag;
    uint32_t _save_config_mask;
    twr_cmwx1zzabz_config _config;
    bool _join_command;
    bool _link_check_command;
    bool _custom_command;
    char _custom_command_buf[TWR_CMWX1ZZABZ_CUSTOM_COMMAND_BUFFER_SIZE];
    uint8_t _tx_port;
    bool _debug;

    char _fw_version[TWR_CMWX1ZZABZ_FW_VERSION_BUFFER_SIZE];

    int32_t _cmd_rfq_rssi;
    int32_t _cmd_rfq_snr;

    uint32_t _cmd_frmcnt_uplink;
    uint32_t _cmd_frmcnt_downlink;

    uint8_t _cmd_link_check_margin;
    uint8_t _cmd_link_check_gwcnt;

    twr_tick_t _timeout;
};

//! @endcond

//! @brief Initialize CMWX1ZZABZ
//! @param[in] self Instance
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void twr_cmwx1zzabz_init(twr_cmwx1zzabz_t *self, twr_uart_channel_t uart_channel);

//! @brief Deinitialize CMWX1ZZABZ
//! @param[in] self Instance

void twr_cmwx1zzabz_deinit(twr_cmwx1zzabz_t *self);

//! @brief Reboot and initialize CMWX1ZZABZ
//! @param[in] self Instance

void twr_cmwx1zzabz_reboot(twr_cmwx1zzabz_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_cmwx1zzabz_set_event_handler(twr_cmwx1zzabz_t *self, void (*event_handler)(twr_cmwx1zzabz_t *, twr_cmwx1zzabz_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool twr_cmwx1zzabz_is_ready(twr_cmwx1zzabz_t *self);

//! @brief Send LoRa message
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 51 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool twr_cmwx1zzabz_send_message(twr_cmwx1zzabz_t *self, const void *buffer, size_t length);

//! @brief Send LoRa confirmed message
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 51 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool twr_cmwx1zzabz_send_message_confirmed(twr_cmwx1zzabz_t *self, const void *buffer, size_t length);

//! @brief Set DEVADDR
//! @param[in] self Instance
//! @param[in] devaddr Pointer to 8 character string

void twr_cmwx1zzabz_set_devaddr(twr_cmwx1zzabz_t *self, char *devaddr);

//! @brief Get DEVADDR
//! @param[in] self Instance
//! @param[in] devaddr Pointer to at least 8+1 character string

void twr_cmwx1zzabz_get_devaddr(twr_cmwx1zzabz_t *self, char *devaddr);

//! @brief Set DEVEUI
//! @param[in] self Instance
//! @param[in] deveui Pointer to 16 character string. WARNING, this value should not be changed as it is unique number programmed in the factory!

void twr_cmwx1zzabz_set_deveui(twr_cmwx1zzabz_t *self, char *deveui);

//! @brief Get DEVEUI
//! @param[in] self Instance
//! @param[in] deveui Pointer to at least 16+1 character string.

void twr_cmwx1zzabz_get_deveui(twr_cmwx1zzabz_t *self, char *deveui);

//! @brief Set APPEUI
//! @param[in] self Instance
//! @param[in] appeui Pointer to 16 character string

void twr_cmwx1zzabz_set_appeui(twr_cmwx1zzabz_t *self, char *appeui);

//! @brief Get APPEUI
//! @param[in] self Instance
//! @param[in] appeui Pointer to at least 16+1 character string

void twr_cmwx1zzabz_get_appeui(twr_cmwx1zzabz_t *self, char *appeui);

//! @brief Set NWKSKEY
//! @param[in] self Instance
//! @param[in] nwkskey Pointer to 32 character string

void twr_cmwx1zzabz_set_nwkskey(twr_cmwx1zzabz_t *self, char *nwkskey);

//! @brief Set NWKSKEY
//! @param[in] self Instance
//! @param[in] nwkskey Pointer to at least 32+1 character string

void twr_cmwx1zzabz_get_nwkskey(twr_cmwx1zzabz_t *self, char *nwkskey);

//! @brief Set APPSKEY
//! @param[in] self Instance
//! @param[in] appskey Pointer to 32 character string

void twr_cmwx1zzabz_set_appskey(twr_cmwx1zzabz_t *self, char *appskey);

//! @brief Get APPSKEY
//! @param[in] self Instance
//! @param[in] appskey Pointer to at least 32+1 character string

void twr_cmwx1zzabz_get_appskey(twr_cmwx1zzabz_t *self, char *appskey);

//! @brief Set APPKEY
//! @param[in] self Instance
//! @param[in] appkey Pointer to 32 character string

void twr_cmwx1zzabz_set_appkey(twr_cmwx1zzabz_t *self, char *appkey);

//! @brief Get APPKEY
//! @param[in] self Instance
//! @param[in] appkey Pointer to at least 32+1 character string

void twr_cmwx1zzabz_get_appkey(twr_cmwx1zzabz_t *self, char *appkey);

//! @brief Set BAND
//! @param[in] self Instance
//! @param[in] band Set correct frequency and modulation for EU, US and other countries

void twr_cmwx1zzabz_set_band(twr_cmwx1zzabz_t *self, twr_cmwx1zzabz_config_band_t band);

//! @brief Get BAND
//! @param[in] self Instance
//! @return Band value

twr_cmwx1zzabz_config_band_t twr_cmwx1zzabz_get_band(twr_cmwx1zzabz_t *self);

//! @brief Set ABP/OTAA mode
//! @param[in] self Instance
//! @param[in] mode ABP or OTAA mode

void twr_cmwx1zzabz_set_mode(twr_cmwx1zzabz_t *self, twr_cmwx1zzabz_config_mode_t mode);

//! @brief Get ABP/OTAA mode
//! @param[in] self Instance
//! @return ABP or OTAA mode

twr_cmwx1zzabz_config_mode_t twr_cmwx1zzabz_get_mode(twr_cmwx1zzabz_t *self);

//! @brief Set device class
//! @param[in] self Instance
//! @param[in] class Supported are Class A and C

void twr_cmwx1zzabz_set_class(twr_cmwx1zzabz_t *self, twr_cmwx1zzabz_config_class_t class);

//! @brief Get device class
//! @param[in] self Instance
//! @return Class A or C

twr_cmwx1zzabz_config_class_t twr_cmwx1zzabz_get_class(twr_cmwx1zzabz_t *self);

//! @brief Start LoRa OTAA join procedure
//! @param[in] self Instance
//! @note The output of the join is handled by callback events
//! @see twr_cmwx1zzabz_event_t

void twr_cmwx1zzabz_join(twr_cmwx1zzabz_t *self);

//! @brief Get port of the received message
//! @param[in] self Instance
//! @return port

uint8_t twr_cmwx1zzabz_get_received_message_port(twr_cmwx1zzabz_t *self);

//! @brief Get length of the received message
//! @param[in] self Instance
//! @return length

uint32_t twr_cmwx1zzabz_get_received_message_length(twr_cmwx1zzabz_t *self);

//! @brief Get received message data
//! @param[in] self Instance
//! @param[in] buffer Destination buffer for received data
//! @param[in] buffer_size Size of the destination buffer
//! @return Length of the received message. Zero if the destination buffer is not big enough.

uint32_t twr_cmwx1zzabz_get_received_message_data(twr_cmwx1zzabz_t *self, uint8_t *buffer, uint32_t buffer_size);

//! @brief Set the port for the transmission of the messages
//! @param[in] self Instance
//! @param[in] port Port

void twr_cmwx1zzabz_set_port(twr_cmwx1zzabz_t *self, uint8_t port);

//! @brief Get the port for the transmission of the messages
//! @param[in] self Instance
//! @return Port

uint8_t twr_cmwx1zzabz_get_port(twr_cmwx1zzabz_t *self);

//! @brief Set the frequency and datarate for RX2 receive window
//! @param[in] self Instance
//! @param[in] frequency Frequency in Hz
//! @param[in] datarate Datarate

void twr_cmwx1zzabz_set_rx2(twr_cmwx1zzabz_t *self, uint32_t frequency, uint8_t datarate);

//! @brief Get the frequency and datarate for RX2 receive window
//! @param[in] self Instance
//! @param[in] frequency Pointer to save frequency in Hz
//! @param[in] datarate Pointer to save datarate

void twr_cmwx1zzabz_get_rx2(twr_cmwx1zzabz_t *self, uint32_t *frequency, uint8_t *datarate);

//! @brief Set the configuration enabling public networks
//! @param[in] self Instance
//! @param[in] public enable public networks

void twr_cmwx1zzabz_set_nwk_public(twr_cmwx1zzabz_t *self, uint8_t public);

//! @brief Get the configuration if public networks are enabled
//! @param[in] self Instance
//! @return public networks enabled

uint8_t twr_cmwx1zzabz_get_nwk_public(twr_cmwx1zzabz_t *self);

//! @brief Set the configuration adaptive data rate
//! @param[in] self Instance
//! @param[in] public enable or disable adaptive data rate

void twr_cmwx1zzabz_set_adaptive_datarate(twr_cmwx1zzabz_t *self, bool enable);

//! @brief Get the configuration if adaptive data rate are enabled
//! @param[in] self Instance
//! @return public adaptive data rate enabled

bool twr_cmwx1zzabz_get_adaptive_datarate(twr_cmwx1zzabz_t *self);

//! @brief Set the configuration of datarate
//! @param[in] self Instance
//! @param[in] datarate Datarate

void twr_cmwx1zzabz_set_datarate(twr_cmwx1zzabz_t *self, uint8_t datarate);

//! @brief Get the configuration of datarate
//! @param[in] self Instance
//! @return datarate Datarate (see the enums)

uint8_t twr_cmwx1zzabz_get_datarate(twr_cmwx1zzabz_t *self);

//! @brief Set debugging flag which prints modem communication to twr_log
//! @param[in] self Instance
//! @param[in] debug Boolean value

void twr_cmwx1zzabz_set_debug(twr_cmwx1zzabz_t *self, bool debug);

//! @brief Get pointer to string containg last sent command
//! @param[in] self Instance
//! @return Buffer char* containing last modem response

char *twr_cmwx1zzabz_get_error_command(twr_cmwx1zzabz_t *self);

//! @brief Get pointer to string containg response on last sent command
//! @param[in] self Instance
//! @return Buffer char* containing last modem response

char *twr_cmwx1zzabz_get_error_response(twr_cmwx1zzabz_t *self);

//! @brief Send custom AT command to LoRa Module
//! @param[in] self Instance
//! @param[in] at_command AT command

bool twr_cmwx1zzabz_custom_at(twr_cmwx1zzabz_t *self, char *at_command);

//! @brief Request RF quality of the last received packet (JOIN, LNPARAM, confirmed message)
//! @param[in] self Instance

bool twr_cmwx1zzabz_rfq(twr_cmwx1zzabz_t *self);

//! @brief Get RF quality values in callback
//! @param[in] self Instance
//! @param[out] rssi RSSI
//! @param[out] snr SNR
//! @return true

bool twr_cmwx1zzabz_get_rfq(twr_cmwx1zzabz_t *self, int32_t *rssi, int32_t *snr);

//! @brief Request frame counter value
//! @param[in] self Instance

bool twr_cmwx1zzabz_frame_counter(twr_cmwx1zzabz_t *self);

//! @brief Get frame counter value
//! @param[in] self Instance
//! @param[out] uplink Uplink frame counter
//! @param[out] downlink Downlink frame counter
//! @return true

bool twr_cmwx1zzabz_get_frame_counter(twr_cmwx1zzabz_t *self, uint32_t *uplink, uint32_t *downlink);

//! @brief Request send of link check packet
//! @param[in] self Instance

bool twr_cmwx1zzabz_link_check(twr_cmwx1zzabz_t *self);

//! @brief Get link check values
//! @param[in] self Instance
//! @param[out] margin Margin id dBm
//! @param[out] gateway_count Number of gateways that received link check packet
//! @return true

bool twr_cmwx1zzabz_get_link_check(twr_cmwx1zzabz_t *self, uint8_t *margin, uint8_t *gateway_count);

//! @brief Send factory reset command to LoRa Module
//! @param[in] self Instance

bool twr_cmwx1zzabz_factory_reset(twr_cmwx1zzabz_t *self);

//! @brief Get firmware version string
//! @param[in] self Instance
//! @return Pointer to the string containing version

char *twr_cmwx1zzabz_get_fw_version(twr_cmwx1zzabz_t *self);

//! @brief Set number of transmissions of unconfirmed message
//! @param[in] self Instance
//! @param[in] repeat Number of transmissions

void twr_cmwx1zzabz_set_repeat_unconfirmed(twr_cmwx1zzabz_t *self, uint8_t repeat);

//! @brief Get number of transmissions of unconfirmed message
//! @param[in] self Instance
//! @return Number of transmissions

uint8_t twr_cmwx1zzabz_get_repeat_unconfirmed(twr_cmwx1zzabz_t *self);

//! @brief Set number of transmissions of confirmed message
//! @param[in] self Instance
//! @param[in] repeat Number of transmissions

void twr_cmwx1zzabz_set_repeat_confirmed(twr_cmwx1zzabz_t *self, uint8_t repeat);

//! @brief Get number of transmissions of confirmed message
//! @param[in] self Instance
//! @return Number of transmissions

uint8_t twr_cmwx1zzabz_get_repeat_confirmed(twr_cmwx1zzabz_t *self);

//! @}

#endif // _TWR_CMWX1ZZABZ_H
