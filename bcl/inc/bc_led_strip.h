#ifndef _BC_LED_STRIP_H
#define _BC_LED_STRIP_H

#include <bc_scheduler.h>

typedef enum
{
    BC_LED_STRIP_TYPE_RGBW = 4,
    BC_LED_STRIP_TYPE_RGB = 3

} bc_led_strip_type_t;

typedef struct
{
    bc_led_strip_type_t type;
    int count;
    uint32_t *buffer;

} bc_led_strip_buffer_t;

typedef struct
{
    bool (*init)(const bc_led_strip_buffer_t *buffer);
    void (*set_pixel)(int position, uint32_t color);
    void (*set_pixel_rgbw)(int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    bool (*write)(void);

} bc_led_strip_driver_t;

typedef enum
{
    BC_LED_STRIP_EFFECT_NONE = 0,
    BC_LED_STRIP_EFFECT_TEST,
    BC_LED_STRIP_EFFECT_RAINBOW_CYCLE

} bc_led_strip_effect_t;

typedef struct
{
    const bc_led_strip_driver_t *_driver;
    const bc_led_strip_buffer_t *_buffer;
    bc_scheduler_task_id_t _task_id;
    struct
    {
        bc_led_strip_effect_t name;
        int led;
        int step;
        bc_tick_t wait;

    } _effect;

} bc_led_strip_t;

void bc_led_strip_init(bc_led_strip_t *self, const bc_led_strip_driver_t *driver, const bc_led_strip_buffer_t *buffer);

int bc_led_strip_get_pixel_count(bc_led_strip_t *self);

bc_led_strip_type_t bc_led_strip_get_strip_type(bc_led_strip_t *self);

void bc_led_strip_set_pixel(bc_led_strip_t *self, int position, uint32_t color);

void bc_led_strip_set_pixel_rgbw(bc_led_strip_t *self, int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

bool bc_led_strip_set_rgbw_framebuffer(bc_led_strip_t *self, const uint8_t *framebuffer, size_t length);

void bc_led_strip_fill(bc_led_strip_t *self, uint32_t color);

bool bc_led_strip_write(bc_led_strip_t *self);

void bc_led_strip_test(bc_led_strip_t *self);

void bc_led_strip_effect_rainbow_cycle(bc_led_strip_t *self);

#endif // _BC_LED_STRIP_H
