#include <twr_tick.h>
#include <twr_irq.h>
#include <stm32l0xx.h>

static volatile twr_tick_t _twr_tick_counter = 0;

twr_tick_t twr_tick_get(void)
{
    twr_tick_t tick;

    // Disable interrupts
    twr_irq_disable();

    // Get current tick counter
    tick = _twr_tick_counter;

    // Enable interrupts
    twr_irq_enable();

    return tick;
}

void twr_tick_wait(twr_tick_t delay)
{
    twr_tick_t timeout = twr_tick_get() + delay;

    while (twr_tick_get() < timeout)
    {
        continue;
    }
}

void twr_tick_increment_irq(twr_tick_t delta)
{
    _twr_tick_counter += delta;
}
