#include <hio_scheduler.h>
#include <hio_system.h>
#include <hio_error.h>

static struct
{
    struct
    {
        hio_tick_t tick_execution;
        void (*task)(void *);
        void *param;

    } pool[HIO_SCHEDULER_MAX_TASKS];

    hio_tick_t tick_spin;
    hio_scheduler_task_id_t current_task_id;
    hio_scheduler_task_id_t max_task_id;
    int sleep_bypass_semaphore;

} _hio_scheduler;

void application_error(hio_error_t code);

void hio_scheduler_init(void)
{
    memset(&_hio_scheduler, 0, sizeof(_hio_scheduler));
}

void hio_scheduler_run(void)
{
    static hio_scheduler_task_id_t *task_id = &_hio_scheduler.current_task_id;

    while (true)
    {
        _hio_scheduler.tick_spin = hio_tick_get();

        for (*task_id = 0; *task_id <= _hio_scheduler.max_task_id; (*task_id)++)
        {
            if (_hio_scheduler.pool[*task_id].task != NULL)
            {
                if (_hio_scheduler.tick_spin >= _hio_scheduler.pool[*task_id].tick_execution)
                {
                    _hio_scheduler.pool[*task_id].tick_execution = HIO_TICK_INFINITY;

                    _hio_scheduler.pool[*task_id].task(_hio_scheduler.pool[*task_id].param);
                }
            }
        }
        if (_hio_scheduler.sleep_bypass_semaphore == 0)
        {
            hio_system_sleep();
        }
    }
}

hio_scheduler_task_id_t hio_scheduler_register(void (*task)(void *), void *param, hio_tick_t tick)
{
    for (hio_scheduler_task_id_t i = 0; i < HIO_SCHEDULER_MAX_TASKS; i++)
    {
        if (_hio_scheduler.pool[i].task == NULL)
        {
            _hio_scheduler.pool[i].tick_execution = tick;
            _hio_scheduler.pool[i].task = task;
            _hio_scheduler.pool[i].param = param;

            if (_hio_scheduler.max_task_id < i)
            {
                _hio_scheduler.max_task_id = i;
            }

            return i;
        }
    }

    application_error(HIO_ERROR_NOT_ENOUGH_TASKS);

    return 0;
}

void hio_scheduler_unregister(hio_scheduler_task_id_t task_id)
{
    _hio_scheduler.pool[task_id].task = NULL;

    if (_hio_scheduler.max_task_id == task_id)
    {
        do
        {
            if (_hio_scheduler.max_task_id == 0)
            {
                break;
            }

            _hio_scheduler.max_task_id--;

        } while (_hio_scheduler.pool[_hio_scheduler.max_task_id].task == NULL);
    }
}

hio_scheduler_task_id_t hio_scheduler_get_current_task_id(void)
{
    return _hio_scheduler.current_task_id;
}

hio_tick_t hio_scheduler_get_spin_tick(void)
{
    return _hio_scheduler.tick_spin;
}

void hio_scheduler_disable_sleep(void)
{
    _hio_scheduler.sleep_bypass_semaphore++;
}

void hio_scheduler_enable_sleep(void)
{
    _hio_scheduler.sleep_bypass_semaphore--;
}

void hio_scheduler_plan_now(hio_scheduler_task_id_t task_id)
{
    _hio_scheduler.pool[task_id].tick_execution = 0;
}

void hio_scheduler_plan_absolute(hio_scheduler_task_id_t task_id, hio_tick_t tick)
{
    _hio_scheduler.pool[task_id].tick_execution = tick;
}

void hio_scheduler_plan_relative(hio_scheduler_task_id_t task_id, hio_tick_t tick)
{
    _hio_scheduler.pool[task_id].tick_execution = _hio_scheduler.tick_spin + tick;
}

void hio_scheduler_plan_from_now(hio_scheduler_task_id_t task_id, hio_tick_t tick)
{
    _hio_scheduler.pool[task_id].tick_execution = hio_tick_get() + tick;
}

void hio_scheduler_plan_current_now(void)
{
    _hio_scheduler.pool[_hio_scheduler.current_task_id].tick_execution = 0;
}

void hio_scheduler_plan_current_absolute(hio_tick_t tick)
{
    _hio_scheduler.pool[_hio_scheduler.current_task_id].tick_execution = tick;
}

void hio_scheduler_plan_current_relative(hio_tick_t tick)
{
    _hio_scheduler.pool[_hio_scheduler.current_task_id].tick_execution = _hio_scheduler.tick_spin + tick;
}

void hio_scheduler_plan_current_from_now(hio_tick_t tick)
{
    _hio_scheduler.pool[_hio_scheduler.current_task_id].tick_execution = hio_tick_get() + tick;
}
