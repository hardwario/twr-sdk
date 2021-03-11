#ifndef _TWR_RADIO_H
#define _TWR_RADIO_H

#include <twr_common.h>
#include <twr_button.h>
#include <twr_led.h>
#include <twr_spirit1.h>

//! @addtogroup twr_radio twr_radio
//! @brief Radio implementation
//! @{

#ifndef TWR_RADIO_MAX_DEVICES
#define TWR_RADIO_MAX_DEVICES 4
#endif

#ifndef TWR_RADIO_PUB_QUEUE_BUFFER_SIZE
#define TWR_RADIO_PUB_QUEUE_BUFFER_SIZE 512
#endif

#ifndef TWR_RADIO_RX_QUEUE_BUFFER_SIZE
#define TWR_RADIO_RX_QUEUE_BUFFER_SIZE 128
#endif

#define TWR_RADIO_ID_SIZE           6
#define TWR_RADIO_HEAD_SIZE         (TWR_RADIO_ID_SIZE + 2)
#define TWR_RADIO_MAX_BUFFER_SIZE   (TWR_SPIRIT1_MAX_PACKET_SIZE - TWR_RADIO_HEAD_SIZE)
#define TWR_RADIO_MAX_TOPIC_LEN     (TWR_RADIO_MAX_BUFFER_SIZE - 1 - 4 - 1)
#define TWR_RADIO_NULL_BOOL         0xff
#define TWR_RADIO_NULL_INT          INT32_MIN
#define TWR_RADIO_NULL_FLOAT        NAN
#define TWR_RADIO_NULL_UINT32       UINT32_MAX
#define TWR_RADIO_NULL_UINT16       UINT16_MAX

//! @brief Radio mode

typedef enum
{
    //! @brief Unknown mode
    TWR_RADIO_MODE_UNKNOWN = 0,

    //! @brief Gateway mode
    TWR_RADIO_MODE_GATEWAY = 1,

    //! @brief Node listening mode
    TWR_RADIO_MODE_NODE_LISTENING = 2,

    //! @brief Node sleeping mode, suitable for battery
    TWR_RADIO_MODE_NODE_SLEEPING = 3,

} twr_radio_mode_t;

typedef enum
{
    TWR_RADIO_EVENT_INIT_FAILURE = 0,
    TWR_RADIO_EVENT_INIT_DONE = 1,
    TWR_RADIO_EVENT_ATTACH = 2,
    TWR_RADIO_EVENT_ATTACH_FAILURE = 3,
    TWR_RADIO_EVENT_DETACH = 4,
    TWR_RADIO_EVENT_SCAN_FIND_DEVICE = 5,
    TWR_RADIO_EVENT_PAIRED = 6,
    TWR_RADIO_EVENT_UNPAIRED = 7,
    TWR_RADIO_EVENT_TX_DONE = 8,
    TWR_RADIO_EVENT_TX_ERROR = 9

} twr_radio_event_t;

typedef enum
{
    TWR_RADIO_HEADER_PAIRING         = 0x00,
    TWR_RADIO_HEADER_PUB_PUSH_BUTTON = 0x01,
    TWR_RADIO_HEADER_PUB_TEMPERATURE = 0x02,
    TWR_RADIO_HEADER_PUB_HUMIDITY    = 0x03,
    TWR_RADIO_HEADER_PUB_LUX_METER   = 0x04,
    TWR_RADIO_HEADER_PUB_BAROMETER   = 0x05,
    TWR_RADIO_HEADER_PUB_CO2         = 0x06,
    TWR_RADIO_HEADER_PUB_BUFFER      = 0x07,
    TWR_RADIO_HEADER_NODE_ATTACH     = 0x08,
    TWR_RADIO_HEADER_NODE_DETACH     = 0x09,
    TWR_RADIO_HEADER_PUB_BATTERY     = 0x0a,
    TWR_RADIO_HEADER_PUB_INFO        = 0x0b, // deprecated

    TWR_RADIO_HEADER_PUB_ACCELERATION = 0x0d,
    TWR_RADIO_HEADER_PUB_TOPIC_STRING = 0x0e,
    TWR_RADIO_HEADER_PUB_TOPIC_UINT32 = 0x0f,
    TWR_RADIO_HEADER_PUB_TOPIC_BOOL  = 0x10,
    TWR_RADIO_HEADER_PUB_TOPIC_INT   = 0x11,
    TWR_RADIO_HEADER_PUB_TOPIC_FLOAT = 0x12,
    TWR_RADIO_HEADER_PUB_EVENT_COUNT = 0x13,
    TWR_RADIO_HEADER_PUB_STATE       = 0x14,
    TWR_RADIO_HEADER_NODE_STATE_SET  = 0x15,
    TWR_RADIO_HEADER_NODE_STATE_GET  = 0x16,
    TWR_RADIO_HEADER_NODE_BUFFER     = 0x17,
    TWR_RADIO_HEADER_NODE_LED_STRIP_COLOR_SET      = 0x18,
    TWR_RADIO_HEADER_NODE_LED_STRIP_BRIGHTNESS_SET = 0x19,
    TWR_RADIO_HEADER_NODE_LED_STRIP_COMPOUND_SET   = 0x1a,
    TWR_RADIO_HEADER_NODE_LED_STRIP_EFFECT_SET     = 0x1b,
    TWR_RADIO_HEADER_NODE_LED_STRIP_THERMOMETER_SET = 0x1c,
    TWR_RADIO_HEADER_SUB_DATA        = 0x1d,
    TWR_RADIO_HEADER_PUB_VALUE_INT   = 0x1e,

    TWR_RADIO_HEADER_SUB_REG         = 0x20,

    TWR_RADIO_HEADER_ACK             = 0xaa,


} twr_radio_header_t;

//! @brief Subscribe payload type

typedef enum
{
    TWR_RADIO_SUB_PT_BOOL = 0,
    TWR_RADIO_SUB_PT_INT = 1,
    TWR_RADIO_SUB_PT_FLOAT = 2,
    TWR_RADIO_SUB_PT_STRING = 3,
    TWR_RADIO_SUB_PT_NULL = 4,

} twr_radio_sub_pt_t;

typedef struct twr_radio_sub_t twr_radio_sub_t;

struct twr_radio_sub_t {
    const char *topic;
    twr_radio_sub_pt_t type;
    void (*callback)(uint64_t *id, const char *, void *,  void *);
    void *param;
};

typedef struct
{
    uint64_t id;
    uint16_t message_id;
    bool message_id_synced;
    twr_radio_mode_t mode;
    int rssi;

} twr_radio_peer_t;

//! @brief Initialize radio
//! @param[in] mode

void twr_radio_init(twr_radio_mode_t mode);

void twr_radio_set_event_handler(void (*event_handler)(twr_radio_event_t, void *), void *event_param);

void twr_radio_listen(twr_tick_t timeout);

void twr_radio_pairing_request(const char *firmware, const char *version);

void twr_radio_pairing_mode_start(void);

void twr_radio_pairing_mode_stop(void);

bool twr_radio_peer_device_add(uint64_t id);

bool twr_radio_peer_device_remove(uint64_t id);

bool twr_radio_peer_device_purge_all(void);

void twr_radio_get_peer_id(uint64_t *id, int length);

void twr_radio_scan_start(void);

void twr_radio_scan_stop(void);

void twr_radio_automatic_pairing_start(void);

void twr_radio_automatic_pairing_stop(void);

uint64_t twr_radio_get_my_id(void);

uint64_t twr_radio_get_event_id(void);

bool twr_radio_is_peer_device(uint64_t id);

bool twr_radio_pub_queue_put(const void *buffer, size_t length);

//! @brief Clear the publish message queue

void twr_radio_pub_queue_clear();

void twr_radio_set_subs(twr_radio_sub_t *subs, int length);

bool twr_radio_send_sub_data(uint64_t *id, uint8_t order, void *payload, size_t size);

void twr_radio_set_rx_timeout_for_sleeping_node(twr_tick_t timeout);

twr_radio_peer_t *twr_radio_get_peer_device(uint64_t id);

uint8_t *twr_radio_id_to_buffer(uint64_t *id, uint8_t *buffer);
uint8_t *twr_radio_bool_to_buffer(bool *value, uint8_t *buffer);
uint8_t *twr_radio_int_to_buffer(int *value, uint8_t *buffer);
uint8_t *twr_radio_uint16_to_buffer(uint16_t *value, uint8_t *buffer);
uint8_t *twr_radio_uint32_to_buffer(uint32_t *value, uint8_t *buffer);
uint8_t *twr_radio_float_to_buffer(float *value, uint8_t *buffer);
uint8_t *twr_radio_data_to_buffer(void *data, size_t length, uint8_t *buffer);
uint8_t *twr_radio_id_from_buffer(uint8_t *buffer, uint64_t *id);
uint8_t *twr_radio_bool_from_buffer(uint8_t *buffer, bool *value, bool **pointer);
uint8_t *twr_radio_int_from_buffer(uint8_t *buffer, int *value, int **pointer);
uint8_t *twr_radio_uint16_from_buffer(uint8_t *buffer, uint16_t *value, uint16_t **pointer);
uint8_t *twr_radio_uint32_from_buffer(uint8_t *buffer, uint32_t *value, uint32_t **pointer);
uint8_t *twr_radio_float_from_buffer(uint8_t *buffer, float *value, float **pointer);
uint8_t *twr_radio_data_from_buffer(uint8_t *buffer, void *data, size_t length);

void twr_radio_init_pairing_button();

//! @}

#endif // _TWR_RADIO_H
