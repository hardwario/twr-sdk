#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <hio.h>

// Forward declarations

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param);

void temperature_tag_event_handler(hio_tag_temperature_t *self, hio_tag_temperature_event_t event, void *event_param);

void sigfox_module_event_handler(hio_module_sigfox_t *self, hio_module_sigfox_event_t event, void *event_param);

#endif // _APPLICATION_H
