#include <bc_irq.h>
#include <stm32l0xx.h>

static volatile uint32_t _bc_irq_primask = 0;
static volatile uint32_t _bc_irq_disable = 0;

void bc_irq_disable(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    if (_bc_irq_disable == 0)
    {
        _bc_irq_primask = primask & 1;
    }

    _bc_irq_disable++;
}

void bc_irq_enable(void)
{
    if (_bc_irq_disable != 0)
    {
        _bc_irq_disable--;

        if (_bc_irq_disable == 0 && _bc_irq_primask == 0)
        {
            __enable_irq();
        }
    }
}
