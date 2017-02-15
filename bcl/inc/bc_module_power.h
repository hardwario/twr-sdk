#ifndef _BC_MODULE_POWER_H
#define _BC_MODULE_POWER_H

#include <bc_ws2812b.h>

//! @addtogroup bc_module_power bc_module_power
//! @brief Driver for Power Module
//! @section example Init and framebuffer example
//! @code
//! void application_init()
//! {
//!     bc_module_power_init();
//!
//!     // Then you fill the framebuffer with RGBW values.
//!     // You can change the values in the frameBuffer on the fly.
//!     // Each LED pixel has 4 bytes in framebuffer. For red, green, blue and white color.
//!     frameBuffer[0] = 255;   // Set first LED red channel to 255
//!     frameBuffer[5] = 255;   // Set second LED green channel to 255
//!     frameBuffer[10] = 255;  // Set third LED blue channel to 255
//!     frameBuffer[15] = 255;  // Set fourth LED white channel to 255
//!
//!		uint8_t framebuffer[50];
//!   	memset(framebuffer, 255, sizeof(framebuffer));
//!   	bc_module_power_print_frame_buffer(framebuffer, sizeof(framebuffer));
//! }
//! @endcode
//! @{

//! @brief Set the LED count
#define BC_MODULE_POWER_MAX_LED_STRIP_COUNT 150

void bc_module_power_init();

void bc_module_power_led_strip_test(void);

bool bc_module_power_set_led_strip(const uint8_t *frame_buffer, size_t length);

void bc_module_power_set_relay(bool state);

bool bc_module_power_get_relay();

void bc_module_power_led_strip_set_pixel(uint16_t position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);


//! @}

#endif // _BC_MODULE_POWER_H
