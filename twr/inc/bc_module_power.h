#ifndef _BC_MODULE_POWER_H
#define _BC_MODULE_POWER_H

#include <bc_common.h>
#include <bc_led_strip.h>

//! @addtogroup bc_module_power bc_module_power
//! @brief Driver for Power Module
//! @{

//! @cond

extern const bc_led_strip_buffer_t bc_module_power_led_strip_buffer_rgbw_144;
extern const bc_led_strip_buffer_t bc_module_power_led_strip_buffer_rgb_150;

//! @endcond

//! @brief Initialize power module

void bc_module_power_init(void);

//! @brief Set relay state
//! @param[in] state Desired relay state

void bc_module_power_relay_set_state(bool state);

//! @brief Set relay state
//! @return Relay state

bool bc_module_power_relay_get_state(void);

//! @brief Set relay state
//! @return Relay state

const bc_led_strip_driver_t *bc_module_power_get_led_strip_driver(void);

//! @}

#endif // _BC_MODULE_POWER_H
