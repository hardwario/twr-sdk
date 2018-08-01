#include <bc_timer.h>
#include <stm32l0xx.h>

const uint16_t _bc_timer_prescaler_lut[3] =
{
    2,
    15,
    31,
};

inline void bc_timer_init(void)
{
    // Enable clock for TIM22
    RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;
}

inline void bc_timer_start(void)
{
    TIM22->PSC = _bc_timer_prescaler_lut[bc_system_clock_get()]; // 7 instructions

    TIM22->CNT = 0;

    TIM22->EGR = TIM_EGR_UG;

    TIM22->CR1 |= TIM_CR1_CEN;
}

inline uint16_t bc_timer_get_microseconds(void)
{
    return TIM22->CNT;
}

inline void bc_timer_delay(uint16_t microseconds)
{
    uint16_t t = bc_timer_get_microseconds() + microseconds;

    while (bc_timer_get_microseconds() < t)
    {
        continue;
    }
}

inline void bc_timer_clear(void)
{
    TIM22->CNT = 0;
}

inline void bc_timer_stop(void)
{
    TIM22->CR1 &= ~TIM_CR1_CEN;
}
