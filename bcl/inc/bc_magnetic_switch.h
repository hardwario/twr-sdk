#ifndef BC_MAGNETIC_SWITCH_H
#define BC_MAGNETIC_SWITCH_H

#include <bc_gpio.h>
#include <bc_tick.h>
#include <bc_scheduler.h>

typedef enum
{
    BC_MAGNETIC_SWITCH_EVENT_STATE_CHANGE

} bc_magnetic_switch_event_t;

typedef struct bc_magnetic_switch_t bc_magnetic_switch_t;

typedef enum
{
    BC_MAGNETIC_SWITCH_STATE_READY,
    BC_MAGNETIC_SWITCH_STATE_MEASURE,

} bc_magnetic_switch_state_t;

struct bc_magnetic_switch_t
{
    bc_gpio_channel_t _channel;
    bc_gpio_pull_t _gpio_pull;
    void (*_event_handler)(bc_magnetic_switch_t *, bc_magnetic_switch_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bc_magnetic_switch_state_t _state;
    int _pin_state;
    bc_scheduler_task_id_t _task_id;
    bc_tick_t _scan_interval;
    bc_tick_t _debounce_time;
    bc_tick_t _tick_debounce;

};

void bc_magnetic_switch_init(bc_magnetic_switch_t *self, bc_gpio_channel_t channel, bc_gpio_pull_t gpio_pull);

void bc_magnetic_switch_set_event_handler(bc_magnetic_switch_t *self, void (*event_handler)(bc_magnetic_switch_t *,bc_magnetic_switch_event_t, void*), void *event_param);

bool bc_magnetic_switch_is_open(bc_magnetic_switch_t *self);

void bc_magnetic_switch_set_scan_interval(bc_magnetic_switch_t *self, bc_tick_t scan_interval);

void bc_magnetic_switch_set_debounce_time(bc_magnetic_switch_t *self, bc_tick_t debounce_time);

#endif // BC_MAGNETIC_SWITCH_H
