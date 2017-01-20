#ifndef _BC_SCHEDULER_H
#define _BC_SCHEDULER_H

#include <bc_common.h>
#include <bc_tick.h>

//! @addtogroup bc_scheduler bc_scheduler
//! @brief Task scheduler
//! @{

//! @brief Maximum number of tasks
#define BC_SCHEDULER_MAX_TASKS 32

typedef size_t bc_scheduler_task_id_t;

//! @brief Initialize task scheduler

void bc_scheduler_init(void);

//! @brief Run task scheduler

void bc_scheduler_run(void);

//! @brief Register new task to the scheduler
//! @param[in] task Pointer to the task function
//! @param[in] param Optional parameter pointer which will be passed to the registered task
//! @return Assigned task ID

bc_scheduler_task_id_t bc_scheduler_register(bc_tick_t (*task)(void *), void *param, bc_tick_t tick);

//! @brief Unregister previously registered task
//! @param[in] task_id Task ID of the registered task

void bc_scheduler_unregister(bc_scheduler_task_id_t task_id);

//! @brief Plan previously registered task to a given time
//! @param[in] task_id Task ID of the registered task
//! @param[in] tick Tick at which the task will be run

void bc_scheduler_plan(bc_scheduler_task_id_t task_id, bc_tick_t tick);

//! @}

#endif
