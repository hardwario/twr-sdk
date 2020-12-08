#include <stm32l0xx.h>

void twr_dfu_jump(void)
{
    // Set magic code, the rest is in the system_stm32l0xx.c file
    *((unsigned long *)0x20000000) = 0xDEADBEEF;

    // Reset the processor
    NVIC_SystemReset();
}
