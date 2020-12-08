#include <twr_irq.h>
#include <stm32l0xx.h>

static volatile uint32_t _twr_irq_primask = 0;
static volatile uint32_t _twr_irq_disable = 0;

void twr_irq_disable(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    if (_twr_irq_disable == 0)
    {
        _twr_irq_primask = primask & 1;
    }

    _twr_irq_disable++;
}

void twr_irq_enable(void)
{
    if (_twr_irq_disable != 0)
    {
        _twr_irq_disable--;

        if (_twr_irq_disable == 0 && _twr_irq_primask == 0)
        {
            __enable_irq();
        }
    }
}
