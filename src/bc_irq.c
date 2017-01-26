#include <bc_irq.h>
#include <stm32l0xx.h>

void bc_irq_disable(void)
{
    __disable_irq();
}

void bc_irq_enable(void)
{
    __enable_irq();
}
