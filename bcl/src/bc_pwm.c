#include <bc_pwm.h>

/*
    Table of pins and TIM channels

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

static void _bc_pwm_tim2_configure(uint32_t resolution_us, uint32_t period_cycles)
{
    // Enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Errata workaround
    RCC->APB1ENR;

    // Disable counter if it is running
    TIM2->CR1 &= ~TIM_CR1_CEN;

    // Set prescaler to 5 * 32 (5 microseconds resolution)
    //TIM2->PSC = 5 * 32 - 1;
    TIM2->PSC = resolution_us * 32 - 1;
    TIM2->ARR = period_cycles - 1;

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
}

static void _bc_pwm_tim3_configure(uint32_t resolution_us, uint32_t period_cycles)
{
    // Enable TIM3 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Errata workaround
    RCC->APB1ENR;

    // Disable counter if it is running
    TIM3->CR1 &= ~TIM_CR1_CEN;

    // Set prescaler to 5 * 32 (5 microseconds resolution)
    TIM3->PSC = resolution_us * 32 - 1;
    TIM3->ARR = period_cycles - 1;

    // CH1
    TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
    TIM3->CCER |= TIM_CCER_CC1E;
    // CH2 - is on PB5 - RADIO_MOSI
    //TIM3->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
    //TIM3->CCER |= TIM_CCER_CC2E;
    // CH3
    TIM3->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;
    TIM3->CCER |= TIM_CCER_CC3E;
    // CH4
    TIM3->CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2;
    TIM3->CCER |= TIM_CCER_CC4E;

    TIM3->CR1 |= TIM_CR1_CEN;
}

static void _bc_pwm_tim21_configure(uint32_t resolution_us, uint32_t period_cycles)
{
    // Enable TIM21 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;

    // Errata workaround
    RCC->APB2ENR;

    // Disable counter if it is running
    TIM21->CR1 &= ~TIM_CR1_CEN;

    // Set prescaler to 5 * 32 (5 microseconds resolution)
    TIM21->PSC = resolution_us * 32 - 1;
    TIM21->ARR = period_cycles - 1;

    // CH1
    TIM21->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;
    TIM21->CCER |= TIM_CCER_CC1E;
    // CH2
    TIM21->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2;
    TIM21->CCER |= TIM_CCER_CC2E;

    TIM21->CR1 |= TIM_CR1_CEN;
}

void bc_pwm_tim_configure(bc_pwm_tim_t tim, uint32_t resolution_us, uint32_t period_cycles)
{
    switch (tim)
    {
        case BC_PWM_TIM2_P0_P1_P2_P3:
        {
            _bc_pwm_tim2_configure(resolution_us, period_cycles);
            break;
        }
        case BC_PWM_TIM3_P6_P7_P8:
        {
            _bc_pwm_tim3_configure(resolution_us, period_cycles);
            break;
        }
        case BC_PWM_TIM21_P12_P14:
        {
            _bc_pwm_tim21_configure(resolution_us, period_cycles);
            break;
        }
        default:
        {
            break;
        }
    }
}

void bc_pwm_init(bc_pwm_channel_t channel)
{
    static bool tim2_initialized = false;
    static bool tim3_initialized = false;
    static bool tim21_initialized = false;
    static bool pll_enabled = false;

    if (!pll_enabled)
    {
        bc_system_pll_enable();
        pll_enabled = true;
    }

    if (!tim2_initialized && (channel == BC_PWM_P0 || channel == BC_PWM_P1 || channel == BC_PWM_P2 || channel == BC_PWM_P3))
    {
        // 5 us * 255 = cca 784 Hz
        bc_pwm_tim_configure(BC_PWM_TIM2_P0_P1_P2_P3, 5, 255);
        tim2_initialized = true;
    }

    if (!tim3_initialized && (channel == BC_PWM_P6 || channel == BC_PWM_P7 || channel == BC_PWM_P8))
    {
        bc_pwm_tim_configure(BC_PWM_TIM3_P6_P7_P8, 5, 255);
        tim3_initialized = true;
    }

    if (!tim21_initialized && (channel == BC_PWM_P12 || channel == BC_PWM_P14))
    {
        bc_pwm_tim_configure(BC_PWM_TIM21_P12_P14, 5, 255);
        tim21_initialized = true;
    }
}

void bc_pwm_enable(bc_pwm_channel_t channel)
{
    bc_gpio_init(channel);

    if (channel == BC_PWM_P12 || channel == BC_PWM_P14)
    {
        bc_gpio_set_mode(channel, BC_GPIO_MODE_ALTERNATE_6);
    }
    else
    {
        bc_gpio_set_mode(channel, BC_GPIO_MODE_ALTERNATE_2);
    }
}

void bc_pwm_disable(bc_pwm_channel_t channel)
{
    bc_gpio_set_mode(channel, BC_GPIO_MODE_ANALOG);
}

void bc_pwm_set(bc_pwm_channel_t channel, uint16_t pwm_value)
{
    switch (channel)
    {
        case BC_PWM_P0:
        {
            TIM2->CCR1 = pwm_value;
            break;
        }
        case BC_PWM_P1:
        {
            TIM2->CCR2 = pwm_value;
            break;
        }
        case BC_PWM_P2:
        {
            TIM2->CCR3 = pwm_value;
            break;
        }
        case BC_PWM_P3:
        {
            TIM2->CCR4 = pwm_value;
            break;
        }
        case BC_PWM_P6:
        {
            TIM3->CCR4 = pwm_value;
            break;
        }
        case BC_PWM_P7:
        {
            TIM3->CCR1 = pwm_value;
            break;
        }
        case BC_PWM_P8:
        {
            TIM3->CCR3 = pwm_value;
            break;
        }
        case BC_PWM_P12:
        {
            TIM21->CCR2 = pwm_value;
            break;
        }
        case BC_PWM_P14:
        {
            TIM21->CCR1 = pwm_value;
            break;
        }
        default:
        {
            break;
        }
    }
}
