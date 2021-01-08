#include <twr_scheduler.h>
#include <twr_system.h>
#include <twr_error.h>

static struct
{
    struct
    {
        twr_tick_t tick_execution;
        void (*task)(void *);
        void *param;

    } pool[TWR_SCHEDULER_MAX_TASKS];

    twr_tick_t tick_spin;
    twr_scheduler_task_id_t current_task_id;
    twr_scheduler_task_id_t max_task_id;

} _twr_scheduler;

void application_idle();
void application_error(twr_error_t code);

void twr_scheduler_init(void)
{
    memset(&_twr_scheduler, 0, sizeof(_twr_scheduler));
}

void twr_scheduler_run(void)
{
    static twr_scheduler_task_id_t *task_id = &_twr_scheduler.current_task_id;

    while (true)
    {
        _twr_scheduler.tick_spin = twr_tick_get();

        for (*task_id = 0; *task_id <= _twr_scheduler.max_task_id; (*task_id)++)
        {
            if (_twr_scheduler.pool[*task_id].task != NULL)
            {
                if (_twr_scheduler.tick_spin >= _twr_scheduler.pool[*task_id].tick_execution)
                {
                    _twr_scheduler.pool[*task_id].tick_execution = TWR_TICK_INFINITY;

                    _twr_scheduler.pool[*task_id].task(_twr_scheduler.pool[*task_id].param);
                }
            }
        }
        application_idle();
    }
}

twr_scheduler_task_id_t twr_scheduler_register(void (*task)(void *), void *param, twr_tick_t tick)
{
    for (twr_scheduler_task_id_t i = 0; i < TWR_SCHEDULER_MAX_TASKS; i++)
    {
        if (_twr_scheduler.pool[i].task == NULL)
        {
            _twr_scheduler.pool[i].tick_execution = tick;
            _twr_scheduler.pool[i].task = task;
            _twr_scheduler.pool[i].param = param;

            if (_twr_scheduler.max_task_id < i)
            {
                _twr_scheduler.max_task_id = i;
            }

            return i;
        }
    }

    application_error(TWR_ERROR_NOT_ENOUGH_TASKS);

    return 0;
}

void twr_scheduler_unregister(twr_scheduler_task_id_t task_id)
{
    _twr_scheduler.pool[task_id].task = NULL;

    if (_twr_scheduler.max_task_id == task_id)
    {
        do
        {
            if (_twr_scheduler.max_task_id == 0)
            {
                break;
            }

            _twr_scheduler.max_task_id--;

        } while (_twr_scheduler.pool[_twr_scheduler.max_task_id].task == NULL);
    }
}

twr_scheduler_task_id_t twr_scheduler_get_current_task_id(void)
{
    return _twr_scheduler.current_task_id;
}

twr_tick_t twr_scheduler_get_spin_tick(void)
{
    return _twr_scheduler.tick_spin;
}

void twr_scheduler_plan_now(twr_scheduler_task_id_t task_id)
{
    _twr_scheduler.pool[task_id].tick_execution = 0;
}

void twr_scheduler_plan_absolute(twr_scheduler_task_id_t task_id, twr_tick_t tick)
{
    _twr_scheduler.pool[task_id].tick_execution = tick;
}

void twr_scheduler_plan_relative(twr_scheduler_task_id_t task_id, twr_tick_t tick)
{
    _twr_scheduler.pool[task_id].tick_execution = _twr_scheduler.tick_spin + tick;
}

void twr_scheduler_plan_from_now(twr_scheduler_task_id_t task_id, twr_tick_t tick)
{
    _twr_scheduler.pool[task_id].tick_execution = twr_tick_get() + tick;
}

void twr_scheduler_plan_current_now(void)
{
    _twr_scheduler.pool[_twr_scheduler.current_task_id].tick_execution = 0;
}

void twr_scheduler_plan_current_absolute(twr_tick_t tick)
{
    _twr_scheduler.pool[_twr_scheduler.current_task_id].tick_execution = tick;
}

void twr_scheduler_plan_current_relative(twr_tick_t tick)
{
    _twr_scheduler.pool[_twr_scheduler.current_task_id].tick_execution = _twr_scheduler.tick_spin + tick;
}

void twr_scheduler_plan_current_from_now(twr_tick_t tick)
{
    _twr_scheduler.pool[_twr_scheduler.current_task_id].tick_execution = twr_tick_get() + tick;
}
