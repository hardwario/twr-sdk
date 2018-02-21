#include <bc_system.h>
#include <bc_scheduler.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

#define _BC_SYSTEM_USE_MARKS 0

#if _BC_SYSTEM_USE_MARKS
#include <bc_gpio.h>
#endif

#define _BC_SYSTEM_DEBUG_ENABLE 0

// #define _BC_SYSTEM_TICK_COUNTER_TO_SYSTEM(__COUNTER_TICK__)  ((__COUNTER_TICK__ * 30518) / 1000000)
#define _BC_SYSTEM_TICK_COUNTER_TO_SYSTEM(__COUNTER_TICK__)  ((__COUNTER_TICK__) / 33)

// #define _BC_SYSTEM_TICK_SYSTEM_TO_COUNTER(__SYSTEM_TICK__)  ((__SYSTEM_TICK__ * 32768) / 1000)
#define _BC_SYSTEM_TICK_SYSTEM_TO_COUNTER(__SYSTEM_TICK__)  ((__SYSTEM_TICK__) * 33)

static const uint32_t bc_system_source_clock_table[3] =
{
    RCC_CFGR_SW_MSI,

    RCC_CFGR_SW_HSI,

    RCC_CFGR_SW_PLL
};

static struct
{
    bc_tick_t tick_counter;

    int hsi16_enable_semaphore;

    int pll_enable_semaphore;

    int deep_sleep_disable_semaphore;

} _bc_system;

static void _bc_system_init_flash(void);

static void _bc_system_init_debug(void);

static void _bc_system_init_clock(void);

static void _bc_system_init_power(void);

static void _bc_system_init_gpio(void);

static void _bc_system_init_lptim(void);

static void _bc_system_switch_clock(bc_system_clock_t clock);

void _bc_scheduler_hook_set_interrupt_tick(bc_tick_t tick);

void bc_system_init(void)
{
#if _BC_SYSTEM_USE_MARKS
    bc_gpio_init(BC_GPIO_P6);
    bc_gpio_set_mode(BC_GPIO_P6, BC_GPIO_MODE_OUTPUT);

    bc_gpio_init(BC_GPIO_P5);
    bc_gpio_set_mode(BC_GPIO_P5, BC_GPIO_MODE_OUTPUT);
#endif

    _bc_system_init_flash();

    _bc_system_init_debug();

    _bc_system_init_clock();

    _bc_system_init_power();

    _bc_system_init_gpio();

    _bc_system_init_lptim();
}

static void _bc_system_init_flash(void)
{
    memset(&_bc_system, 0, sizeof(_bc_system));

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

static void _bc_system_init_lptim(void)
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

    RCC->APB1ENR |= RCC_APB1ENR_LPTIM1EN;

    // Errata workaround
    RCC->APB1ENR;

    RCC->APB1SMENR |= RCC_APB1SMENR_LPTIM1SMEN;

    // Use LSE as a clock source for LPTIM
    RCC->CCIPR |= RCC_CCIPR_LPTIM1SEL;

    EXTI->IMR |= EXTI_IMR_IM29;

    EXTI->RTSR |= EXTI_IMR_IM29;

    LPTIM1->ICR = LPTIM_ICR_ARRMCF | LPTIM_ICR_CMPMCF;

    LPTIM1->IER = LPTIM_IER_ARRMIE;

    LPTIM1->CR = LPTIM_CR_ENABLE;

    LPTIM1->ARR = 0xffff;

    LPTIM1->CR |= LPTIM_CR_CNTSTRT;

    NVIC_EnableIRQ(LPTIM1_IRQn);
}

void bc_system_sleep(void)
{
    __WFI();
}

void bc_system_deep_sleep_enable(void)
{
    _bc_system.deep_sleep_disable_semaphore--;

    if (_bc_system.deep_sleep_disable_semaphore == 0)
    {
        SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    }
}

void bc_system_deep_sleep_disable(void)
{
    if (_bc_system.deep_sleep_disable_semaphore == 0)
    {
        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    }

    _bc_system.deep_sleep_disable_semaphore++;
}

bc_system_clock_t bc_system_clock_source_get(void)
{
    if (_bc_system.pll_enable_semaphore != 0)
    {
        return BC_SYSTEM_CLOCK_PLL;
    }
    else if (_bc_system.hsi16_enable_semaphore != 0)
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
    if (++_bc_system.hsi16_enable_semaphore == 1)
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
    if (--_bc_system.hsi16_enable_semaphore == 0)
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
    if (++_bc_system.pll_enable_semaphore == 1)
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
    if (--_bc_system.pll_enable_semaphore == 0)
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

uint32_t bc_system_clock_get(void)
{
    return SystemCoreClock;
}

bc_tick_t bc_system_tick_get(void)
{
    uint64_t tick_counter;
    bc_tick_t tick_system;

    // Get current counter tick
    tick_counter = _bc_system.tick_counter + LPTIM1->CNT;

    // tick_system = _BC_SYSTEM_TICK_COUNTER_TO_SYSTEM(tick_counter);

    tick_system = tick_counter >> 5;

    return tick_system;
}

void bc_system_reset(void)
{
    NVIC_SystemReset();
}

__attribute__((weak)) void bc_system_error(void)
{
#ifdef RELEASE
    bc_system_reset();
#else
    for (;;)
        ;
#endif
}

void HardFault_Handler(void)
{
    bc_system_error();
}

void LPTIM1_IRQHandler(void)
{
    if ((LPTIM1->ISR & LPTIM_ISR_ARRM) != 0)
    {
        // Clear interrupt flag
        LPTIM1->ICR = LPTIM_ICR_ARRMCF;

        // Counter overflow period is 2 seconds
        _bc_system.tick_counter += 65536;
    }
    // This interrupt is used as a wakeup for scheduler
    else if ((LPTIM1->ISR & LPTIM_ISR_CMPM) != 0)
    {
#if _BC_SYSTEM_USE_MARKS
        bc_gpio_set_output(BC_GPIO_P6, 1);
#endif
        // Disable compare IRQ
        LPTIM1->IER &= ~LPTIM_IER_CMPMIE;

        // Clear interrupt flag
        LPTIM1->ICR = LPTIM_ICR_CMPMCF;

#if _BC_SYSTEM_USE_MARKS
        bc_gpio_set_output(BC_GPIO_P6, 0);
#endif
    }

    // Clear EXTI interrupt flag
    EXTI->PR = EXTI_IMR_IM29;
}

static void _bc_system_switch_clock(bc_system_clock_t clock)
{
    uint32_t clock_mask = bc_system_source_clock_table[clock];

    bc_irq_disable();

    uint32_t rcc_cfgr = RCC->CFGR;
    rcc_cfgr &= ~RCC_CFGR_SW_Msk;
    rcc_cfgr |= clock_mask;
    RCC->CFGR = rcc_cfgr;

    bc_irq_enable();
}

void _bc_scheduler_hook_set_interrupt_tick(bc_tick_t tick)
{
    // uint64_t tick_counter = _BC_SYSTEM_TICK_SYSTEM_TO_COUNTER(tick);
    static uint64_t tick_counter;

    static bc_tick_t tick_system_now;
    // uint64_t tick_counter_now = _BC_SYSTEM_TICK_SYSTEM_TO_COUNTER(tick_system_now);
    static uint64_t tick_counter_now;

    // TODO counter overflow

    tick_counter = tick << 5;
    tick_system_now = bc_system_tick_get();
    tick_counter_now = tick_system_now << 5;

    if ((tick_counter_now & 0xffffffffffff0000) == (tick_counter & 0xffffffffffff0000))
    {
#if _BC_SYSTEM_USE_MARKS
        bc_gpio_set_output(BC_GPIO_P5, 1);
#endif
        LPTIM1->CMP = tick_counter & 0x000000000000ffff;

        LPTIM1->IER |= LPTIM_IER_CMPMIE;
#if _BC_SYSTEM_USE_MARKS
        bc_gpio_set_output(BC_GPIO_P5, 0);
#endif
    }
    else
    {
        LPTIM1->IER &= ~LPTIM_IER_CMPMIE;
    }
}
