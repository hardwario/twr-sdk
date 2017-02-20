#ifndef _BC_MODULE_POWER_H
#define _BC_MODULE_POWER_H

#include <bc_common.h>
#include <bc_led_strip.h>

//! @addtogroup bc_module_power bc_module_power
//! @brief Driver for Power Module
//! @{

void bc_module_power_init(void);

void bc_module_power_relay_set_state(bool state);

bool bc_module_power_relay_get_state(void);

void bc_module_power_led_strip_init(bc_led_strip_t *led_strip);

void bc_module_power_led_strip_test(void);

void bc_module_power_led_strip_set_pixel(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

void bc_module_power_led_strip_set_pixel_uint(int position, uint32_t color);

bool bc_module_power_led_strip_set_framebuffer(const uint8_t *framebuffer, size_t length);

//! @}

#endif // _BC_MODULE_POWER_H
