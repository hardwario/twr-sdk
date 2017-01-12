#include <bc_rtc.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

void bc_rtc_init(void)
{
    // Disable backup write protection
    PWR->CR |= PWR_CR_DBP;

    while ((PWR->CR & PWR_CR_DBP) == 0)
    {
        continue;
    }

    RCC->CSR |= RCC_CSR_LSEON | RCC_CSR_LSEDRV_1;

    while((RCC->CSR & RCC_CSR_LSERDY) == 0)
    {
        continue;
    }

    // Enable clock, LSE as a clock
    RCC->CSR |= RCC_CSR_RTCEN | RCC_CSR_RTCSEL_LSE;
    RCC->CSR;

    bc_irq_disable();

    // Disable write protection
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    bc_irq_enable();

    // Enable init mode
    RTC->ISR |= RTC_ISR_INIT;

    // Wait for RTC to be in init mode
    while((RTC->ISR & RTC_ISR_INITF) == 0)
    {
        continue;
    }

    // Synch prediv
    RTC->PRER = 255;

    // Asynch prediv
    RTC->PRER |= (127UL) << 16UL;

    // Exit from init mode
    RTC->ISR &= ~RTC_ISR_INIT;

    // Enable NVIC IRQ
    NVIC_EnableIRQ(RTC_IRQn);

    // Disable WUTE
    RTC->CR &= ~RTC_CR_WUTE;

    // Wait until WUTE is disabled
    while((RTC->ISR & RTC_ISR_WUTWF) == 0)
    {
        continue;
    }

    RTC->WUTR = 63;

    // Clear WUTF
    RTC->ISR &= ~RTC_ISR_WUTF;

    // RTC IRQ need to be configured through EXTI
    EXTI->IMR |= EXTI_IMR_IM20;

    // Exti rising edge
    EXTI->RTSR |= EXTI_IMR_IM20;

    // Enable wake up timer
    RTC->CR |= RTC_CR_WUTIE | RTC_CR_WUTE;

    // Enable write protection
    RTC->WPR = 0xFF;
}

volatile uint32_t rtcCounter = 0;

void RTC_IRQHandler(void)
{
    // WUTR 20 = 10 ms

    // Check IRQ flag
    if (RTC->ISR & RTC_ISR_WUTF)
    {
        rtcCounter++;

        // Clear WUTF
        RTC->ISR &= ~RTC_ISR_WUTF;
    }

    // Clear EXTI flag
    EXTI->PR = EXTI_IMR_IM20;
}
