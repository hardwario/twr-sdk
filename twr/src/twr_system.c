#include <twr_system.h>
#include <twr_scheduler.h>
#include <twr_irq.h>
#include <twr_i2c.h>
#include <twr_timer.h>
#include <stm32l0xx.h>
#include <stm32l0xx_hal_conf.h>
#include <twr_rtc.h>
#include <twr_sleep.h>

#define _TWR_SYSTEM_DEBUG_ENABLE 0

static const uint32_t twr_system_clock_table[3] =
{
    RCC_CFGR_SW_MSI,
    RCC_CFGR_SW_HSI,
    RCC_CFGR_SW_PLL
};

static int _twr_system_hsi16_enable_semaphore;

static int _twr_system_pll_enable_semaphore;

static int _twr_system_deep_sleep_disable_semaphore;

static void _twr_system_init_flash(void);

static void _twr_system_init_debug(void);

static void _twr_system_init_clock(void);

static void _twr_system_init_power(void);

static void _twr_system_init_gpio(void);

static void _twr_system_init_rtc(void);

static void _twr_system_init_shutdown_i2c_sensors(void);

static void _twr_system_switch_clock(twr_system_clock_t clock);

void twr_system_init(void)
{
    _twr_system_init_flash();

    _twr_system_init_debug();

    _twr_system_init_clock();

    _twr_system_init_power();

    _twr_system_init_gpio();

    _twr_system_init_rtc();

    _twr_system_init_shutdown_i2c_sensors();
}

static void _twr_system_init_flash(void)
{
    // Enable prefetch
    FLASH->ACR |= FLASH_ACR_PRFTEN;

    // One wait state is used to read word from NVM
    FLASH->ACR |= FLASH_ACR_LATENCY;
}

static void _twr_system_init_debug(void)
{
#if _TWR_SYSTEM_DEBUG_ENABLE == 1

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

static void _twr_system_init_clock(void)
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

static void _twr_system_init_power(void)
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

static void _twr_system_init_gpio(void)
{
    // Enable clock for GPIOA
    RCC->IOPENR = RCC_IOPENR_GPIOAEN;

    // Errata workaround
    RCC->IOPENR;

    // Set analog mode on PA4
    GPIOA->MODER |= GPIO_MODER_MODE4_1 | GPIO_MODER_MODE4_0;
}

static void _twr_system_init_rtc(void)
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

    twr_rtc_enable_write();

    // Initialize RTC only once
    if ((RTC->ISR & RTC_ISR_INITS) == 0)
    {
        twr_rtc_set_init(true);

        // Set RTC prescaler
        RTC->PRER = TWR_RTC_PREDIV_S - 1;
        RTC->PRER |= (TWR_RTC_PREDIV_A - 1) << 16;

        // Make sure RTC runs in 24-hour mode
        RTC->CR &= ~RTC_CR_FMT;

        twr_rtc_set_init(false);
    }

    // Disable timer
    RTC->CR &= ~RTC_CR_WUTE;

    // Wait until timer configuration update is allowed...
    while ((RTC->ISR & RTC_ISR_WUTWF) == 0)
    {
        continue;
    }

    // Set wake-up auto-reload value based on the configured scheduler interval.
    RTC->WUTR = LSE_VALUE / 16 * TWR_SCHEDULER_INTERVAL_MS / 1000;

    // Clear timer flag
    RTC->ISR &= ~RTC_ISR_WUTF;

    // Enable timer interrupts
    RTC->CR |= RTC_CR_WUTIE;

    // Enable timer
    RTC->CR |= RTC_CR_WUTE;

    twr_rtc_disable_write();

    // RTC IRQ needs to be configured through EXTI
    EXTI->IMR |= EXTI_IMR_IM20;

    // Enable rising edge trigger
    EXTI->RTSR |= EXTI_IMR_IM20;

    // Enable RTC interrupt requests
    NVIC_EnableIRQ(RTC_IRQn);
}

static void _twr_system_init_shutdown_i2c_sensors(void)
{
    twr_i2c_init(TWR_I2C_I2C0, TWR_I2C_SPEED_100_KHZ);

    // tmp112
    twr_i2c_memory_write_16b(TWR_I2C_I2C0, 0x49, 0x01, 0x0180);
    // lis2dh12
    twr_i2c_memory_write_8b(TWR_I2C_I2C0, 0x19, 0x20, 0x07);

    twr_i2c_deinit(TWR_I2C_I2C0);
}

void twr_system_deep_sleep_enable(void)
{
    _twr_system_deep_sleep_disable_semaphore--;

    if (_twr_system_deep_sleep_disable_semaphore == 0)
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }
}

void twr_system_deep_sleep_disable(void)
{
    if (_twr_system_deep_sleep_disable_semaphore == 0)
    {
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }

    _twr_system_deep_sleep_disable_semaphore++;
}

void twr_system_enter_standby_mode(void)
{
    _twr_system_init_shutdown_i2c_sensors();

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

    twr_system_sleep();
}

twr_system_clock_t twr_system_clock_get(void)
{
    if (_twr_system_pll_enable_semaphore != 0)
    {
        return TWR_SYSTEM_CLOCK_PLL;
    }
    else if (_twr_system_hsi16_enable_semaphore != 0)
    {
        return TWR_SYSTEM_CLOCK_HSI;
    }
    else
    {
        return TWR_SYSTEM_CLOCK_MSI;
    }
}

void twr_system_hsi16_enable(void)
{
    if (++_twr_system_hsi16_enable_semaphore == 1)
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

        _twr_system_switch_clock(TWR_SYSTEM_CLOCK_HSI);

        // Set SysTick reload value
        SysTick->LOAD = 16000 - 1;

        // Update SystemCoreClock variable
        SystemCoreClock = 16000000;
    }

    twr_sleep_disable();
}

void twr_system_hsi16_disable(void)
{
    if (--_twr_system_hsi16_enable_semaphore == 0)
    {
        _twr_system_switch_clock(TWR_SYSTEM_CLOCK_MSI);

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

    twr_sleep_enable();
}

void twr_system_pll_enable(void)
{
    if (++_twr_system_pll_enable_semaphore == 1)
    {
        twr_system_hsi16_enable();

        // Turn PLL on
        RCC->CR |= RCC_CR_PLLON;

        while ((RCC->CR & RCC_CR_PLLRDY) == 0)
        {
            continue;
        }

        _twr_system_switch_clock(TWR_SYSTEM_CLOCK_PLL);

        // Set SysTick reload value
        SysTick->LOAD = 32000 - 1;

        // Update SystemCoreClock variable
        SystemCoreClock = 32000000;
    }
}

void twr_system_pll_disable(void)
{
    if (--_twr_system_pll_enable_semaphore == 0)
    {
        _twr_system_switch_clock(TWR_SYSTEM_CLOCK_HSI);

        // Turn PLL off
        RCC->CR &= ~RCC_CR_PLLON;

        while ((RCC->CR & RCC_CR_PLLRDY) != 0)
        {
            continue;
        }

        twr_system_hsi16_disable();
    }
}

uint32_t twr_system_get_clock(void)
{
    return SystemCoreClock;
}

void twr_system_reset(void)
{
    NVIC_SystemReset();
}

bool twr_system_get_vbus_sense(void)
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

        twr_timer_init();
        twr_timer_start();
        twr_timer_delay(10);
        twr_timer_stop();
    }

    return (GPIOA->IDR & GPIO_IDR_ID12) != 0;
}

__attribute__((weak)) void twr_system_error(void)
{
#ifdef RELEASE
    twr_system_reset();
#else
    for (;;);
#endif
}

void HardFault_Handler(void)
{
    twr_system_error();
}

void RTC_IRQHandler(void)
{
    // If wake-up timer flag is set...
    if (RTC->ISR & RTC_ISR_WUTF)
    {
        // Clear wake-up timer flag
        RTC->ISR &= ~RTC_ISR_WUTF;

        twr_tick_increment_irq(TWR_SCHEDULER_INTERVAL_MS);
    }

    // Clear EXTI interrupt flag
    EXTI->PR = EXTI_IMR_IM20;
}


static void _twr_system_switch_clock(twr_system_clock_t clock)
{
    uint32_t clock_mask = twr_system_clock_table[clock];

    twr_irq_disable();

    uint32_t rcc_cfgr = RCC->CFGR;
    rcc_cfgr &= ~RCC_CFGR_SW_Msk;
    rcc_cfgr |= clock_mask;
    RCC->CFGR = rcc_cfgr;

    twr_irq_enable();
}
