#ifndef _HIO_MODULE_POWER_H
#define _HIO_MODULE_POWER_H

#include <hio_common.h>
#include <hio_led_strip.h>

//! @addtogroup hio_module_power hio_module_power
//! @brief Driver for Power Module
//! @{

//! @cond

extern const hio_led_strip_buffer_t hio_module_power_led_strip_buffer_rgbw_144;
extern const hio_led_strip_buffer_t hio_module_power_led_strip_buffer_rgb_150;

//! @endcond

//! @brief Initialize power module

void hio_module_power_init(void);

//! @brief Set relay state
//! @param[in] state Desired relay state

void hio_module_power_relay_set_state(bool state);

//! @brief Set relay state
//! @return Relay state

bool hio_module_power_relay_get_state(void);

//! @brief Set relay state
//! @return Relay state

const hio_led_strip_driver_t *hio_module_power_get_led_strip_driver(void);

//! @}

#endif // _HIO_MODULE_POWER_H
