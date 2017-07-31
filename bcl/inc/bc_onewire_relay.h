#ifndef _BC_ONEWIRE_RELAY_H
#define _BC_ONEWIRE_RELAY_H

#include <bc_onewire.h>

#define BC_ONEWIRE_RELAY_FAMILY_CODE 0x29

//! @addtogroup bc_onewire_relay bc_onewire_relay
//! @brief Driver for BigClown 1-wire relay, chipset: DS2408
//! @{

typedef enum
{
    BC_ONEWIRE_RELAY_CHANNEL_Q1 = 0,
    BC_ONEWIRE_RELAY_CHANNEL_Q2 = 1,
    BC_ONEWIRE_RELAY_CHANNEL_Q3 = 2,
    BC_ONEWIRE_RELAY_CHANNEL_Q4 = 3,
    BC_ONEWIRE_RELAY_CHANNEL_Q5 = 4,
    BC_ONEWIRE_RELAY_CHANNEL_Q6 = 5,
    BC_ONEWIRE_RELAY_CHANNEL_Q7 = 6,
    BC_ONEWIRE_RELAY_CHANNEL_Q8 = 7

} bc_onewire_relay_channel_t;

//! @brief BigClown 1-wire relay instance

typedef struct
{
    uint64_t _device_number;
    bc_gpio_channel_t _channel;
    uint8_t _state;
    bool _state_valid;

} bc_onewire_relay_t;

//! @brief Initialize relay
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool bc_onewire_relay_init(bc_onewire_relay_t *self, bc_gpio_channel_t channel, uint64_t device_number);

//! @brief Set relay to specified state
//! @param[in] self Instance
//! @param[in] channel
//! @param[in] state Desired relay state
//! @return true On success
//! @return false On failure

bool bc_onewire_relay_set_state(bc_onewire_relay_t *self, bc_onewire_relay_channel_t relay_channel, bool state);

//! @brief Get current relay state
//! @param[in] self Instance
//! @param[in] channel
//! @param[out] state
//! @return true When state is valid
//! @return false When state is invalid

bool bc_onewire_relay_get_state(bc_onewire_relay_t *self, bc_onewire_relay_channel_t relay_channel, bool *state);

//! @}

#endif // _BC_ONEWIRE_RELAY_H
