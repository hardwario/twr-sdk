#include <twr_sleep.h>
#include <twr_system.h>

twr_sleep_manager_t sleep_manager = {
    .disable_sleep = 0
};

void twr_sleep_disable(void)
{
    sleep_manager.disable_sleep++;
}

void twr_sleep_enable(void)
{
    sleep_manager.disable_sleep--;
}
