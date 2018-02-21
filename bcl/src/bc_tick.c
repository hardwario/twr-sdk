#include <bc_tick.h>
#include <bc_irq.h>
#include <bc_system.h>
#include <stm32l0xx.h>

bc_tick_t bc_tick_get(void)
{
    return bc_system_tick_get();
}
