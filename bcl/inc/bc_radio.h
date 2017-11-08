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
#define BC_RADIO_ID_SIZE           6
#define BC_RADIO_HEAD_SIZE         (BC_RADIO_ID_SIZE + 2)
#define BC_RADIO_MAX_BUFFER_SIZE   (BC_SPIRIT1_MAX_PACKET_SIZE - BC_RADIO_HEAD_SIZE)
#define BC_RADIO_MAX_TOPIC_LEN     (BC_RADIO_MAX_BUFFER_SIZE - 1 - 4 - 1)
#define BC_RADIO_NULL_BOOL         0xff
#define BC_RADIO_NULL_INT          INT32_MIN
#define BC_RADIO_NULL_FLOAT        NAN

//! @brief Radio mode

typedef enum
{
    //! @brief Gateway mode
    BC_RADIO_MODE_GATEWAY = 0,

    //! @brief Node sleeping mode, suitable for battery
    BC_RADIO_MODE_NODE_SLEEPING = 1,

    //! @brief Node listening mode
    BC_RADIO_MODE_NODE_LISTENING = 2,

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

} bc_radio_event_t;

typedef enum
{
    BC_RADIO_HEADER_PAIRING = 0,
    BC_RADIO_HEADER_ACK = 1,

    BC_RADIO_HEADER_NODE_ATTACH = 6,
    BC_RADIO_HEADER_NODE_DETACH = 7,

    BC_RADIO_HEADER_PUB_TOPIC_BOOL = 10,
    BC_RADIO_HEADER_PUB_TOPIC_INT = 11,
    BC_RADIO_HEADER_PUB_TOPIC_FLOAT = 12,
    BC_RADIO_HEADER_PUB_EVENT_COUNT = 13,
    BC_RADIO_HEADER_PUB_STATE = 14,
    BC_RADIO_HEADER_NODE_STATE_SET = 15,
    BC_RADIO_HEADER_NODE_STATE_GET = 16,
    BC_RADIO_HEADER_PUB_BUFFER = 17,
    BC_RADIO_HEADER_NODE_BUFFER = 18,

    BC_RADIO_HEADER_PUB_THERMOMETER = 80,
    BC_RADIO_HEADER_PUB_HUMIDITY = 81,
    BC_RADIO_HEADER_PUB_LUX_METER = 82,
    BC_RADIO_HEADER_PUB_BAROMETER = 83,
    BC_RADIO_HEADER_PUB_CO2 = 84,
    BC_RADIO_HEADER_PUB_BATTERY = 85,

} bc_radio_header_t;

//! @brief Initialize radio
//! @param[in] mode

void bc_radio_init(bc_radio_mode_t mode);

void bc_radio_set_event_handler(void (*event_handler)(bc_radio_event_t, void *), void *event_param);

void bc_radio_listen(void);

void bc_radio_sleep(void);

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

uint8_t *bc_radio_id_to_buffer(uint64_t *id, uint8_t *buffer);
uint8_t *bc_radio_bool_to_buffer(bool *value, uint8_t *buffer);
uint8_t *bc_radio_int_to_buffer(int *value, uint8_t *buffer);
uint8_t *bc_radio_float_to_buffer(float *value, uint8_t *buffer);
uint8_t *bc_radio_id_from_buffer(uint8_t *buffer, uint64_t *id);
uint8_t *bc_radio_bool_from_buffer(uint8_t *buffer, bool **value);
uint8_t *bc_radio_int_from_buffer(uint8_t *buffer, int **value);
uint8_t *bc_radio_float_from_buffer(uint8_t *buffer, float **value);

void bc_radio_init_pairing_button();

//! @}

#endif // _BC_RADIO_H
