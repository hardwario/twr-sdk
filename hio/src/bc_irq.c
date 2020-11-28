#include <hio_irq.h>
#include <stm32l0xx.h>

static volatile uint32_t _hio_irq_primask = 0;
static volatile uint32_t _hio_irq_disable = 0;

void hio_irq_disable(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    if (_hio_irq_disable == 0)
    {
        _hio_irq_primask = primask & 1;
    }

    _hio_irq_disable++;
}

void hio_irq_enable(void)
{
    if (_hio_irq_disable != 0)
    {
        _hio_irq_disable--;

        if (_hio_irq_disable == 0 && _hio_irq_primask == 0)
        {
            __enable_irq();
        }
    }
}
