#include <bc_scheduler.h>
#include <bc_system.h>
#include <bc_error.h>

void application_init(void);

void application_task(void *param);

void application_error(bc_error_t code);

int main(void)
{
    bc_system_init();

    while (bc_tick_get() < 500)
    {
        continue;
    }

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

__attribute__((weak)) void application_error(bc_error_t code)
{
    (void) code;

#ifdef RELEASE

    bc_system_reset();

#else

    while (true)
    {
        continue;
    }

#endif
}
