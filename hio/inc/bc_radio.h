#ifndef _HIO_RADIO_H
#define _HIO_RADIO_H

#include <hio_common.h>
#include <hio_button.h>
#include <hio_led.h>
#include <hio_spirit1.h>

//! @addtogroup hio_radio hio_radio
//! @brief Radio implementation
//! @{

#ifndef HIO_RADIO_MAX_DEVICES
#define HIO_RADIO_MAX_DEVICES 4
#endif

#ifndef HIO_RADIO_PUB_QUEUE_BUFFER_SIZE
#define HIO_RADIO_PUB_QUEUE_BUFFER_SIZE 512
#endif

#ifndef HIO_RADIO_RX_QUEUE_BUFFER_SIZE
#define HIO_RADIO_RX_QUEUE_BUFFER_SIZE 128
#endif

#define HIO_RADIO_ID_SIZE           6
#define HIO_RADIO_HEAD_SIZE         (HIO_RADIO_ID_SIZE + 2)
#define HIO_RADIO_MAX_BUFFER_SIZE   (HIO_SPIRIT1_MAX_PACKET_SIZE - HIO_RADIO_HEAD_SIZE)
#define HIO_RADIO_MAX_TOPIC_LEN     (HIO_RADIO_MAX_BUFFER_SIZE - 1 - 4 - 1)
#define HIO_RADIO_NULL_BOOL         0xff
#define HIO_RADIO_NULL_INT          INT32_MIN
#define HIO_RADIO_NULL_FLOAT        NAN
#define HIO_RADIO_NULL_UINT32       UINT32_MAX
#define HIO_RADIO_NULL_UINT16       UINT16_MAX

//! @brief Radio mode

typedef enum
{
    //! @brief Unknown mode
    HIO_RADIO_MODE_UNKNOWN = 0,

    //! @brief Gateway mode
    HIO_RADIO_MODE_GATEWAY = 1,

    //! @brief Node listening mode
    HIO_RADIO_MODE_NODE_LISTENING = 2,

    //! @brief Node sleeping mode, suitable for battery
    HIO_RADIO_MODE_NODE_SLEEPING = 3,

} hio_radio_mode_t;

typedef enum
{
    HIO_RADIO_EVENT_INIT_FAILURE = 0,
    HIO_RADIO_EVENT_INIT_DONE = 1,
    HIO_RADIO_EVENT_ATTACH = 2,
    HIO_RADIO_EVENT_ATTACH_FAILURE = 3,
    HIO_RADIO_EVENT_DETACH = 4,
    HIO_RADIO_EVENT_SCAN_FIND_DEVICE = 5,
    HIO_RADIO_EVENT_PAIRED = 6,
    HIO_RADIO_EVENT_UNPAIRED = 7,
    HIO_RADIO_EVENT_TX_DONE = 8,
    HIO_RADIO_EVENT_TX_ERROR = 9

} hio_radio_event_t;

typedef enum
{
    HIO_RADIO_HEADER_PAIRING         = 0x00,
    HIO_RADIO_HEADER_PUB_PUSH_BUTTON = 0x01,
    HIO_RADIO_HEADER_PUB_TEMPERATURE = 0x02,
    HIO_RADIO_HEADER_PUB_HUMIDITY    = 0x03,
    HIO_RADIO_HEADER_PUB_LUX_METER   = 0x04,
    HIO_RADIO_HEADER_PUB_BAROMETER   = 0x05,
    HIO_RADIO_HEADER_PUB_CO2         = 0x06,
    HIO_RADIO_HEADER_PUB_BUFFER      = 0x07,
    HIO_RADIO_HEADER_NODE_ATTACH     = 0x08,
    HIO_RADIO_HEADER_NODE_DETACH     = 0x09,
    HIO_RADIO_HEADER_PUB_BATTERY     = 0x0a,
    HIO_RADIO_HEADER_PUB_INFO        = 0x0b, // deprecated

    HIO_RADIO_HEADER_PUB_ACCELERATION = 0x0d,
    HIO_RADIO_HEADER_PUB_TOPIC_STRING = 0x0e,
    HIO_RADIO_HEADER_PUB_TOPIC_UINT32 = 0x0f,
    HIO_RADIO_HEADER_PUB_TOPIC_BOOL  = 0x10,
    HIO_RADIO_HEADER_PUB_TOPIC_INT   = 0x11,
    HIO_RADIO_HEADER_PUB_TOPIC_FLOAT = 0x12,
    HIO_RADIO_HEADER_PUB_EVENT_COUNT = 0x13,
    HIO_RADIO_HEADER_PUB_STATE       = 0x14,
    HIO_RADIO_HEADER_NODE_STATE_SET  = 0x15,
    HIO_RADIO_HEADER_NODE_STATE_GET  = 0x16,
    HIO_RADIO_HEADER_NODE_BUFFER     = 0x17,
    HIO_RADIO_HEADER_NODE_LED_STRIP_COLOR_SET      = 0x18,
    HIO_RADIO_HEADER_NODE_LED_STRIP_BRIGHTNESS_SET = 0x19,
    HIO_RADIO_HEADER_NODE_LED_STRIP_COMPOUND_SET   = 0x1a,
    HIO_RADIO_HEADER_NODE_LED_STRIP_EFFECT_SET     = 0x1b,
    HIO_RADIO_HEADER_NODE_LED_STRIP_THERMOMETER_SET = 0x1c,
    HIO_RADIO_HEADER_SUB_DATA        = 0x1d,
    HIO_RADIO_HEADER_PUB_VALUE_INT   = 0x1e,

    HIO_RADIO_HEADER_SUB_REG         = 0x20,

    HIO_RADIO_HEADER_ACK             = 0xaa,


} hio_radio_header_t;

//! @brief Subscribe payload type

typedef enum
{
    HIO_RADIO_SUB_PT_BOOL = 0,
    HIO_RADIO_SUB_PT_INT = 1,
    HIO_RADIO_SUB_PT_FLOAT = 2,
    HIO_RADIO_SUB_PT_STRING = 3,
    HIO_RADIO_SUB_PT_NULL = 4,

} hio_radio_sub_pt_t;

typedef struct hio_radio_sub_t hio_radio_sub_t;

struct hio_radio_sub_t {
    const char *topic;
    hio_radio_sub_pt_t type;
    void (*callback)(uint64_t *id, const char *, void *,  void *);
    void *param;
};

typedef struct
{
    uint64_t id;
    uint16_t message_id;
    bool message_id_synced;
    hio_radio_mode_t mode;
    int rssi;

} hio_radio_peer_t;

//! @brief Initialize radio
//! @param[in] mode

void hio_radio_init(hio_radio_mode_t mode);

void hio_radio_set_event_handler(void (*event_handler)(hio_radio_event_t, void *), void *event_param);

void hio_radio_listen(hio_tick_t timeout);

void hio_radio_pairing_request(const char *firmware, const char *version);

void hio_radio_pairing_mode_start(void);

void hio_radio_pairing_mode_stop(void);

bool hio_radio_peer_device_add(uint64_t id);

bool hio_radio_peer_device_remove(uint64_t id);

bool hio_radio_peer_device_purge_all(void);

void hio_radio_get_peer_id(uint64_t *id, int length);

void hio_radio_scan_start(void);

void hio_radio_scan_stop(void);

void hio_radio_automatic_pairing_start(void);

void hio_radio_automatic_pairing_stop(void);

uint64_t hio_radio_get_my_id(void);

uint64_t hio_radio_get_event_id(void);

bool hio_radio_is_peer_device(uint64_t id);

bool hio_radio_pub_queue_put(const void *buffer, size_t length);

void hio_radio_set_subs(hio_radio_sub_t *subs, int length);

bool hio_radio_send_sub_data(uint64_t *id, uint8_t order, void *payload, size_t size);

void hio_radio_set_rx_timeout_for_sleeping_node(hio_tick_t timeout);

hio_radio_peer_t *hio_radio_get_peer_device(uint64_t id);

uint8_t *hio_radio_id_to_buffer(uint64_t *id, uint8_t *buffer);
uint8_t *hio_radio_bool_to_buffer(bool *value, uint8_t *buffer);
uint8_t *hio_radio_int_to_buffer(int *value, uint8_t *buffer);
uint8_t *hio_radio_uint16_to_buffer(uint16_t *value, uint8_t *buffer);
uint8_t *hio_radio_uint32_to_buffer(uint32_t *value, uint8_t *buffer);
uint8_t *hio_radio_float_to_buffer(float *value, uint8_t *buffer);
uint8_t *hio_radio_data_to_buffer(void *data, size_t length, uint8_t *buffer);
uint8_t *hio_radio_id_from_buffer(uint8_t *buffer, uint64_t *id);
uint8_t *hio_radio_bool_from_buffer(uint8_t *buffer, bool *value, bool **pointer);
uint8_t *hio_radio_int_from_buffer(uint8_t *buffer, int *value, int **pointer);
uint8_t *hio_radio_uint16_from_buffer(uint8_t *buffer, uint16_t *value, uint16_t **pointer);
uint8_t *hio_radio_uint32_from_buffer(uint8_t *buffer, uint32_t *value, uint32_t **pointer);
uint8_t *hio_radio_float_from_buffer(uint8_t *buffer, float *value, float **pointer);
uint8_t *hio_radio_data_from_buffer(uint8_t *buffer, void *data, size_t length);

void hio_radio_init_pairing_button();

//! @}

#endif // _HIO_RADIO_H
