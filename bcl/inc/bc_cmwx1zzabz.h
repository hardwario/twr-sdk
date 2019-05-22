#ifndef _BC_CMWX1ZZABZ_H
#define _BC_CMWX1ZZABZ_H

#include <bc_scheduler.h>
#include <bc_gpio.h>
#include <bc_uart.h>

//! @addtogroup bc_cmwx1zzabz bc_cmwx1zzabz
//! @brief Driver for CMWX1ZZABZ muRata LoRa modem
//! @{

//! @cond

#define BC_CMWX1ZZABZ_TX_MAX_PACKET_SIZE 230

#define BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE (BC_CMWX1ZZABZ_TX_MAX_PACKET_SIZE + 25)
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
    BC_CMWX1ZZABZ_EVENT_SEND_MESSAGE_START = 2,

    //! @brief RF frame transmission finished event
    BC_CMWX1ZZABZ_EVENT_SEND_MESSAGE_DONE = 3,

    //! @brief Configuration save done
    BC_CMWX1ZZABZ_EVENT_CONFIG_SAVE_DONE = 4,

    //! @brief OTAA join success
    BC_CMWX1ZZABZ_EVENT_JOIN_SUCCESS = 5,

    //! @brief OTAA join error
    BC_CMWX1ZZABZ_EVENT_JOIN_ERROR = 6,

    //! @brief Received message
    BC_CMWX1ZZABZ_EVENT_MESSAGE_RECEIVED = 7,

    //! @brief Retransmission of the confirmed message
    BC_CMWX1ZZABZ_EVENT_MESSAGE_RETRANSMISSION = 8,

    //! @brief Sent message confirmed
    BC_CMWX1ZZABZ_EVENT_MESSAGE_CONFIRMED = 9,

    //! @brief Sent message not confirmed
    BC_CMWX1ZZABZ_EVENT_MESSAGE_NOT_CONFIRMED = 10

} bc_cmwx1zzabz_event_t;

//! @brief CMWX1ZZABZ instance

typedef struct bc_cmwx1zzabz_t bc_cmwx1zzabz_t;

//! @brief LoRa mode ABP/OTAA

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_MODE_ABP = 0,
    BC_CMWX1ZZABZ_CONFIG_MODE_OTAA = 1

} bc_cmwx1zzabz_config_mode_t;

//! @brief Frequency modes and standards

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_BAND_AS923 = 0,
    BC_CMWX1ZZABZ_CONFIG_BAND_AU915 = 1,
    BC_CMWX1ZZABZ_CONFIG_BAND_EU868 = 5,
    BC_CMWX1ZZABZ_CONFIG_BAND_KR920 = 6,
    BC_CMWX1ZZABZ_CONFIG_BAND_IN865 = 7,
    BC_CMWX1ZZABZ_CONFIG_BAND_US915 = 8

} bc_cmwx1zzabz_config_band_t;

//! @brief LoRa device class A or C

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_CLASS_A = 0,
    BC_CMWX1ZZABZ_CONFIG_CLASS_C = 2

} bc_cmwx1zzabz_config_class_t;

//! @brief Datarate for AS923

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF12_125KHZ = 0,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF11_125KHZ = 1,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF10_125KHZ = 2,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF9_125KHZ = 3,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF8_125KHZ = 4,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF7_125KHZ = 5,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_SF7_250KHZ = 6,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AS923_FSK_50KBPS = 7,

} bc_cmwx1zzabz_config_datarate_as923_t;

//! @brief Datarate for AU915

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF12_125KHZ = 0,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF11_125KHZ = 1,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF10_125KHZ = 2,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF9_125KHZ = 3,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF8_125KHZ = 4,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF7_125KHZ = 5,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF8_500KHZ = 6,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF12_500KHZ = 8,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF11_500KHZ = 9,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF10_500KHZ = 10,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF9_500KHZ = 11,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF8_500KHZ_2 = 12,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_AU915_SF7_500KHZ = 13

} bc_cmwx1zzabz_config_datarate_au915_t;


//! @brief Datarate for EU868

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF12_125KHZ = 0,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF11_125KHZ = 1,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF10_125KHZ = 2,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF9_125KHZ = 3,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF8_125KHZ = 4,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF7_125KHZ = 5,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_SF7_250KHZ = 6,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_EU868_FSK_50KBPS = 7

} bc_cmwx1zzabz_config_datarate_eu868_t;

//! @brief Datarate for KR920

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF12_125KHZ = 0,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF11_125KHZ = 1,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF10_125KHZ = 2,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF9_125KHZ = 3,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF8_125KHZ = 4,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_KR920_SF7_125KHZ = 5

} bc_cmwx1zzabz_config_datarate_kr920_t;

//! @brief Datarate for US915

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF10_125KHZ = 0,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF9_125KHZ = 1,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF8_125KHZ = 2,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF7_125KHZ = 3,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF8_500KHZ = 4,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF12_500KHZ = 8,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF11_500KHZ = 9,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF10_500KHZ = 10,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF9_500KHZ = 11,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF8_500KHZ_2 = 12,
    BC_CMWX1ZZABZ_CONFIG_DATARATE_US915_SF7_500KHZ = 13

} bc_cmwx1zzabz_config_datarate_us915_t;

//! @cond

typedef enum
{
    BC_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR = 0,
    BC_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI = 1,
    BC_CMWX1ZZABZ_CONFIG_INDEX_APPEUI = 2,
    BC_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY = 3,
    BC_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY = 4,
    BC_CMWX1ZZABZ_CONFIG_INDEX_APPKEY = 5,
    BC_CMWX1ZZABZ_CONFIG_INDEX_BAND = 6,
    BC_CMWX1ZZABZ_CONFIG_INDEX_MODE = 7,
    BC_CMWX1ZZABZ_CONFIG_INDEX_CLASS = 8,
    BC_CMWX1ZZABZ_CONFIG_INDEX_RX2 = 9,
    BC_CMWX1ZZABZ_CONFIG_INDEX_NWK = 10,
    BC_CMWX1ZZABZ_CONFIG_INDEX_DATARATE = 11,
    BC_CMWX1ZZABZ_CONFIG_INDEX_LAST_ITEM

} bc_cmwx1zzabz_config_index_t;

typedef enum
{
    BC_CMWX1ZZABZ_STATE_READY = 0,
    BC_CMWX1ZZABZ_STATE_ERROR = 1,
    BC_CMWX1ZZABZ_STATE_INITIALIZE = 2,
    BC_CMWX1ZZABZ_STATE_IDLE = 3,
    BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND = 5,
    BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_RESPONSE = 6,

    BC_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND = 7,
    BC_CMWX1ZZABZ_STATE_CONFIG_SAVE_RESPONSE = 8,

    BC_CMWX1ZZABZ_STATE_SEND_MESSAGE_COMMAND = 9,
    BC_CMWX1ZZABZ_STATE_SEND_MESSAGE_CONFIRMED_COMMAND = 10,
    BC_CMWX1ZZABZ_STATE_SEND_MESSAGE_RESPONSE = 11,

    BC_CMWX1ZZABZ_STATE_JOIN_SEND = 12,
    BC_CMWX1ZZABZ_STATE_JOIN_RESPONSE = 13,

    BC_CMWX1ZZABZ_STATE_RECEIVE = 14

} bc_cmwx1zzabz_state_t;

typedef struct
{
    bc_cmwx1zzabz_config_band_t band;
    bc_cmwx1zzabz_config_mode_t mode;
    bc_cmwx1zzabz_config_class_t class;
    char devaddr[8 + 1];
    char deveui[16 + 1];
    char appeui[16 + 1];
    char nwkskey[32 + 1];
    char appskey[32 + 1];
    char appkey[32 + 1];
    uint32_t rx2_frequency;
    uint8_t rx2_datarate;
    uint8_t nwk_public;
    uint8_t datarate;

} bc_cmwx1zzabz_config;

struct bc_cmwx1zzabz_t
{
    bc_scheduler_task_id_t _task_id;
    bc_uart_channel_t _uart_channel;
    bc_cmwx1zzabz_state_t _state;
    bc_cmwx1zzabz_state_t _state_after_sleep;
    bc_fifo_t _tx_fifo;
    bc_fifo_t _rx_fifo;
    uint8_t _tx_fifo_buffer[BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE];
    uint8_t _rx_fifo_buffer[BC_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE];
    void (*_event_handler)(bc_cmwx1zzabz_t *, bc_cmwx1zzabz_event_t, void *);
    void *_event_param;
    char _command[BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE];
    char _response[BC_CMWX1ZZABZ_RX_FIFO_BUFFER_SIZE];
    uint8_t _message_buffer[BC_CMWX1ZZABZ_TX_MAX_PACKET_SIZE];
    size_t _message_length;
    uint8_t _message_port;
    uint8_t _init_command_index;
    uint8_t _save_command_index;
    bool _save_flag;
    uint32_t _save_config_mask;
    bc_cmwx1zzabz_config _config;
    bool _join_command;
    uint8_t _tx_port;
};

//! @endcond

//! @brief Initialize CMWX1ZZABZ
//! @param[in] self Instance
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

//! @brief Send LoRa message
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 51 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool bc_cmwx1zzabz_send_message(bc_cmwx1zzabz_t *self, const void *buffer, size_t length);

//! @brief Send LoRa confirmed message
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 51 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool bc_cmwx1zzabz_send_message_confirmed(bc_cmwx1zzabz_t *self, const void *buffer, size_t length);

//! @brief Set DEVADDR
//! @param[in] self Instance
//! @param[in] devaddr Pointer to 8 character string

void bc_cmwx1zzabz_set_devaddr(bc_cmwx1zzabz_t *self, char *devaddr);

//! @brief Get DEVADDR
//! @param[in] self Instance
//! @param[in] devaddr Pointer to at least 8+1 character string

void bc_cmwx1zzabz_get_devaddr(bc_cmwx1zzabz_t *self, char *devaddr);

//! @brief Set DEVEUI
//! @param[in] self Instance
//! @param[in] deveui Pointer to 16 character string. WARNING, this value should not be changed as it is unique number programmed in the factory!

void bc_cmwx1zzabz_set_deveui(bc_cmwx1zzabz_t *self, char *deveui);

//! @brief Get DEVEUI
//! @param[in] self Instance
//! @param[in] deveui Pointer to at least 16+1 character string.

void bc_cmwx1zzabz_get_deveui(bc_cmwx1zzabz_t *self, char *deveui);

//! @brief Set APPEUI
//! @param[in] self Instance
//! @param[in] appeui Pointer to 16 character string

void bc_cmwx1zzabz_set_appeui(bc_cmwx1zzabz_t *self, char *appeui);

//! @brief Get APPEUI
//! @param[in] self Instance
//! @param[in] appeui Pointer to at least 16+1 character string

void bc_cmwx1zzabz_get_appeui(bc_cmwx1zzabz_t *self, char *appeui);

//! @brief Set NWKSKEY
//! @param[in] self Instance
//! @param[in] nwkskey Pointer to 32 character string

void bc_cmwx1zzabz_set_nwkskey(bc_cmwx1zzabz_t *self, char *nwkskey);

//! @brief Set NWKSKEY
//! @param[in] self Instance
//! @param[in] nwkskey Pointer to at least 32+1 character string

void bc_cmwx1zzabz_get_nwkskey(bc_cmwx1zzabz_t *self, char *nwkskey);

//! @brief Set APPSKEY
//! @param[in] self Instance
//! @param[in] appskey Pointer to 32 character string

void bc_cmwx1zzabz_set_appskey(bc_cmwx1zzabz_t *self, char *appskey);

//! @brief Get APPSKEY
//! @param[in] self Instance
//! @param[in] appskey Pointer to at least 32+1 character string

void bc_cmwx1zzabz_get_appskey(bc_cmwx1zzabz_t *self, char *appskey);

//! @brief Set APPKEY
//! @param[in] self Instance
//! @param[in] appkey Pointer to 32 character string

void bc_cmwx1zzabz_set_appkey(bc_cmwx1zzabz_t *self, char *appkey);

//! @brief Get APPKEY
//! @param[in] self Instance
//! @param[in] appkey Pointer to at least 32+1 character string

void bc_cmwx1zzabz_get_appkey(bc_cmwx1zzabz_t *self, char *appkey);

//! @brief Set BAND
//! @param[in] self Instance
//! @param[in] band Set correct frequency and modulation for EU, US and other countries

void bc_cmwx1zzabz_set_band(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_config_band_t band);

//! @brief Get BAND
//! @param[in] self Instance
//! @return Band value

bc_cmwx1zzabz_config_band_t bc_cmwx1zzabz_get_band(bc_cmwx1zzabz_t *self);

//! @brief Set ABP/OTAA mode
//! @param[in] self Instance
//! @param[in] mode ABP or OTAA mode

void bc_cmwx1zzabz_set_mode(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_config_mode_t mode);

//! @brief Get ABP/OTAA mode
//! @param[in] self Instance
//! @return ABP or OTAA mode

bc_cmwx1zzabz_config_mode_t bc_cmwx1zzabz_get_mode(bc_cmwx1zzabz_t *self);

//! @brief Set device class
//! @param[in] self Instance
//! @param[in] class Supported are Class A and C

void bc_cmwx1zzabz_set_class(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_config_class_t class);

//! @brief Get device class
//! @param[in] self Instance
//! @return Class A or C

bc_cmwx1zzabz_config_class_t bc_cmwx1zzabz_get_class(bc_cmwx1zzabz_t *self);

//! @brief Start LoRa OTAA join procedure
//! @param[in] self Instance
//! @note The output of the join is handled by callback events
//! @see bc_cmwx1zzabz_event_t

void bc_cmwx1zzabz_join(bc_cmwx1zzabz_t *self);

//! @brief Get port of the received message
//! @param[in] self Instance
//! @return port

uint8_t bc_cmwx1zzabz_get_received_message_port(bc_cmwx1zzabz_t *self);

//! @brief Get length of the received message
//! @param[in] self Instance
//! @return length

uint32_t bc_cmwx1zzabz_get_received_message_length(bc_cmwx1zzabz_t *self);

//! @brief Get received message data
//! @param[in] self Instance
//! @param[in] buffer Destination buffer for received data
//! @param[in] buffer_size Size of the destination buffer
//! @return Length of the received message. Zero if the destination buffer is not big enough.

uint32_t bc_cmwx1zzabz_get_received_message_data(bc_cmwx1zzabz_t *self, uint8_t *buffer, uint32_t buffer_size);

//! @brief Set the port for the transmission of the messages
//! @param[in] self Instance
//! @param[in] port Port

void bc_cmwx1zzabz_set_port(bc_cmwx1zzabz_t *self, uint8_t port);

//! @brief Get the port for the transmission of the messages
//! @param[in] self Instance
//! @return Port

uint8_t bc_cmwx1zzabz_get_port(bc_cmwx1zzabz_t *self);

//! @brief Set the frequency and datarate for RX2 receive window
//! @param[in] self Instance
//! @param[in] frequency Frequency in Hz
//! @param[in] datarate Datarate

void bc_cmwx1zzabz_set_rx2(bc_cmwx1zzabz_t *self, uint32_t frequency, uint8_t datarate);

//! @brief Get the frequency and datarate for RX2 receive window
//! @param[in] self Instance
//! @param[in] frequency Pointer to save frequency in Hz
//! @param[in] datarate Pointer to save datarate

void bc_cmwx1zzabz_get_rx2(bc_cmwx1zzabz_t *self, uint32_t *frequency, uint8_t *datarate);

//! @brief Set the configuration enabling public networks
//! @param[in] self Instance
//! @param[in] public enable public networks

void bc_cmwx1zzabz_set_nwk_public(bc_cmwx1zzabz_t *self, uint8_t public);

//! @brief Get the configuration if public networks are enabled
//! @param[in] self Instance
//! @return public networks enabled

uint8_t bc_cmwx1zzabz_get_nwk_public(bc_cmwx1zzabz_t *self);

//! @brief Set the configuration of datarate
//! @param[in] self Instance
//! @param[in] datarate Datarate

void bc_cmwx1zzabz_set_datarate(bc_cmwx1zzabz_t *self, uint8_t datarate);

//! @brief Get the configuration of datarate
//! @param[in] self Instance
//! @return datarate Datarate (see the enums)

uint8_t bc_cmwx1zzabz_get_datarate(bc_cmwx1zzabz_t *self);

//! @}

#endif // _BC_CMWX1ZZABZ_H
