
#include <bc_rtc.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

void bc_rtc_init(void)
{
    // Disable interrupts
    bc_irq_disable();

    // Disable backup write protection
    PWR->CR |= PWR_CR_DBP;

    while((PWR->CR & PWR_CR_DBP) == 0)
    {
        continue;
    }

    RCC->CSR |= RCC_CSR_LSEON | RCC_CSR_LSEDRV_1;

    while((RCC->CSR & RCC_CSR_LSERDY) == 0);



    //RCC->CSR |= RCC_CSR_RTCRST;
    //RCC->CSR &= ~RCC_CSR_RTCRST;

    //while((RCC->CSR & RCC_CSR_LSERDY) == 0);

    // Enable clock, LSE as a clock
    RCC->CSR |= RCC_CSR_RTCEN | RCC_CSR_RTCSEL_LSE;
    RTC->WUTR;

    // Disable write protection
    RTC->WPR = 0xCAU;
    RTC->WPR = 0x53U;

    // Enable init mode
    RTC->ISR |= RTC_ISR_INIT;

    // Wait for RTC to be in init mode
    while ((RTC->ISR & RTC_ISR_INITF) == 0UL)
    {
        continue;
    }

    // Synch prediv
    RTC->PRER = 255;

    // Asynch prediv
    RTC->PRER |= 127UL << 16UL;

    NVIC_EnableIRQ(RTC_IRQn);

    RTC->WUTR = 5;

    // RTC IRQ need to be configured through EXTI
    EXTI->IMR |= EXTI_IMR_IM20;

    // Exti rising edge
    EXTI->RTSR |= EXTI_IMR_IM20;

    // Enable wake up timer interrupt
    RTC->CR |= RTC_CR_WUTIE | RTC_CR_WUTE;

    // Exit from init mode
    RTC->ISR &= ~RTC_ISR_INIT;

    RTC->WPR = 0xFF;

    // Enable interrupts
    bc_irq_enable();
}

volatile uint32_t rtcCounter = 0;

void RTC_IRQHandler(void)
{
    rtcCounter++;
}
