#include <bc_common.h>
#include <bc_rtc.h>
#include <stm32l0xx_hal.h>
#include <stm32l083xx.h>

#define DEBUG_ENABLE    0

void SystemClock_Config(void);
void Error_Handler(void);

void bc_module_core_init()
{
    HAL_Init();

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriority(SVC_IRQn, 0, 0);
    HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    SystemClock_Config();

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

    GPIOB->ODR      =   GPIO_ODR_OD7 | GPIO_ODR_OD8 | GPIO_ODR_OD9;
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

    bc_rtc_init();
}

void bc_module_core_sleep()
{
    SCB->SCR |= ((uint32_t)SCB_SCR_SLEEPDEEP_Msk);
    __WFI();
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_USB;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void Error_Handler(void)
{
    // TODO Replace
    for (;;);
}
