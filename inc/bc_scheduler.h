#ifndef _BC_SCHEDULER_H
#define _BC_SCHEDULER_H

#include <bc_common.h>
#include <bc_tick.h>

#define BC_SCHEDULER_MAX_TASKS 32

void bc_scheduler_init(void);
void bc_scheduler_run(void);
void bc_scheduler_register(bc_tick_t (*task)(void *), void *param, bc_tick_t tick);
void bc_scheduler_unregister(bc_tick_t (*task)(void *), void *param);

#endif
