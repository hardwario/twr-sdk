#include <bc_pwm.h>

/*
P0 PA0 TIM2_CH1
P1 PA1 TIM2_CH2
P2 PA2 TIM2_CH3 TIM21_CH1
P3 PA3 TIM2_CH4 TIM21_CH2
P4 PA4 none
P5 PA5 TIM2_CH1
P6 PB1 TIM3_CH4
P7 PA6 TIM3_CH1 TIM22_CH2
P8 PB0 TIM3_CH3
P9 PB2 LPTIM1_OUT

P10 PA10 none
P11 PA9 none
P12 PB14 TIM21_CH2
P13 PB15 none
P14 PB13 TIM21_CH1
P15 PB12 none
P16 PB8 none
P17 PB9 none
*/

void bc_pwm_tim2_init(void)
{
    static bool tim2_initialized = false;

    if (tim2_initialized)
    {
        return;
    }

    bc_system_pll_enable();

    // Enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Errata workaround
    RCC->APB1ENR;

    // Set prescaler to 5 * 32 (5 microseconds resolution)
    TIM2->PSC = 5 * 32 - 1;
    TIM2->ARR= 255 - 1;

    // CH1
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
    TIM2->CCER |= TIM_CCER_CC1E;
    // CH2
    TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
    TIM2->CCER |= TIM_CCER_CC2E;
    // CH3
    TIM2->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
    TIM2->CCER |= TIM_CCER_CC3E;
    // CH4
    TIM2->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
    TIM2->CCER |= TIM_CCER_CC4E;

    TIM2->CR1 |= TIM_CR1_CEN;

    tim2_initialized = true;
}


void bc_pwm_tim3_init(void)
{
    static bool tim3_initialized = false;

    if (tim3_initialized)
    {
        return;
    }

    bc_system_pll_enable();

    // Enable TIM3 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Errata workaround
    RCC->APB1ENR;

    // Set prescaler to 5 * 32 (5 microseconds resolution)
    TIM3->PSC = 5 * 32 - 1;
    TIM3->ARR= 255 - 1;

    // CH1
    TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
    TIM3->CCER |= TIM_CCER_CC1E;
    // CH2
    //TIM3->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
    //TIM3->CCER |= TIM_CCER_CC2E;
    // CH3
    TIM3->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
    TIM3->CCER |= TIM_CCER_CC3E;
    // CH4
    TIM3->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
    TIM3->CCER |= TIM_CCER_CC4E;

    TIM3->CR1 |= TIM_CR1_CEN;

    tim3_initialized = true;
}


void bc_pwm_init(bc_gpio_channel_t channel)
{
    if (channel == BC_GPIO_P0 || channel == BC_GPIO_P1 || channel == BC_GPIO_P2 || channel == BC_GPIO_P3)
    {
        bc_pwm_tim2_init();
    }

    if (channel == BC_GPIO_P6 || channel == BC_GPIO_P7 || channel == BC_GPIO_P8)
    {
        bc_pwm_tim3_init();
    }
}

void bc_pwm_enable(bc_gpio_channel_t channel)
{
    bc_gpio_init(channel);
    bc_gpio_set_mode(channel, BC_GPIO_MODE_ALTERNATE_2);
}

void bc_pwm_set(bc_gpio_channel_t channel, uint8_t pwm_value)
{
    switch (channel)
    {
        case BC_GPIO_P0:
        TIM2->CCR1 = pwm_value;
        break;

        case BC_GPIO_P1:
        TIM2->CCR2 = pwm_value;
        break;

        case BC_GPIO_P2:
        TIM2->CCR3 = pwm_value;
        break;

        case BC_GPIO_P3:
        TIM2->CCR4 = pwm_value;
        break;

    case BC_GPIO_P4:
    case BC_GPIO_P5:
        break;

        case BC_GPIO_P6:
        TIM3->CCR4 = pwm_value;
        break;

        case BC_GPIO_P7:
        TIM3->CCR1 = pwm_value;
        break;

        case BC_GPIO_P8:
        TIM3->CCR3 = pwm_value;
        break;

        case BC_GPIO_P9:
        case BC_GPIO_P10:
        case BC_GPIO_P11:
        case BC_GPIO_P12:
        case BC_GPIO_P13:
        case BC_GPIO_P14:
        case BC_GPIO_P15:
        case BC_GPIO_P16:
        case BC_GPIO_P17:
        case BC_GPIO_LED:
        case BC_GPIO_BUTTON:
        default:
        break;
    }
}
