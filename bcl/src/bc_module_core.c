#include <bc_common.h>
#include <bc_rtc.h>
#include <stm32l0xx_hal.h>

#define DEBUG_ENABLE 1

void SystemClock_Config(void);
void Error_Handler(void);

void bc_module_core_init()
{
    HAL_Init();

    FLASH->ACR |= FLASH_ACR_PRE_READ;   // Enable pre-read

    RCC->APB1ENR |= RCC_APB1ENR_PWREN;  // Enable APB1 clock

#if DEBUG_ENABLE
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN | // Enable system configuration controller block
    RCC_APB2ENR_DBGMCUEN;   // Enable MCU debug module clock

    DBGMCU->CR = DBGMCU_CR_DBG_SLEEP |  // Debug Sleep mode (FCLK=On, HCLK=On)
    DBGMCU_CR_DBG_STOP |   // Debug Stop mode (FCLK=On, HCLK=On)
    DBGMCU_CR_DBG_STANDBY; // Debug Standby mode (FCLK=On, HCLK=On)

    DBGMCU->APB1FZ = DBGMCU_APB1_FZ_DBG_TIM2_STOP |
    DBGMCU_APB1_FZ_DBG_TIM6_STOP |
    DBGMCU_APB1_FZ_DBG_RTC_STOP |
    DBGMCU_APB1_FZ_DBG_WWDG_STOP |
    DBGMCU_APB1_FZ_DBG_IWDG_STOP |
    DBGMCU_APB1_FZ_DBG_I2C1_STOP |
    DBGMCU_APB1_FZ_DBG_I2C2_STOP |
    DBGMCU_APB1_FZ_DBG_LPTIMER_STOP;

    DBGMCU->APB2FZ = DBGMCU_APB2_FZ_DBG_TIM21_STOP |
    DBGMCU_APB2_FZ_DBG_TIM22_STOP;
#else
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Enable system configuration controller block

    DBGMCU->APB1FZ = 0;
    DBGMCU->APB2FZ = 0;
#endif

    HAL_NVIC_SetPriority(SVC_IRQn, 0, 0);
    HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    SystemClock_Config();

    RCC->CFGR |= RCC_CFGR_STOPWUCK; // Device is waked up with HSI16 (else MSI ... 64 KHz to 4 MHz)

    PWR->CR = PWR_CR_DBP |    // Disable backup domain
    PWR_CR_VOS_0 |  // Internal regulator setup (1.8V)
    PWR_CR_LPSDSR | // Enable deep-sleep (else only sleep)
    PWR_CR_ULP;     // Disable Vrefint in sleep mode

    RCC->IOPENR = RCC_IOPENR_GPIOAEN |    // Enable GPIOA clocks
    RCC_IOPENR_GPIOBEN |    // Enable GPIOB clocks
    RCC_IOPENR_GPIOCEN |    // Enable GPIOC clocks
    RCC_IOPENR_GPIOHEN;     // Enable GPIOH clocks

    GPIOA->MODER |= GPIO_MODER_MODE4_1 | GPIO_MODER_MODE4_0;

#if !DEBUG_ENABLE
    GPIOA->MODER = 0xffffffff;
#endif

    bc_rtc_init();  // Initialize RTC to get "ClownTick" interrupt for scheduler
}

void bc_module_core_sleep()
{
    SCB->SCR |= ((uint32_t) SCB_SCR_SLEEPDEEP_Msk);
    __WFI();
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /**Initializes the CPU, AHB and APB busses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_LPUART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    /**Configure the Systick interrupt time
     */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /**Configure the Systick
     */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void Error_Handler(void)
{
    // TODO Replace
    for (;;)
        ;
}
