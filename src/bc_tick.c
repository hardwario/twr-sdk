#include <bc_tick.h>
#include <stm32l0xx.h>

bc_tick_t bc_tick_get(void)
{
    return (bc_tick_t) HAL_GetTick();
}
