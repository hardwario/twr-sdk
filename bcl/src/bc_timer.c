#include <bc_timer.h>
#include <bc_error.h>

typedef struct bc_timer_irq_t
{
    void (*irq_handler)(void *);
    void *irq_param;
} bc_timer_irq_t;

bc_timer_irq_t bc_timer_tim2_irq;
bc_timer_irq_t bc_timer_tim3_irq;
bc_timer_irq_t bc_timer_tim6_irq;

const uint16_t _bc_timer_prescaler_lut[3] =
{
    2,
    15,
    31,
};

static int _bc_timer_lock_count = 0;

inline void bc_timer_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;
}

inline void bc_timer_start(void)
{
    if (_bc_timer_lock_count++ == 0)
    {
        TIM22->PSC = _bc_timer_prescaler_lut[bc_system_clock_get()]; // 7 instructions

        TIM22->CNT = 0;

        TIM22->EGR = TIM_EGR_UG;

        TIM22->CR1 |= TIM_CR1_CEN;
    }
}

inline void bc_timer_stop(void)
{
    if (_bc_timer_lock_count < 1) bc_error(BC_ERROR_ERROR_UNLOCK);

    if (_bc_timer_lock_count == 1)
    {
        TIM22->CR1 &= ~TIM_CR1_CEN;
    }

    _bc_timer_lock_count--;
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

void bc_timer_clear_irq_handler(TIM_TypeDef *tim)
{
    if (tim == TIM3)
    {
        bc_timer_tim3_irq.irq_handler = NULL;
    }
    if (tim == TIM6)
    {
        bc_timer_tim6_irq.irq_handler = NULL;
    }
}

bool bc_timer_set_irq_handler(TIM_TypeDef *tim, void (*irq_handler)(void *), void *irq_param)
{
    if (tim == TIM3)
    {
        if (bc_timer_tim3_irq.irq_handler == NULL)
        {
            bc_timer_tim3_irq.irq_handler = irq_handler;
            bc_timer_tim3_irq.irq_param = irq_param;
            return true;
        }
    }
    if (tim == TIM6)
    {
        if (bc_timer_tim6_irq.irq_handler == NULL)
        {
            bc_timer_tim6_irq.irq_handler = irq_handler;
            bc_timer_tim6_irq.irq_param = irq_param;
            return true;
        }
    }

    if (tim == TIM2)
    {
        if (bc_timer_tim2_irq.irq_handler == NULL)
        {
            bc_timer_tim2_irq.irq_handler = irq_handler;
            bc_timer_tim2_irq.irq_param = irq_param;
            return true;
        }
    }

    return false;
}

void TIM3_IRQHandler(void)
{
    if (bc_timer_tim3_irq.irq_handler)
    {
        bc_timer_tim3_irq.irq_handler(bc_timer_tim3_irq.irq_param);
    }
}

void TIM2_IRQHandler(void)
{
    if (bc_timer_tim2_irq.irq_handler)
    {
        bc_timer_tim2_irq.irq_handler(bc_timer_tim2_irq.irq_param);
    }
}

void TIM6_IRQHandler()
{
    if (bc_timer_tim6_irq.irq_handler)
    {
        bc_timer_tim6_irq.irq_handler(bc_timer_tim6_irq.irq_param);
    }
}
