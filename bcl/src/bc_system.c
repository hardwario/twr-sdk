#include <bc_system.h>
#include <bc_scheduler.h>
#include <bc_irq.h>
#include <bc_i2c.h>
#include <bc_timer.h>
#include <stm32l0xx.h>

#define _BC_SYSTEM_DEBUG_ENABLE 0

static const uint32_t bc_system_clock_table[3] =
{
    RCC_CFGR_SW_MSI,
    RCC_CFGR_SW_HSI,
    RCC_CFGR_SW_PLL
};

static int _bc_system_hsi16_enable_semaphore;

static int _bc_system_pll_enable_semaphore;

static int _bc_system_deep_sleep_disable_semaphore;

static void _bc_system_init_flash(void);

static void _bc_system_init_debug(void);

static void _bc_system_init_clock(void);

static void _bc_system_init_power(void);

static void _bc_system_init_gpio(void);

static void _bc_system_init_rtc(void);

static void _bc_system_init_shutdown_tmp112(void);

static void _bc_system_switch_clock(bc_system_clock_t clock);

void bc_system_init(void)
{
    _bc_system_init_flash();

    _bc_system_init_debug();

    _bc_system_init_clock();

    _bc_system_init_power();

    _bc_system_init_gpio();

    _bc_system_init_rtc();

    _bc_system_init_shutdown_tmp112();
}

static void _bc_system_init_flash(void)
{
    // Enable prefetch
    FLASH->ACR |= FLASH_ACR_PRFTEN;

    // One wait state is used to read word from NVM
    FLASH->ACR |= FLASH_ACR_LATENCY;
}

static void _bc_system_init_debug(void)
{
#if _BC_SYSTEM_DEBUG_ENABLE == 1

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

static void _bc_system_init_clock(void)
{

    // Update SystemCoreClock variable
    SystemCoreClock = 2097000;

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    // Set regulator range to 1.2V
    PWR->CR |= PWR_CR_VOS;

    // Set PLL divider and multiplier
    RCC->CFGR = RCC_CFGR_PLLDIV2 | RCC_CFGR_PLLMUL4;

    // Set SysTick reload value
    SysTick->LOAD = 2097 - 1;

    // Reset SysTick counter
    SysTick->VAL = 0;

    // Use processor clock as SysTick clock source
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

    // Enable SysTick interrupt
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    // Enable SysTick counter
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

static void _bc_system_init_power(void)
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

    // Enable deep-sleep
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}

static void _bc_system_init_gpio(void)
{
    // Enable clock for GPIOA
    RCC->IOPENR = RCC_IOPENR_GPIOAEN;

    // Errata workaround
    RCC->IOPENR;

    // Set analog mode on PA4
    GPIOA->MODER |= GPIO_MODER_MODE4_1 | GPIO_MODER_MODE4_0;
}

static void _bc_system_init_rtc(void)
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

    // Initialize RTC only once
    if ((RTC->ISR & RTC_ISR_INITS) == 0)
    {

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

    }

    // Disable timer
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

    // Enable timer interrupts
    RTC->CR |= RTC_CR_WUTIE;

    // Enable timer
    RTC->CR |= RTC_CR_WUTE;

    // Enable write protection
    RTC->WPR = 0xff;


    // RTC IRQ needs to be configured through EXTI
    EXTI->IMR |= EXTI_IMR_IM20;

    // Enable rising edge trigger
    EXTI->RTSR |= EXTI_IMR_IM20;

    // Enable RTC interrupt requests
    NVIC_EnableIRQ(RTC_IRQn);
}

static void _bc_system_init_shutdown_tmp112(void)
{
    bc_i2c_init(BC_I2C_I2C0, BC_I2C_SPEED_100_KHZ);

    bc_i2c_memory_write_16b(BC_I2C_I2C0, 0x49, 0x01, 0x0180);

    bc_i2c_deinit(BC_I2C_I2C0);
}

void bc_system_sleep(void)
{
    __WFI();
}

void bc_system_deep_sleep_enable(void)
{
    _bc_system_deep_sleep_disable_semaphore--;

    if (_bc_system_deep_sleep_disable_semaphore == 0)
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }
}

void bc_system_deep_sleep_disable(void)
{
    if (_bc_system_deep_sleep_disable_semaphore == 0)
    {
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }

    _bc_system_deep_sleep_disable_semaphore++;
}

void bc_system_enter_standby_mode(void)
{
    bc_i2c_init(BC_I2C_I2C0, BC_I2C_SPEED_100_KHZ);

    // tmp112
    bc_i2c_memory_write_16b(BC_I2C_I2C0, 0x49, 0x01, 0x0180);

    // lis2dh12
    bc_i2c_memory_write_16b(BC_I2C_I2C0, 0x19, 0x20, 0x07);

    bc_i2c_deinit(BC_I2C_I2C0);

    __disable_irq();

    GPIOA->MODER = 0xFFFFFFFF;
    GPIOB->MODER = 0xFFFFFFFF;
    GPIOC->MODER = 0xFFFFFFFF;
    GPIOH->MODER = 0xFFFFFFFF;

    // Disable RTC clock
    RCC->CSR &= ~(RCC_CSR_RTCEN | RCC_CSR_LSEON | RCC_CSR_RTCSEL_LSE);

    // Errata workaround
    RCC->CSR;

    RCC->CSR &= ~RCC_CSR_LSEDRV_Msk;

    PWR->CR &= ~PWR_CR_LPSDSR;

    PWR->CR |= PWR_CR_PDDS;

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    PWR->CR |= PWR_CR_CWUF;

    __WFI();
}

bc_system_clock_t bc_system_clock_get(void)
{
    if (_bc_system_pll_enable_semaphore != 0)
    {
        return BC_SYSTEM_CLOCK_PLL;
    }
    else if (_bc_system_hsi16_enable_semaphore != 0)
    {
        return BC_SYSTEM_CLOCK_HSI;
    }
    else
    {
        return BC_SYSTEM_CLOCK_MSI;
    }
}

void bc_system_hsi16_enable(void)
{
    if (++_bc_system_hsi16_enable_semaphore == 1)
    {
        // Set regulator range to 1.8V
        PWR->CR |= PWR_CR_VOS_0;
        PWR->CR &= ~PWR_CR_VOS_1;

        // Enable flash latency and preread
        FLASH->ACR |= FLASH_ACR_LATENCY | FLASH_ACR_PRE_READ;

        // Turn HSI16 on
        RCC->CR |= RCC_CR_HSION;

        while ((RCC->CR & RCC_CR_HSIRDY) == 0)
        {
            continue;
        }

        _bc_system_switch_clock(BC_SYSTEM_CLOCK_HSI);

        // Set SysTick reload value
        SysTick->LOAD = 16000 - 1;

        // Update SystemCoreClock variable
        SystemCoreClock = 16000000;
    }

    bc_scheduler_disable_sleep();
}

void bc_system_hsi16_disable(void)
{
    if (--_bc_system_hsi16_enable_semaphore == 0)
    {
        _bc_system_switch_clock(BC_SYSTEM_CLOCK_MSI);

        // Turn HSI16 off
        RCC->CR &= ~RCC_CR_HSION;

        while ((RCC->CR & RCC_CR_HSIRDY) != 0)
        {
            continue;
        }

        // Set SysTick reload value
        SysTick->LOAD = 2097 - 1;

        // Update SystemCoreClock variable
        SystemCoreClock = 2097000;

        // Disable latency
        FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRE_READ);

        // Set regulator range to 1.2V
        PWR->CR |= PWR_CR_VOS;
    }

    bc_scheduler_enable_sleep();
}

void bc_system_pll_enable(void)
{
    if (++_bc_system_pll_enable_semaphore == 1)
    {
        bc_system_hsi16_enable();

        // Turn PLL on
        RCC->CR |= RCC_CR_PLLON;

        while ((RCC->CR & RCC_CR_PLLRDY) == 0)
        {
            continue;
        }

        _bc_system_switch_clock(BC_SYSTEM_CLOCK_PLL);

        // Set SysTick reload value
        SysTick->LOAD = 32000 - 1;

        // Update SystemCoreClock variable
        SystemCoreClock = 32000000;
    }
}

void bc_system_pll_disable(void)
{
    if (--_bc_system_pll_enable_semaphore == 0)
    {
        _bc_system_switch_clock(BC_SYSTEM_CLOCK_HSI);

        // Turn PLL off
        RCC->CR &= ~RCC_CR_PLLON;

        while ((RCC->CR & RCC_CR_PLLRDY) != 0)
        {
            continue;
        }

        bc_system_hsi16_disable();
    }
}

uint32_t bc_system_get_clock(void)
{
    return SystemCoreClock;
}

void bc_system_reset(void)
{
    NVIC_SystemReset();
}

bool bc_system_get_vbus_sense(void)
{
    static bool init = false;

    if (!init)
    {
        init = true;

        // Enable clock for GPIOA
        RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

        // Errata workaround
        RCC->IOPENR;

        // Enable pull-down
        GPIOA->PUPDR |= GPIO_PUPDR_PUPD12_1;

        // Input mode
        GPIOA->MODER &= ~GPIO_MODER_MODE12_Msk;

        bc_timer_init();
        bc_timer_start();
        bc_timer_delay(10);
        bc_timer_stop();
    }

    return (GPIOA->IDR & GPIO_IDR_ID12) != 0;
}

__attribute__((weak)) void bc_system_error(void)
{
#ifdef RELEASE
    bc_system_reset();
#else
    for (;;);
#endif
}

void HardFault_Handler(void)
{
    bc_system_error();
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


static void _bc_system_switch_clock(bc_system_clock_t clock)
{
    uint32_t clock_mask = bc_system_clock_table[clock];

    bc_irq_disable();

    uint32_t rcc_cfgr = RCC->CFGR;
    rcc_cfgr &= ~RCC_CFGR_SW_Msk;
    rcc_cfgr |= clock_mask;
    RCC->CFGR = rcc_cfgr;

    bc_irq_enable();
}
