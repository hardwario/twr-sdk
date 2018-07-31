#ifndef _BC_TIMER_H
#define _BC_TIMER_H

#include <bc_system.h>
#include <stm32l0xx.h>

//! @addtogroup bc_timer bc_timer
//! @brief Driver for timer
//! @{

extern const uint16_t _bc_timer_prescaler_lut[3];

//! @brief Initialize timer

inline void bc_timer_init(void)
{
    // Enable clock for TIM22
    RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;
}

//! @brief Start timer

inline void bc_timer_start(void)
{
    TIM22->PSC = _bc_timer_prescaler_lut[bc_system_clock_get()]; // 7 instructions

    TIM22->CNT = 0;

    TIM22->EGR = TIM_EGR_UG;

    TIM22->CR1 |= TIM_CR1_CEN;
}

//! @brief Get actual tick of timer
//! @return Actual state of timer counter (microseconds from start)

inline uint16_t bc_timer_get_microseconds(void)
{
    return TIM22->CNT;
}

//! @brief Relative delay
//! @param[in] tick tick to delay in us

inline void bc_timer_delay(uint16_t microseconds)
{
    uint16_t t = bc_timer_get_microseconds() + microseconds;

    while (bc_timer_get_microseconds() < t)
    {
        continue;
    }
}

//! @brief Clear timer counter

inline void bc_timer_clear(void)
{
    TIM22->CNT = 0;
}

//! @brief Stop timer

inline void bc_timer_stop(void)
{
    TIM22->CR1 &= ~TIM_CR1_CEN;
}

//! @}

#endif // _BC_TIMER_H
