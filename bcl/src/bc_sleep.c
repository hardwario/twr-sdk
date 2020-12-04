#include <bc_sleep.h>
#include <bc_system.h>

bc_sleep_manager_t sleep_manager = {
    .disable_sleep = 0
};

void bc_sleep_disable(void)
{
    sleep_manager.disable_sleep++;
}

void bc_sleep_enable(void)
{
    sleep_manager.disable_sleep--;
}
