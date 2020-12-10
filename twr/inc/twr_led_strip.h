#ifndef _TWR_LED_STRIP_H
#define _TWR_LED_STRIP_H

#include <twr_scheduler.h>

typedef enum
{
    TWR_LED_STRIP_TYPE_RGBW = 4,
    TWR_LED_STRIP_TYPE_RGB = 3

} twr_led_strip_type_t;

typedef struct
{
    twr_led_strip_type_t type;
    int count;
    uint32_t *buffer;

} twr_led_strip_buffer_t;

typedef struct
{
    bool (*init)(const twr_led_strip_buffer_t *buffer);
    void (*set_pixel)(int position, uint32_t color);
    void (*set_pixel_rgbw)(int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    bool (*write)(void);
    bool (*is_ready)(void);

} twr_led_strip_driver_t;

typedef enum
{
    TWR_LED_STRIP_EVENT_EFFECT_DONE = 0,

} twr_led_strip_event_t;

typedef struct twr_led_strip_t twr_led_strip_t;

struct twr_led_strip_t
{
    const twr_led_strip_driver_t *_driver;
    const twr_led_strip_buffer_t *_buffer;

    struct
    {
        int led;
        int round;
        twr_tick_t wait;
        uint32_t color;
        twr_scheduler_task_id_t task_id;

    } _effect;
    uint8_t _brightness;
    void (*_event_handler)(twr_led_strip_t *, twr_led_strip_event_t, void *);
    void *_event_param;

};

void twr_led_strip_init(twr_led_strip_t *self, const twr_led_strip_driver_t *driver, const twr_led_strip_buffer_t *buffer);

void twr_led_strip_set_event_handler(twr_led_strip_t *self, void (*event_handler)(twr_led_strip_t *, twr_led_strip_event_t, void *), void *event_param);

int twr_led_strip_get_pixel_count(twr_led_strip_t *self);

twr_led_strip_type_t twr_led_strip_get_strip_type(twr_led_strip_t *self);

void twr_led_strip_set_pixel(twr_led_strip_t *self, int position, uint32_t color);

void twr_led_strip_set_pixel_rgbw(twr_led_strip_t *self, int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

bool twr_led_strip_set_rgbw_framebuffer(twr_led_strip_t *self, uint8_t *framebuffer, size_t length);

void twr_led_strip_fill(twr_led_strip_t *self, uint32_t color);

bool twr_led_strip_write(twr_led_strip_t *self);

bool twr_led_strip_is_ready(twr_led_strip_t *self);

void twr_led_strip_set_brightness(twr_led_strip_t *self, uint8_t brightness);

void twr_led_strip_effect_stop(twr_led_strip_t *self);

void twr_led_strip_effect_test(twr_led_strip_t *self);

void twr_led_strip_effect_rainbow(twr_led_strip_t *self, twr_tick_t wait);

// Slightly different, this makes the rainbow equally distributed throughout
void twr_led_strip_effect_rainbow_cycle(twr_led_strip_t *self, twr_tick_t wait);

// Fill the dots one after the other with a color
void twr_led_strip_effect_color_wipe(twr_led_strip_t *self, uint32_t color, twr_tick_t wait);

//Theatre-style crawling lights.
void twr_led_strip_effect_theater_chase(twr_led_strip_t *self, uint32_t color, twr_tick_t wait);

void twr_led_strip_effect_theater_chase_rainbow(twr_led_strip_t *self, twr_tick_t wait);

void twr_led_strip_effect_stroboscope(twr_led_strip_t *self, uint32_t color, twr_tick_t wait);

void twr_led_strip_effect_icicle(twr_led_strip_t *self, uint32_t color, twr_tick_t wait);

void twr_led_strip_effect_pulse_color(twr_led_strip_t *self, uint32_t color, twr_tick_t wait);

void twr_led_strip_thermometer(twr_led_strip_t *self, float temperature, float min, float max, uint8_t white_dots, float set_point, uint32_t color);

#endif // _TWR_LED_STRIP_H
