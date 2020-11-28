#ifndef _HIO_ONEWIRE_RELAY_H
#define _HIO_ONEWIRE_RELAY_H

#include <hio_onewire.h>

#define HIO_ONEWIRE_RELAY_FAMILY_CODE 0x29

//! @addtogroup hio_onewire_relay hio_onewire_relay
//! @brief Driver for HARDWARIO 1-wire relay, chipset: DS2408
//! @{

typedef enum
{
    HIO_ONEWIRE_RELAY_CHANNEL_Q1 = 0,
    HIO_ONEWIRE_RELAY_CHANNEL_Q2 = 1,
    HIO_ONEWIRE_RELAY_CHANNEL_Q3 = 2,
    HIO_ONEWIRE_RELAY_CHANNEL_Q4 = 3,
    HIO_ONEWIRE_RELAY_CHANNEL_Q5 = 4,
    HIO_ONEWIRE_RELAY_CHANNEL_Q6 = 5,
    HIO_ONEWIRE_RELAY_CHANNEL_Q7 = 6,
    HIO_ONEWIRE_RELAY_CHANNEL_Q8 = 7

} hio_onewire_relay_channel_t;

//! @brief HARDWARIO 1-wire relay instance

typedef struct
{
    uint64_t _device_number;
    hio_onewire_t *_onewire;
    uint8_t _state;
    bool _state_valid;

} hio_onewire_relay_t;

//! @brief Initialize relay
//! @param[in] self Instance
//! @param[in] Pointer on instance 1-Wire
//! @return true On success
//! @return false On failure

bool hio_onewire_relay_init(hio_onewire_relay_t *self, hio_onewire_t *onewire, uint64_t device_number);

//! @brief Set relay to specified state
//! @param[in] self Instance
//! @param[in] channel
//! @param[in] state Desired relay state
//! @return true On success
//! @return false On failure

bool hio_onewire_relay_set_state(hio_onewire_relay_t *self, hio_onewire_relay_channel_t relay_channel, bool state);

//! @brief Get current relay state
//! @param[in] self Instance
//! @param[in] channel
//! @param[out] state
//! @return true When state is valid
//! @return false When state is invalid

bool hio_onewire_relay_get_state(hio_onewire_relay_t *self, hio_onewire_relay_channel_t relay_channel, bool *state);

//! @}

#endif // _HIO_ONEWIRE_RELAY_H
