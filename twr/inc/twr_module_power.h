#ifndef _TWR_MODULE_POWER_H
#define _TWR_MODULE_POWER_H

#include <twr_common.h>
#include <twr_led_strip.h>

//! @addtogroup twr_module_power twr_module_power
//! @brief Driver for Power Module
//! @{

//! @cond

extern const twr_led_strip_buffer_t twr_module_power_led_strip_buffer_rgbw_144;
extern const twr_led_strip_buffer_t twr_module_power_led_strip_buffer_rgb_150;

//! @endcond

//! @brief Initialize power module

void twr_module_power_init(void);

//! @brief Set relay state
//! @param[in] state Desired relay state

void twr_module_power_relay_set_state(bool state);

//! @brief Set relay state
//! @return Relay state

bool twr_module_power_relay_get_state(void);

//! @brief Set relay state
//! @return Relay state

const twr_led_strip_driver_t *twr_module_power_get_led_strip_driver(void);

//! @}

#endif // _TWR_MODULE_POWER_H
