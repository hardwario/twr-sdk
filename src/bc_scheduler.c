#include <bc_scheduler.h>

static struct
{
    struct
    {
        bc_tick_t tick;
        bc_tick_t (*task)(void *);
        void *param;

    } pool[BC_SCHEDULER_MAX_TASKS];

    size_t count;

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

        for (size_t i = 0; i < bc_scheduler.count; i++)
        {
            if (bc_scheduler.pool[i].task != NULL)
            {
                if (tick_now >= bc_scheduler.pool[i].tick)
                {
                    bc_tick_t tick = bc_scheduler.pool[i].task(bc_scheduler.pool[i].param);

                    if (tick > max_tick)
                    {
                        max_tick = tick;
                    }

                    bc_scheduler.pool[i].tick = tick_now + tick;
                }
            }
        }
    }
}

void bc_scheduler_register(bc_tick_t (*task)(void *), void *param, bc_tick_t tick)
{
    if (bc_scheduler.count >= BC_SCHEDULER_MAX_TASKS)
    {
        // TODO Replace
        for (;;);
    }

    bc_scheduler.pool[bc_scheduler.count].tick = tick;
    bc_scheduler.pool[bc_scheduler.count].task = task;
    bc_scheduler.pool[bc_scheduler.count].param = param;

    bc_scheduler.count++;
}

void bc_scheduler_unregister(bc_tick_t (*task)(void *), void *param)
{
  (void) task;
  (void) param;

  // TODO Implement this
}
