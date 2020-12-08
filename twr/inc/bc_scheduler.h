#ifndef _TWR_SCHEDULER_H
#define _TWR_SCHEDULER_H

#include <twr_tick.h>

//! @addtogroup twr_scheduler twr_scheduler
//! @brief Task scheduler
//! @{

//! @brief Maximum number of tasks

#ifndef TWR_SCHEDULER_MAX_TASKS
#define TWR_SCHEDULER_MAX_TASKS 32
#endif

#ifndef TWR_SCHEDULER_INTERVAL_MS
#define TWR_SCHEDULER_INTERVAL_MS 10
#endif

//! @brief Task ID assigned by scheduler

typedef size_t twr_scheduler_task_id_t;

//! @brief Initialize task scheduler

void twr_scheduler_init(void);

//! @brief Run task scheduler (this call never ends)

void twr_scheduler_run(void);

//! @brief Register task in scheduler
//! @param[in] task Task function address
//! @param[in] param Optional parameter which is passed to task function (can be NULL)
//! @param[in] tick Absolute tick when task will be scheduled
//! @return Assigned task ID

twr_scheduler_task_id_t twr_scheduler_register(void (*task)(void *), void *param, twr_tick_t tick);

//! @brief Unregister specified task
//! @param[in] task_id Task ID to be unregistered

void twr_scheduler_unregister(twr_scheduler_task_id_t task_id);

//! @brief Get task ID of currently executing task
//! @return Task ID

twr_scheduler_task_id_t twr_scheduler_get_current_task_id(void);

//! @brief Get current tick of spin in which task has been run
//! @return Tick of spin

twr_tick_t twr_scheduler_get_spin_tick(void);

//! @brief Disable sleep mode, implemented as semaphore

void twr_scheduler_disable_sleep(void);

//! @brief Enable sleep mode, implemented as semaphore

void twr_scheduler_enable_sleep(void);

//! @brief Schedule specified task for immediate execution
//! @param[in] task_id Task ID to be scheduled

void twr_scheduler_plan_now(twr_scheduler_task_id_t task_id);

//! @brief Schedule specified task to absolute tick
//! @param[in] task_id Task ID to be scheduled
//! @param[in] tick Tick at which the task will be run

void twr_scheduler_plan_absolute(twr_scheduler_task_id_t task_id, twr_tick_t tick);

//! @brief Schedule specified task to tick relative from current spin
//! @param[in] task_id Task ID to be scheduled
//! @param[in] tick Tick at which the task will be run as a relative value from current spin

void twr_scheduler_plan_relative(twr_scheduler_task_id_t task_id, twr_tick_t tick);

//! @brief Schedule specified task to tick relative from now
//! @param[in] task_id Task ID to be scheduled
//! @param[in] tick Tick at which the task will be run as a relative value from now

void twr_scheduler_plan_from_now(twr_scheduler_task_id_t task_id, twr_tick_t tick);

//! @brief Schedule current task for immediate execution

void twr_scheduler_plan_current_now(void);

//! @brief Schedule current task to absolute tick
//! @param[in] tick Tick at which the task will be run

void twr_scheduler_plan_current_absolute(twr_tick_t tick);

//! @brief Schedule current task to tick relative from current spin
//! @param[in] tick Tick at which the task will be run as a relative value from current spin

void twr_scheduler_plan_current_relative(twr_tick_t tick);

//! @brief Schedule current task to tick relative from now
//! @param[in] tick Tick at which the task will be run as a relative value from now

void twr_scheduler_plan_current_from_now(twr_tick_t tick);

//! @}

#endif // _TWR_SCHEDULER_H
