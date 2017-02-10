#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <bcl.h>

// Forward declarations

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param);

void sigfox_module_event_handler(bc_module_sigfox_t *self, bc_module_sigfox_event_t event, void *event_param);

#endif // _APPLICATION_H
