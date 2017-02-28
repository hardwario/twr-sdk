#include <bc_module_core.h>
#include <bc_tick.h>
#include <stm32l0xx.h>

#define DEBUG_ENABLE 0

static void _bc_module_core_init_flash(void);

static void _bc_module_core_init_debug(void);

static void _bc_module_core_init_clock(void);

static void _bc_module_core_init_power(void);

static void _bc_module_core_init_gpio(void);

static void _bc_module_core_init_rtc(void);

void bc_module_core_init(void)
{
    _bc_module_core_init_flash();

    _bc_module_core_init_debug();

    _bc_module_core_init_clock();

    _bc_module_core_init_power();

    _bc_module_core_init_gpio();

    _bc_module_core_init_rtc();
}

static void _bc_module_core_init_flash(void)
{
    // Enable prefetch
    FLASH->ACR |= FLASH_ACR_PRFTEN;

    // One wait state is used to read word from NVM
    FLASH->ACR |= FLASH_ACR_LATENCY;
}

static void _bc_module_core_init_debug(void)
{
#if DEBUG_ENABLE == 1

    // Enable clock for DBG
    RCC->APB2ENR |= RCC_APB2ENR_DBGMCUEN;

    // Errata workaround
    RCC->APB2ENR;

    // Enable debug in Standby mode
    DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;

    // Enable debug in Stop mode
    DBGMCU->CR |= DBGMCU_CR_DBG_STOP;

    // Enable debug in Sleep mode
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;

    // LPTIM1 counter stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_LPTIMER_STOP;

    // I2C3 SMBUS timeout mode stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_I2C3_STOP;

    // I2C1 SMBUS timeout mode stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_I2C1_STOP;

    // Debug independent watchdog stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP;

    // Debug window watchdog stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_WWDG_STOP;

    // Debug RTC stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_RTC_STOP;

    // TIM7 counter stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM7_STOP;

    // TIM6 counter stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM6_STOP;

    // TIM3 counter stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM3_STOP;

    // TIM2 counter stopped when core is halted
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM2_STOP;

    // TIM22 counter stopped when core is halted
    DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM22_STOP;

    // TIM21 counter stopped when core is halted
    DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM21_STOP;

#endif
}

static void _bc_module_core_init_clock(void)
{
    // Enable HSI16 oscillator
    RCC->CR |= RCC_CR_HSION;

    // Wait until HSI16 oscillator is ready...
    while ((RCC->CR & RCC_CR_HSIRDY) == 0)
    {
        continue;
    }

    // Use HSI16 oscillator as system clock
    RCC->CFGR |= RCC_CFGR_SW_0;

    // Wait until HSI16 oscillator is used as system clock...
    while ((RCC->CFGR & (RCC_CFGR_SWS_1 | RCC_CFGR_SWS_0)) != RCC_CFGR_SWS_0)
    {
        continue;
    }

    // Update SystemCoreClock variable
    SystemCoreClock = 16000000;

    // Wake-up clock is from HSI16 oscillator
    RCC->CFGR |= RCC_CFGR_STOPWUCK;

    // Disable MSI oscillator
    RCC->CR &= ~RCC_CR_MSION;

    // Set SysTick reload value
    SysTick->LOAD = 16000 - 1;

    // Reset SysTick counter
    SysTick->VAL = 0;

    // Use processor clock as SysTick clock source
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

    // Enable SysTick interrupt
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    // Enable SysTick counter
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

static void _bc_module_core_init_power(void)
{
    // Enable clock for PWR
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Errata workaround
    RCC->APB1ENR;

    // Disable backup write protection
    PWR->CR |= PWR_CR_DBP;

    // Enable fast wake-up
    PWR->CR |= PWR_CR_FWU;

    // Enable ultra-low-power mode
    PWR->CR |= PWR_CR_ULP;

    // Enable regulator low-power mode
    PWR->CR |= PWR_CR_LPSDSR;
}

static void _bc_module_core_init_gpio(void)
{
    // Enable clock for GPIOA
    RCC->IOPENR = RCC_IOPENR_GPIOAEN;

    // Errata workaround
    RCC->IOPENR;

    // Set analog mode on PA4
    GPIOA->MODER |= GPIO_MODER_MODE4_1 | GPIO_MODER_MODE4_0;
}

static void _bc_module_core_init_rtc(void)
{
    // Set LSE oscillator drive capability to medium low drive
    RCC->CSR |= RCC_CSR_LSEDRV_1;

    // Enable LSE oscillator
    RCC->CSR |= RCC_CSR_LSEON;

    // Wait for LSE oscillator to be ready...
    while ((RCC->CSR & RCC_CSR_LSERDY) == 0)
    {
        continue;
    }

    // LSE oscillator clock used as RTC clock
    RCC->CSR |= RCC_CSR_RTCSEL_LSE;

    // Enable RTC clock
    RCC->CSR |= RCC_CSR_RTCEN;

    // Errata workaround
    RCC->CSR;

    // Disable write protection
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;

    // Enable initialization mode
    RTC->ISR |= RTC_ISR_INIT;

    // Wait for RTC to be in initialization mode...
    while ((RTC->ISR & RTC_ISR_INITF) == 0)
    {
        continue;
    }

    // Set RTC prescaler
    RTC->PRER = (127 << 16) | 255;

    // Exit from initialization mode
    RTC->ISR &= ~RTC_ISR_INIT;

    // Enable RTC interrupt requests
    NVIC_EnableIRQ(RTC_IRQn);

    // Enable timer
    RTC->CR &= ~RTC_CR_WUTE;

    // Wait until timer configuration update is allowed...
    while ((RTC->ISR & RTC_ISR_WUTWF) == 0)
    {
        continue;
    }

    // Set wake-up auto-reload value
    RTC->WUTR = 20;

    // Clear timer flag
    RTC->ISR &= ~RTC_ISR_WUTF;

    // RTC IRQ needs to be configured through EXTI
    EXTI->IMR |= EXTI_IMR_IM20;

    // Enable rising edge trigger
    EXTI->RTSR |= EXTI_IMR_IM20;

    // Enable timer interrupts
    RTC->CR |= RTC_CR_WUTIE;

    // Enable timer
    RTC->CR |= RTC_CR_WUTE;

    // Enable write protection
    RTC->WPR = 0xff;
}

void bc_module_core_sleep()
{
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    __WFI();
}

void RTC_IRQHandler(void)
{
    // If wake-up timer flag is set...
    if (RTC->ISR & RTC_ISR_WUTF)
    {
        // Clear wake-up timer flag
        RTC->ISR &= ~RTC_ISR_WUTF;

        bc_tick_inrement_irq(10);
    }

    // Clear EXTI interrupt flag
    EXTI->PR = EXTI_IMR_IM20;
}
