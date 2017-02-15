#ifndef _BC_MODULE_POWER_H
#define _BC_MODULE_POWER_H

#include <bc_common.h>
#include <bc_ws2812b.h>


//TODO rewrite doc

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

typedef struct
{
    bool relay_is_on;
    bool led_strip_on;
    uint16_t led_strip_count;
    bc_ws2812b_type_t led_strip_type;
    bool test;
    //uint8_t *frameBuffer; //[4 * BC_MODULE_POWER_MAX_LED_STRIP_COUNT]

} bc_module_power_t;

extern bc_module_power_t bc_module_power;

void bc_module_power_init(void);

void bc_module_power_led_strip_test(void);

bool bc_module_power_print_frame_buffer(uint8_t *frame_buffer, size_t size);

//! @}

#endif // _BC_MODULE_POWER_H
