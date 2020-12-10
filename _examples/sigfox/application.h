#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <twr.h>

// Forward declarations

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param);

void temperature_tag_event_handler(twr_tag_temperature_t *self, twr_tag_temperature_event_t event, void *event_param);

void sigfox_module_event_handler(twr_module_sigfox_t *self, twr_module_sigfox_event_t event, void *event_param);

#endif // _APPLICATION_H
