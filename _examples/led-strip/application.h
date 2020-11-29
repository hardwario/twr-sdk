#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <hio.h>

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param);
void led_strip_event_handler(hio_led_strip_t *self, hio_led_strip_event_t event, void *event_param);

#endif // _APPLICATION_H
