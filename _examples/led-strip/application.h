#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <bcl.h>

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void led_strip_event_handler(bc_led_strip_t *self, bc_led_strip_event_t event, void *event_param);

#endif // _APPLICATION_H
