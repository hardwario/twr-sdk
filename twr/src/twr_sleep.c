#include <twr_sleep.h>
#include <twr_system.h>
#include <twr_irq.h>

twr_sleep_manager_t sleep_manager = {
    .disable_sleep_semaphore = 0
};

void twr_sleep_disable(void)
{
    twr_irq_disable();
    sleep_manager.disable_sleep_semaphore++;
    twr_irq_enable();
}

void twr_sleep_enable(void)
{
    twr_irq_disable();
    sleep_manager.disable_sleep_semaphore--;
    twr_irq_enable();
}
