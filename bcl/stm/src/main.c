#include <bc_scheduler.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

void application_init(void);
void application_task(void *param);

int main(void)
{
    bc_module_core_init();

    bc_scheduler_init();

    bc_scheduler_register(application_task, NULL, 0);

    application_init();

    bc_scheduler_run();
}

__attribute__((weak)) void application_init(void)
{

}

__attribute__((weak)) void application_task(void *param)
{
    (void) param;
}
