#ifndef _TWR_ONEWIRE_RELAY_H
#define _TWR_ONEWIRE_RELAY_H

#include <twr_onewire.h>

#define TWR_ONEWIRE_RELAY_FAMILY_CODE 0x29

//! @addtogroup twr_onewire_relay twr_onewire_relay
//! @brief Driver for HARDWARIO 1-wire relay, chipset: DS2408
//! @{

typedef enum
{
    TWR_ONEWIRE_RELAY_CHANNEL_Q1 = 0,
    TWR_ONEWIRE_RELAY_CHANNEL_Q2 = 1,
    TWR_ONEWIRE_RELAY_CHANNEL_Q3 = 2,
    TWR_ONEWIRE_RELAY_CHANNEL_Q4 = 3,
    TWR_ONEWIRE_RELAY_CHANNEL_Q5 = 4,
    TWR_ONEWIRE_RELAY_CHANNEL_Q6 = 5,
    TWR_ONEWIRE_RELAY_CHANNEL_Q7 = 6,
    TWR_ONEWIRE_RELAY_CHANNEL_Q8 = 7

} twr_onewire_relay_channel_t;

//! @brief HARDWARIO 1-wire relay instance

typedef struct
{
    uint64_t _device_number;
    twr_onewire_t *_onewire;
    uint8_t _state;
    bool _state_valid;

} twr_onewire_relay_t;

//! @brief Initialize relay
//! @param[in] self Instance
//! @param[in] Pointer on instance 1-Wire
//! @return true On success
//! @return false On failure

bool twr_onewire_relay_init(twr_onewire_relay_t *self, twr_onewire_t *onewire, uint64_t device_number);

//! @brief Set relay to specified state
//! @param[in] self Instance
//! @param[in] channel
//! @param[in] state Desired relay state
//! @return true On success
//! @return false On failure

bool twr_onewire_relay_set_state(twr_onewire_relay_t *self, twr_onewire_relay_channel_t relay_channel, bool state);

//! @brief Get current relay state
//! @param[in] self Instance
//! @param[in] channel
//! @param[out] state
//! @return true When state is valid
//! @return false When state is invalid

bool twr_onewire_relay_get_state(twr_onewire_relay_t *self, twr_onewire_relay_channel_t relay_channel, bool *state);

//! @}

#endif // _TWR_ONEWIRE_RELAY_H
