#ifndef _TWR_CHESTER_A_H
#define _TWR_CHESTER_A_H

#include <twr_i2c.h>
#include <twr_tca9534a.h>

//! @addtogroup twr_chester_a twr_chester_a
//! @brief Driver for CHESTER A
//! @{

//! @brief HARDWARIO CHESTER A instance

typedef struct twr_chester_a_t twr_chester_a_t;

//! @brief Relay enum

typedef enum
{
    TWR_CHESTER_A_RELAY_1 = 0,
    TWR_CHESTER_A_RELAY_2 = 1,
    TWR_CHESTER_A_RELAY_BOTH = 2

} twr_chester_a_relay_t;

//! @cond

struct twr_chester_a_t
{
    twr_tca9534a_t tca9534a;
    bool is_tca9534a_initialized;
};

//! @brief Initialize CHESTER A
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @return true On success
//! @return false On Error

bool twr_chester_a_init(twr_chester_a_t *self, twr_i2c_channel_t i2c_channel);

//! @brief Set relay state
//! @param[in] self Instance
//! @param[in] relay Relay
//! @param[in] state Desired relay state
//! @return true On success
//! @return false On Error

bool twr_chester_a_relay_set_state(twr_chester_a_t *self, twr_chester_a_relay_t relay, bool state);

//! @brief Set relay state
//! @param[in] self Instance
//! @param[in] relay Relay
//! @param[out] state Pointer to relay state
//! @return true On success
//! @return false On Error

bool twr_chester_a_relay_get_state(twr_chester_a_t *self, twr_chester_a_relay_t relay, bool *state);

//! @brief Toggle relay to opposite state
//! @param[in] self Instance
//! @param[in] relay Relay
//! @return true On success
//! @return false On Error

bool twr_chester_a_relay_toggle(twr_chester_a_t *self, twr_chester_a_relay_t relay);

//! @}

#endif // _TWR_CHESTER_A_H
