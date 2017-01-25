#include <bc_scheduler.h>

static struct
{
    struct
    {
        bc_tick_t tick;
        bc_tick_t (*task)(void *, bc_tick_t);
        void *param;

    } pool[BC_SCHEDULER_MAX_TASKS];

    bc_scheduler_task_id_t max_task_id;

} bc_scheduler;

void bc_scheduler_init(void)
{
    memset(&bc_scheduler, 0, sizeof(bc_scheduler));
}

void bc_scheduler_run(void)
{
    while (true)
    {
        bc_tick_t tick_now = bc_tick_get();

        bc_tick_t max_tick = 0;

        for (bc_scheduler_task_id_t i = 0; i <= bc_scheduler.max_task_id; i++)
        {
            if (bc_scheduler.pool[i].task != NULL)
            {
                if (tick_now >= bc_scheduler.pool[i].tick)
                {
                    bc_scheduler.pool[i].tick = bc_scheduler.pool[i].task(bc_scheduler.pool[i].param, tick_now);

                    if (bc_scheduler.pool[i].tick > max_tick)
                    {
                        max_tick = bc_scheduler.pool[i].tick;
                    }
                }
            }
        }
    }
}

bc_scheduler_task_id_t bc_scheduler_register(bc_tick_t (*task)(void *, bc_tick_t), void *param, bc_tick_t tick)
{
    for (bc_scheduler_task_id_t i = 0; i < BC_SCHEDULER_MAX_TASKS; i++)
    {
        if (bc_scheduler.pool[i].task == NULL)
        {
            bc_scheduler.pool[i].tick = tick;
            bc_scheduler.pool[i].task = task;
            bc_scheduler.pool[i].param = param;

            if (bc_scheduler.max_task_id < i)
            {
                bc_scheduler.max_task_id = i;
            }

            return i;
        }
    }

    // TODO Indicate no more tasks available
    for (;;);
}

void bc_scheduler_unregister(bc_scheduler_task_id_t task_id)
{
    bc_scheduler.pool[task_id].task = NULL;

    if (bc_scheduler.max_task_id == task_id)
    {
        do
        {
            if (bc_scheduler.max_task_id == 0)
            {
                break;
            }

            bc_scheduler.max_task_id--;

        } while (bc_scheduler.pool[bc_scheduler.max_task_id].task == NULL);
    }
}

void bc_scheduler_plan(bc_scheduler_task_id_t task_id, bc_tick_t tick)
{
    bc_scheduler.pool[task_id].tick = tick;
}

void bc_scheduler_plan_now(bc_scheduler_task_id_t task_id)
{
    bc_scheduler.pool[task_id].tick = 0;
}
