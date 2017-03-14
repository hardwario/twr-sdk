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
    bool (*init)(bc_led_strip_buffer_t *buffer);
    void (*set_pixel)(int position, uint32_t color);
    void (*set_pixel_rgbw)(int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    bool (*write)(void);

} bc_led_strip_driver_t;

typedef enum
{
    BC_LED_STRIP_EVENT_EFFECT_DONE = 0,

} bc_led_strip_event_t;

typedef struct bc_led_strip_t bc_led_strip_t;

struct bc_led_strip_t
{
    const bc_led_strip_driver_t *_driver;
    bc_led_strip_buffer_t *_buffer;

    struct
    {
        int led;
        int round;
        bc_tick_t wait;
        uint32_t color;
        bc_scheduler_task_id_t task_id;

    } _effect;
    void (*_event_handler)(bc_led_strip_t *, bc_led_strip_event_t, void *);
    void *_event_param;

};

void bc_led_strip_init(bc_led_strip_t *self, const bc_led_strip_driver_t *driver, bc_led_strip_buffer_t *buffer);

void bc_led_strip_set_event_handler(bc_led_strip_t *self, void (*event_handler)(bc_led_strip_t *, bc_led_strip_event_t, void *), void *event_param);

int bc_led_strip_get_pixel_count(bc_led_strip_t *self);

bc_led_strip_type_t bc_led_strip_get_strip_type(bc_led_strip_t *self);

void bc_led_strip_set_pixel(bc_led_strip_t *self, int position, uint32_t color);

void bc_led_strip_set_pixel_rgbw(bc_led_strip_t *self, int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

bool bc_led_strip_set_rgbw_framebuffer(bc_led_strip_t *self, uint8_t *framebuffer, size_t length);

void bc_led_strip_fill(bc_led_strip_t *self, uint32_t color);

bool bc_led_strip_write(bc_led_strip_t *self);

void bc_led_strip_effect_stop();

void bc_led_strip_effect_test(bc_led_strip_t *self);

void bc_led_strip_effect_rainbow(bc_led_strip_t *self, bc_tick_t wait);

// Slightly different, this makes the rainbow equally distributed throughout
void bc_led_strip_effect_rainbow_cycle(bc_led_strip_t *self, bc_tick_t wait);

// Fill the dots one after the other with a color
void bc_led_strip_effect_color_wipe(bc_led_strip_t *self, uint32_t color, bc_tick_t wait);

//Theatre-style crawling lights.
void bc_led_strip_effect_theater_chase(bc_led_strip_t *self, uint32_t color, bc_tick_t wait);

void bc_led_strip_effect_theater_chase_rainbow(bc_led_strip_t *self, bc_tick_t wait);

#endif // _BC_LED_STRIP_H
