#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <twr.h>

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param);
void led_strip_event_handler(twr_led_strip_t *self, twr_led_strip_event_t event, void *event_param);

#endif // _APPLICATION_H
