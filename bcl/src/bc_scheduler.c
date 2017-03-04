#include <bc_scheduler.h>
#include <bc_module_core.h>

static struct
{
    struct
    {
        bc_tick_t tick_execution;
        void (*task)(void *);
        void *param;

    } pool[BC_SCHEDULER_MAX_TASKS];

    bc_tick_t tick_spin;
    bc_scheduler_task_id_t current_task_id;
    bc_scheduler_task_id_t max_task_id;
    int sleep_bypass_semaphore;

} _bc_scheduler;

void bc_scheduler_init(void)
{
    memset(&_bc_scheduler, 0, sizeof(_bc_scheduler));
}

void bc_scheduler_run(void)
{
    static bc_scheduler_task_id_t *task_id = &_bc_scheduler.current_task_id;

    while (true)
    {
        _bc_scheduler.tick_spin = bc_tick_get();

        for (*task_id = 0; *task_id <= _bc_scheduler.max_task_id; (*task_id)++)
        {
            if (_bc_scheduler.pool[*task_id].task != NULL)
            {
                if (_bc_scheduler.tick_spin >= _bc_scheduler.pool[*task_id].tick_execution)
                {
                    _bc_scheduler.pool[*task_id].tick_execution = BC_TICK_INFINITY;

                    _bc_scheduler.pool[*task_id].task(_bc_scheduler.pool[*task_id].param);
                }
            }
        }
        if (_bc_scheduler.sleep_bypass_semaphore == 0)
        {
            bc_module_core_sleep();
        }
    }
}

bc_scheduler_task_id_t bc_scheduler_register(void (*task)(void *), void *param, bc_tick_t tick)
{
    for (bc_scheduler_task_id_t i = 0; i < BC_SCHEDULER_MAX_TASKS; i++)
    {
        if (_bc_scheduler.pool[i].task == NULL)
        {
            _bc_scheduler.pool[i].tick_execution = tick;
            _bc_scheduler.pool[i].task = task;
            _bc_scheduler.pool[i].param = param;

            if (_bc_scheduler.max_task_id < i)
            {
                _bc_scheduler.max_task_id = i;
            }

            return i;
        }
    }

    // TODO Indicate no more tasks available
    for (;;);
}

void bc_scheduler_unregister(bc_scheduler_task_id_t task_id)
{
    _bc_scheduler.pool[task_id].task = NULL;

    if (_bc_scheduler.max_task_id == task_id)
    {
        do
        {
            if (_bc_scheduler.max_task_id == 0)
            {
                break;
            }

            _bc_scheduler.max_task_id--;

        } while (_bc_scheduler.pool[_bc_scheduler.max_task_id].task == NULL);
    }
}

bc_scheduler_task_id_t bc_scheduler_get_current_task_id(void)
{
    return _bc_scheduler.current_task_id;
}

bc_tick_t bc_scheduler_get_spin_tick(void)
{
    return _bc_scheduler.tick_spin;
}

void bc_scheduler_disable_sleep(void)
{
    _bc_scheduler.sleep_bypass_semaphore++;
}

void bc_scheduler_enable_sleep(void)
{
    _bc_scheduler.sleep_bypass_semaphore--;
}

void bc_scheduler_plan_now(bc_scheduler_task_id_t task_id)
{
    _bc_scheduler.pool[task_id].tick_execution = 0;
}

void bc_scheduler_plan_absolute(bc_scheduler_task_id_t task_id, bc_tick_t tick)
{
    _bc_scheduler.pool[task_id].tick_execution = tick;
}

void bc_scheduler_plan_relative(bc_scheduler_task_id_t task_id, bc_tick_t tick)
{
    _bc_scheduler.pool[task_id].tick_execution = _bc_scheduler.tick_spin + tick;
}

void bc_scheduler_plan_current_now(void)
{
    _bc_scheduler.pool[_bc_scheduler.current_task_id].tick_execution = 0;
}

void bc_scheduler_plan_current_absolute(bc_tick_t tick)
{
    _bc_scheduler.pool[_bc_scheduler.current_task_id].tick_execution = tick;
}

void bc_scheduler_plan_current_relative(bc_tick_t tick)
{
    _bc_scheduler.pool[_bc_scheduler.current_task_id].tick_execution = _bc_scheduler.tick_spin + tick;
}
