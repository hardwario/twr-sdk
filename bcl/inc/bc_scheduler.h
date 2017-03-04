#ifndef _BC_SCHEDULER_H
#define _BC_SCHEDULER_H

#include <bc_tick.h>

//! @addtogroup bc_scheduler bc_scheduler
//! @brief Task scheduler
//! @{

//! @brief Maximum number of tasks

#define BC_SCHEDULER_MAX_TASKS 32

//! @brief Task ID assigned by scheduler

typedef size_t bc_scheduler_task_id_t;

//! @brief Initialize task scheduler

void bc_scheduler_init(void);

//! @brief Run task scheduler (this call never ends)

void bc_scheduler_run(void);

//! @brief Register task in scheduler
//! @param[in] task Task function address
//! @param[in] param Optional parameter which is passed to task function (can be NULL)
//! @param[in] tick Absolute tick when task will be scheduled
//! @return Assigned task ID

bc_scheduler_task_id_t bc_scheduler_register(void (*task)(void *), void *param, bc_tick_t tick);

//! @brief Unregister specified task
//! @param[in] task_id Task ID to be unregistered

void bc_scheduler_unregister(bc_scheduler_task_id_t task_id);

//! @brief Get task ID of currently executing task
//! @return Task ID

bc_scheduler_task_id_t bc_scheduler_get_current_task_id(void);

//! @brief Get current tick of spin in which task has been run
//! @return Tick of spin

bc_tick_t bc_scheduler_get_spin_tick(void);

//! @brief Disable sleep mode, implemented as semaphore

void bc_scheduler_disable_sleep(void);

//! @brief Enable sleep mode, implemented as semaphore

void bc_scheduler_enable_sleep(void);

//! @brief Schedule specified task for immediate execution
//! @param[in] task_id Task ID to be scheduled

void bc_scheduler_plan_now(bc_scheduler_task_id_t task_id);

//! @brief Schedule specified task to absolute tick
//! @param[in] task_id Task ID to be scheduled
//! @param[in] tick Tick at which the task will be run

void bc_scheduler_plan_absolute(bc_scheduler_task_id_t task_id, bc_tick_t tick);

//! @brief Schedule specified task to tick relative from current spin
//! @param[in] task_id Task ID to be scheduled
//! @param[in] tick Tick at which the task will be run as a relative value from current spin

void bc_scheduler_plan_relative(bc_scheduler_task_id_t task_id, bc_tick_t tick);

//! @brief Schedule current task for immediate execution

void bc_scheduler_plan_current_now(void);

//! @brief Schedule current task to absolute tick
//! @param[in] tick Tick at which the task will be run

void bc_scheduler_plan_current_absolute(bc_tick_t tick);

//! @brief Schedule current task to tick relative from current spin
//! @param[in] tick Tick at which the task will be run as a relative value from current spin

void bc_scheduler_plan_current_relative(bc_tick_t tick);

//! @}

#endif
