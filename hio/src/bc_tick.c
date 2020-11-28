#include <hio_tick.h>
#include <hio_irq.h>
#include <stm32l0xx.h>

static volatile hio_tick_t _hio_tick_counter = 0;

hio_tick_t hio_tick_get(void)
{
    hio_tick_t tick;

    // Disable interrupts
    hio_irq_disable();

    // Get current tick counter
    tick = _hio_tick_counter;

    // Enable interrupts
    hio_irq_enable();

    return tick;
}

void hio_tick_wait(hio_tick_t delay)
{
    hio_tick_t timeout = hio_tick_get() + delay;

    while (hio_tick_get() < timeout)
    {
        continue;
    }
}

void hio_tick_increment_irq(hio_tick_t delta)
{
    _hio_tick_counter += delta;
}
