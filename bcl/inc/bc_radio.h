#ifndef _BC_RADIO_H
#define _BC_RADIO_H

#include <bc_common.h>
#include <bc_button.h>
#include <bc_led.h>
#include <bc_spirit1.h>

//! @addtogroup bc_radio bc_radio
//! @brief Radio implementation
//! @{

#ifndef BC_RADIO_MAX_DEVICES
#define BC_RADIO_MAX_DEVICES 4
#endif

#ifndef BC_RADIO_PUB_QUEUE_BUFFER_SIZE
#define BC_RADIO_PUB_QUEUE_BUFFER_SIZE 512
#endif

#ifndef BC_RADIO_RX_QUEUE_BUFFER_SIZE
#define BC_RADIO_RX_QUEUE_BUFFER_SIZE 128
#endif

#define BC_RADIO_ID_SIZE           6
#define BC_RADIO_HEAD_SIZE         (BC_RADIO_ID_SIZE + 2)
#define BC_RADIO_MAX_BUFFER_SIZE   (BC_SPIRIT1_MAX_PACKET_SIZE - BC_RADIO_HEAD_SIZE)
#define BC_RADIO_MAX_TOPIC_LEN     (BC_RADIO_MAX_BUFFER_SIZE - 1 - 4 - 1)
#define BC_RADIO_NULL_BOOL         0xff
#define BC_RADIO_NULL_INT          INT32_MIN
#define BC_RADIO_NULL_FLOAT        NAN
#define BC_RADIO_NULL_UINT32       UINT32_MAX
#define BC_RADIO_NULL_UINT16       UINT16_MAX

//! @brief Radio mode

typedef enum
{
    //! @brief Unknown mode
    BC_RADIO_MODE_UNKNOWN = 0,

    //! @brief Gateway mode
    BC_RADIO_MODE_GATEWAY = 1,

    //! @brief Node listening mode
    BC_RADIO_MODE_NODE_LISTENING = 2,

    //! @brief Node sleeping mode, suitable for battery
    BC_RADIO_MODE_NODE_SLEEPING = 3,

} bc_radio_mode_t;

typedef enum
{
    BC_RADIO_EVENT_INIT_FAILURE = 0,
    BC_RADIO_EVENT_INIT_DONE = 1,
    BC_RADIO_EVENT_ATTACH = 2,
    BC_RADIO_EVENT_ATTACH_FAILURE = 3,
    BC_RADIO_EVENT_DETACH = 4,
    BC_RADIO_EVENT_SCAN_FIND_DEVICE = 5,
    BC_RADIO_EVENT_PAIRED = 6,
    BC_RADIO_EVENT_UNPAIRED = 7,
    BC_RADIO_EVENT_TX_DONE = 8,
    BC_RADIO_EVENT_TX_ERROR = 9

} bc_radio_event_t;

typedef enum
{
    BC_RADIO_HEADER_PAIRING         = 0x00,
    BC_RADIO_HEADER_PUB_PUSH_BUTTON = 0x01,
    BC_RADIO_HEADER_PUB_TEMPERATURE = 0x02,
    BC_RADIO_HEADER_PUB_HUMIDITY    = 0x03,
    BC_RADIO_HEADER_PUB_LUX_METER   = 0x04,
    BC_RADIO_HEADER_PUB_BAROMETER   = 0x05,
    BC_RADIO_HEADER_PUB_CO2         = 0x06,
    BC_RADIO_HEADER_PUB_BUFFER      = 0x07,
    BC_RADIO_HEADER_NODE_ATTACH     = 0x08,
    BC_RADIO_HEADER_NODE_DETACH     = 0x09,
    BC_RADIO_HEADER_PUB_BATTERY     = 0x0a,
    BC_RADIO_HEADER_PUB_INFO        = 0x0b, // deprecated

    BC_RADIO_HEADER_PUB_ACCELERATION = 0x0d,
    BC_RADIO_HEADER_PUB_TOPIC_STRING = 0x0e,
    BC_RADIO_HEADER_PUB_TOPIC_UINT32 = 0x0f,
    BC_RADIO_HEADER_PUB_TOPIC_BOOL  = 0x10,
    BC_RADIO_HEADER_PUB_TOPIC_INT   = 0x11,
    BC_RADIO_HEADER_PUB_TOPIC_FLOAT = 0x12,
    BC_RADIO_HEADER_PUB_EVENT_COUNT = 0x13,
    BC_RADIO_HEADER_PUB_STATE       = 0x14,
    BC_RADIO_HEADER_NODE_STATE_SET  = 0x15,
    BC_RADIO_HEADER_NODE_STATE_GET  = 0x16,
    BC_RADIO_HEADER_NODE_BUFFER     = 0x17,
    BC_RADIO_HEADER_NODE_LED_STRIP_COLOR_SET      = 0x18,
    BC_RADIO_HEADER_NODE_LED_STRIP_BRIGHTNESS_SET = 0x19,
    BC_RADIO_HEADER_NODE_LED_STRIP_COMPOUND_SET   = 0x1a,
    BC_RADIO_HEADER_NODE_LED_STRIP_EFFECT_SET     = 0x1b,
    BC_RADIO_HEADER_NODE_LED_STRIP_THERMOMETER_SET = 0x1c,
    BC_RADIO_HEADER_SUB_DATA        = 0x1d,
    BC_RADIO_HEADER_PUB_VALUE_INT   = 0x1e,

    BC_RADIO_HEADER_SUB_REG         = 0x20,

    BC_RADIO_HEADER_ACK             = 0xaa,


} bc_radio_header_t;

//! @brief Subscribe payload type

typedef enum
{
    BC_RADIO_SUB_PT_BOOL = 0,
    BC_RADIO_SUB_PT_INT = 1,
    BC_RADIO_SUB_PT_FLOAT = 2,
    BC_RADIO_SUB_PT_STRING = 3,
    BC_RADIO_SUB_PT_NULL = 4,

} bc_radio_sub_pt_t;

typedef struct bc_radio_sub_t bc_radio_sub_t;

struct bc_radio_sub_t {
    const char *topic;
    bc_radio_sub_pt_t type;
    void (*callback)(uint64_t *id, const char *, void *,  void *);
    void *param;
};

typedef struct
{
    uint64_t id;
    uint16_t message_id;
    bool message_id_synced;
    bc_radio_mode_t mode;
    int rssi;

} bc_radio_peer_t;

//! @brief Initialize radio
//! @param[in] mode

void bc_radio_init(bc_radio_mode_t mode);

void bc_radio_set_event_handler(void (*event_handler)(bc_radio_event_t, void *), void *event_param);

void bc_radio_listen(bc_tick_t timeout);

void bc_radio_pairing_request(const char *firmware, const char *version);

void bc_radio_pairing_mode_start(void);

void bc_radio_pairing_mode_stop(void);

bool bc_radio_peer_device_add(uint64_t id);

bool bc_radio_peer_device_remove(uint64_t id);

bool bc_radio_peer_device_purge_all(void);

void bc_radio_get_peer_id(uint64_t *id, int length);

void bc_radio_scan_start(void);

void bc_radio_scan_stop(void);

void bc_radio_automatic_pairing_start(void);

void bc_radio_automatic_pairing_stop(void);

uint64_t bc_radio_get_my_id(void);

uint64_t bc_radio_get_event_id(void);

bool bc_radio_is_peer_device(uint64_t id);

bool bc_radio_pub_queue_put(const void *buffer, size_t length);

void bc_radio_set_subs(bc_radio_sub_t *subs, int length);

bool bc_radio_send_sub_data(uint64_t *id, uint8_t order, void *payload, size_t size);

void bc_radio_set_rx_timeout_for_sleeping_node(bc_tick_t timeout);

bc_radio_peer_t *bc_radio_get_peer_device(uint64_t id);

uint8_t *bc_radio_id_to_buffer(uint64_t *id, uint8_t *buffer);
uint8_t *bc_radio_bool_to_buffer(bool *value, uint8_t *buffer);
uint8_t *bc_radio_int_to_buffer(int *value, uint8_t *buffer);
uint8_t *bc_radio_uint16_to_buffer(uint16_t *value, uint8_t *buffer);
uint8_t *bc_radio_uint32_to_buffer(uint32_t *value, uint8_t *buffer);
uint8_t *bc_radio_float_to_buffer(float *value, uint8_t *buffer);
uint8_t *bc_radio_data_to_buffer(void *data, size_t length, uint8_t *buffer);
uint8_t *bc_radio_id_from_buffer(uint8_t *buffer, uint64_t *id);
uint8_t *bc_radio_bool_from_buffer(uint8_t *buffer, bool *value, bool **pointer);
uint8_t *bc_radio_int_from_buffer(uint8_t *buffer, int *value, int **pointer);
uint8_t *bc_radio_uint16_from_buffer(uint8_t *buffer, uint16_t *value, uint16_t **pointer);
uint8_t *bc_radio_uint32_from_buffer(uint8_t *buffer, uint32_t *value, uint32_t **pointer);
uint8_t *bc_radio_float_from_buffer(uint8_t *buffer, float *value, float **pointer);
uint8_t *bc_radio_data_from_buffer(uint8_t *buffer, void *data, size_t length);

void bc_radio_init_pairing_button();

//! @}

#endif // _BC_RADIO_H
