#ifndef _BC_MODULE_BATTERY_H
#define _BC_MODULE_BATTERY_H

#include <bc_tick.h>

typedef enum
{
    BC_MODULE_BATTERY_FORMAT_STANDARD = 0,
    BC_MODULE_BATTERY_FORMAT_MINI = 1

} bc_module_battery_format_t;

typedef enum
{
    BC_MODULE_BATTERY_EVENT_LEVEL_LOW = 0,
    BC_MODULE_BATTERY_EVENT_LEVEL_CRITICAL = 1,
    BC_MODULE_BATTERY_EVENT_UPDATE = 2

} bc_module_battery_event_t;

void bc_module_battery_init(bc_module_battery_format_t format);

void bc_module_battery_set_event_handler(void (*event_handler)(bc_module_battery_event_t, void *), void *event_param);

void bc_module_battery_set_update_interval(bc_tick_t interval);

void bc_module_battery_measure(void);

bool bc_module_battery_get_voltage_volt(float *voltage);

#endif // _BC_MODULE_BATTERY_H
