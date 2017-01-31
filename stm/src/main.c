#include <bc_scheduler.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

void application_init(void);
bc_tick_t application_task(void *param, bc_tick_t tick_now);

int main(void)
{
    bc_module_core_init();

    bc_scheduler_init();

    application_init();

    bc_scheduler_register(application_task, NULL, 0);

    bc_scheduler_run();
}

__attribute__((weak)) void application_init(void)
{
}

__attribute__((weak)) bc_tick_t application_task(void *param, bc_tick_t tick_now)
{
    (void) param;
    (void) tick_now;

    return BC_TICK_INFINITY;
}
