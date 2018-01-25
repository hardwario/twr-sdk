#ifndef _BC_TIMER_H
#define _BC_TIMER_H

#include <bc_common.h>
#include <bc_system.h>
#include <stm32l0xx.h>

//! @addtogroup bc_timer bc_timer
//! @brief Driver for timer
//! @{

extern const uint16_t _bc_timer_prescaler_lut[3];

//! @brief Initialize timer

static inline void bc_timer_init(void)
{
    // Enable clock for TIM21
    RCC->APB1ENR |= RCC_APB2ENR_TIM21EN;
}

//! @brief Start timer

static inline void bc_timer_start(void)
{
    TIM21->PSC = _bc_timer_prescaler_lut[bc_system_clock_get()];

    TIM21->CNT = 0;

    TIM21->EGR = TIM_EGR_UG;

    TIM21->CR1 |= TIM_CR1_CEN;
}

//! @brief Get actual tick of timer
//! @return Actual state of timer counter (microseconds from start)

static inline uint16_t bc_timer_get_microseconds(void)
{
    return TIM21->CNT;
}

//! @brief Relative delay
//! @param[in] tick tick to delay in us


static inline void bc_timer_delay(uint16_t microseconds)
{
    uint16_t t = bc_timer_get_microseconds() + microseconds;

    while (bc_timer_get_microseconds() < t)
    {
        continue;
    }
}

//! @brief Clear timer counter

static inline void bc_timer_clear(void)
{
    TIM21->CNT = 0;
}

//! @brief Stop timer

static inline void bc_timer_stop(void)
{
    TIM21->CR1 &= ~TIM_CR1_CEN;

    bc_system_hsi16_disable();
}

#endif // _BC_TIMER_H
