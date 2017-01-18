#ifndef _BC_MODULE_POWER_H
#define _BC_MODULE_POWER_H

#include <bc_common.h>

//! @addtogroup bc_module_power bc_module_power
//! @brief Driver for Power Module
//! @section example Init and framebuffer example
//! @code
//! void application_init()
//! {
//!     bc_module_power_init();
//!
//!     // Then you fill the framebuffer with RGBW values. No need to call any refresh function.
//!     // You can change the values in the frameBuffer on the fly.
//!     // Each LED pixel has 4 bytes in framebuffer. For red, green, blue and white color.
//!     frameBuffer[0] = 255;   // Set first LED red channel to 255
//!     frameBuffer[5] = 255;   // Set second LED green channel to 255
//!     frameBuffer[10] = 255;  // Set third LED blue channel to 255
//!     frameBuffer[15] = 255;  // Set fourth LED white channel to 255
//! }
//! @endcode
//! @{


//! @brief Set the LED count
#define BC_MODULE_POWER_MAX_LED_STRIP_COUNT 150

//! This is the RGBW framebuffer
extern uint8_t frameBuffer[];


#define WS2812_RESET_PERIOD 100
#define WS2812B_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define WS2812B_PORT GPIOA
#define WS2812B_PINS (GPIO_PIN_1)

#define WS2812_BUFFER_COUNT 1
//#define WS2812B_DEBUG

typedef struct ws2812b_buffer_item_t
{
    uint8_t *frame_buffer_pointer;
    uint32_t frame_buffer_size;
    uint32_t frame_buffer_counter;

} ws2812b_buffer_item_t;

typedef struct ws2812b_t
{
    ws2812b_buffer_item_t item[WS2812_BUFFER_COUNT];
    uint8_t transfer_complete;
    uint8_t start_transfer;
    uint32_t timer_period_counter;
    uint32_t repeat_counter;

} ws2812b_t;

typedef enum
{
    BC_MODULE_POWER_RGBW = 4,
    BC_MODULE_POWER_RGB = 3

} bc_module_power_mode_t;

typedef struct
{
    bool relay_is_on;
    bool led_strip_on;
    uint32_t led_strip_count;
    bc_module_power_mode_t led_strip_mode;

} bc_module_power_t;


extern bc_module_power_t bc_module_power;
extern ws2812b_t ws2812b;
extern uint8_t frameBuffer[];

void bc_module_power_init(void);

//! @}

#endif /* _BC_MODULE_POWER_H */
