#include <bc_common.h>
#include <stm32l083xx.h>

#define DEBUG_ENABLE    0

void bc_low_power_init_gpio()
{
    __disable_irq();

    NVIC_DisableIRQ(SysTick_IRQn);

    GPIOA->ODR      =   0;
#if DEBUG_ENABLE    ==  1
    GPIOA->PUPDR    =   GPIO_PUPDR_PUPD13_0 | GPIO_PUPDR_PUPD14_1;
    GPIOA->MODER    =   GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 |
                        GPIO_MODER_MODE7 | /*GPIO_MODER_MODE8 |*/ GPIO_MODER_MODE9 | GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12 |
                        GPIO_MODER_MODE13_1 | GPIO_MODER_MODE14_1 | GPIO_MODER_MODE15;

    DBGMCU->CR      =   DBGMCU_CR_DBG_SLEEP | DBGMCU_CR_DBG_STOP | DBGMCU_CR_DBG_STANDBY;
    DBGMCU->APB1FZ  =   DBGMCU_APB1_FZ_DBG_TIM2_STOP | DBGMCU_APB1_FZ_DBG_TIM6_STOP | DBGMCU_APB1_FZ_DBG_RTC_STOP | DBGMCU_APB1_FZ_DBG_WWDG_STOP |
                        DBGMCU_APB1_FZ_DBG_IWDG_STOP | DBGMCU_APB1_FZ_DBG_I2C1_STOP | DBGMCU_APB1_FZ_DBG_I2C2_STOP | DBGMCU_APB1_FZ_DBG_LPTIMER_STOP;
    DBGMCU->APB2FZ  =   DBGMCU_APB2_FZ_DBG_TIM21_STOP | DBGMCU_APB2_FZ_DBG_TIM22_STOP;

#else
    GPIOA->PUPDR    =   0;
    GPIOA->MODER    =   GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 |
                        GPIO_MODER_MODE7 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9 | GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12 |
                        GPIO_MODER_MODE13 | GPIO_MODER_MODE14 | GPIO_MODER_MODE15;

    DBGMCU->CR      =   0;
    DBGMCU->APB1FZ  =   0;
    DBGMCU->APB2FZ  =   0;
#endif

    GPIOB->ODR      =   GPIO_ODR_OD7 | GPIO_ODR_OD8 | GPIO_ODR_OD9; /* I2C and radio pull-up leakage prevention */
    GPIOB->OTYPER   =   GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9;
    GPIOB->MODER    =   GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 |
                        GPIO_MODER_MODE7_0 | GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12 |
                        GPIO_MODER_MODE13 | GPIO_MODER_MODE14 | GPIO_MODER_MODE15;

    GPIOC->ODR      =   0;
    GPIOC->MODER    =   GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2 | GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 |
                        GPIO_MODER_MODE7 | GPIO_MODER_MODE8 | GPIO_MODER_MODE9 | GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12 |
                        GPIO_MODER_MODE13 | GPIO_MODER_MODE14 | GPIO_MODER_MODE15;

    GPIOH->ODR      =   0;
    GPIOH->MODER    =   GPIO_MODER_MODE0 | GPIO_MODER_MODE1_0;

    __enable_irq();
}

void bc_low_power_init()
{
    /* TODO ... enable dynamic configuration of GPIO pins */

    /* All turned off -> 7.5uA */
    /* Scheduler -> + 5uA */
    /* Button check -> + 7uA (due to pull) */

    RCC->APB1ENR    =   RCC_APB1ENR_PWREN |     // Enable power to APB1 (PCLK1)
                        RCC_APB1ENR_LPTIM1EN;   // Enable low-power timer

    RCC->APB2ENR    =   RCC_APB2ENR_SYSCFGEN |  // System configuration controller clock enabled
                        RCC_APB2ENR_DBGMCUEN;   // DBG clock enable

    FLASH->ACR      =   FLASH_ACR_PRE_READ |    // The pre-read is enabled
                        FLASH_ACR_PRFTEN |      // The prefetch is enabled
                        FLASH_ACR_LATENCY;      // One wait state is used to read a word in the NVM

    PWR->CR         =   PWR_CR_DBP |            // Enable debug pins
                        PWR_CR_VOS_1 |          // Internal regulator setup (1.5V)
                        PWR_CR_LPSDSR |         // Enable deep-sleep (else only sleep)
                        PWR_CR_ULP;             // Disable Vrefint in sleep mode

    RCC->CSR        =   RCC_CSR_LSEON |         // Enable external 32.768 kHz oscillator (LSE OSC)
                        RCC_CSR_LSEDRV_1;       // LSE OSC divider

    while(!(RCC->CSR & RCC_CSR_LSERDY))
    {
        /* TODO: Timeout should be implemented here */
    }

    RCC->CSR        =   RCC_CSR_LSEON |         // Turn LSE on
                        RCC_CSR_LSEDRV_1 |      // viz. DocID025274 Rev 4 (page 221)
                        RCC_CSR_RTCSEL_LSE;     // LSE oscillator clock used as RTC(/LCD) clock

    RCC->IOPENR     =   RCC_IOPENR_GPIOAEN |    // Enable GPIOA clocks
                         RCC_IOPENR_GPIOBEN |   // Enable GPIOB clocks
                         RCC_IOPENR_GPIOCEN |   // Enable GPIOC clocks
                         RCC_IOPENR_GPIODEN |   // Enable GPIOD clocks
                         RCC_IOPENR_GPIOHEN;    // Enable GPIOH clocks
}

void bc_low_power_enter()
{
    SCB->SCR |= ((uint32_t)SCB_SCR_SLEEPDEEP_Msk);
    __WFI();
}
