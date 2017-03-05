#include <bc_tick.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

static bc_tick_t _bc_tick_counter = 0;

bc_tick_t bc_tick_get(void)
{
    bc_tick_t tick;

    // Disable interrupts
    bc_irq_disable();

    // Get current tick counter
    tick = _bc_tick_counter;

    // Enable interrupts
    bc_irq_enable();

    return tick;
}

void bc_tick_inrement_irq(bc_tick_t delta)
{
    _bc_tick_counter += delta;
}
