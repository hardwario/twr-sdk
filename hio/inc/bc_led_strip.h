#ifndef _HIO_LED_STRIP_H
#define _HIO_LED_STRIP_H

#include <hio_scheduler.h>

typedef enum
{
    HIO_LED_STRIP_TYPE_RGBW = 4,
    HIO_LED_STRIP_TYPE_RGB = 3

} hio_led_strip_type_t;

typedef struct
{
    hio_led_strip_type_t type;
    int count;
    uint32_t *buffer;

} hio_led_strip_buffer_t;

typedef struct
{
    bool (*init)(const hio_led_strip_buffer_t *buffer);
    void (*set_pixel)(int position, uint32_t color);
    void (*set_pixel_rgbw)(int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    bool (*write)(void);
    bool (*is_ready)(void);

} hio_led_strip_driver_t;

typedef enum
{
    HIO_LED_STRIP_EVENT_EFFECT_DONE = 0,

} hio_led_strip_event_t;

typedef struct hio_led_strip_t hio_led_strip_t;

struct hio_led_strip_t
{
    const hio_led_strip_driver_t *_driver;
    const hio_led_strip_buffer_t *_buffer;

    struct
    {
        int led;
        int round;
        hio_tick_t wait;
        uint32_t color;
        hio_scheduler_task_id_t task_id;

    } _effect;
    uint8_t _brightness;
    void (*_event_handler)(hio_led_strip_t *, hio_led_strip_event_t, void *);
    void *_event_param;

};

void hio_led_strip_init(hio_led_strip_t *self, const hio_led_strip_driver_t *driver, const hio_led_strip_buffer_t *buffer);

void hio_led_strip_set_event_handler(hio_led_strip_t *self, void (*event_handler)(hio_led_strip_t *, hio_led_strip_event_t, void *), void *event_param);

int hio_led_strip_get_pixel_count(hio_led_strip_t *self);

hio_led_strip_type_t hio_led_strip_get_strip_type(hio_led_strip_t *self);

void hio_led_strip_set_pixel(hio_led_strip_t *self, int position, uint32_t color);

void hio_led_strip_set_pixel_rgbw(hio_led_strip_t *self, int position, uint8_t r, uint8_t g, uint8_t b, uint8_t w);

bool hio_led_strip_set_rgbw_framebuffer(hio_led_strip_t *self, uint8_t *framebuffer, size_t length);

void hio_led_strip_fill(hio_led_strip_t *self, uint32_t color);

bool hio_led_strip_write(hio_led_strip_t *self);

bool hio_led_strip_is_ready(hio_led_strip_t *self);

void hio_led_strip_set_brightness(hio_led_strip_t *self, uint8_t brightness);

void hio_led_strip_effect_stop(hio_led_strip_t *self);

void hio_led_strip_effect_test(hio_led_strip_t *self);

void hio_led_strip_effect_rainbow(hio_led_strip_t *self, hio_tick_t wait);

// Slightly different, this makes the rainbow equally distributed throughout
void hio_led_strip_effect_rainbow_cycle(hio_led_strip_t *self, hio_tick_t wait);

// Fill the dots one after the other with a color
void hio_led_strip_effect_color_wipe(hio_led_strip_t *self, uint32_t color, hio_tick_t wait);

//Theatre-style crawling lights.
void hio_led_strip_effect_theater_chase(hio_led_strip_t *self, uint32_t color, hio_tick_t wait);

void hio_led_strip_effect_theater_chase_rainbow(hio_led_strip_t *self, hio_tick_t wait);

void hio_led_strip_effect_stroboscope(hio_led_strip_t *self, uint32_t color, hio_tick_t wait);

void hio_led_strip_effect_icicle(hio_led_strip_t *self, uint32_t color, hio_tick_t wait);

void hio_led_strip_effect_pulse_color(hio_led_strip_t *self, uint32_t color, hio_tick_t wait);

void hio_led_strip_thermometer(hio_led_strip_t *self, float temperature, float min, float max, uint8_t white_dots, float set_point, uint32_t color);

#endif // _HIO_LED_STRIP_H
