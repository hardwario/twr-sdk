#include <twr_timer.h>
#include <twr_error.h>

typedef struct twr_timer_irq_t
{
    void (*irq_handler)(void *);
    void *irq_param;
} twr_timer_irq_t;

twr_timer_irq_t twr_timer_tim2_irq;
twr_timer_irq_t twr_timer_tim3_irq;
twr_timer_irq_t twr_timer_tim6_irq;

const uint16_t _twr_timer_prescaler_lut[3] =
{
    2,
    15,
    31,
};

static int _twr_timer_lock_count = 0;

inline void twr_timer_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;
}

inline void twr_timer_start(void)
{
    if (_twr_timer_lock_count++ == 0)
    {
        TIM22->PSC = _twr_timer_prescaler_lut[twr_system_clock_get()]; // 7 instructions

        TIM22->CNT = 0;

        TIM22->EGR = TIM_EGR_UG;

        TIM22->CR1 |= TIM_CR1_CEN;
    }
}

inline void twr_timer_stop(void)
{
    if (_twr_timer_lock_count < 1) twr_error(TWR_ERROR_ERROR_UNLOCK);

    if (_twr_timer_lock_count == 1)
    {
        TIM22->CR1 &= ~TIM_CR1_CEN;
    }

    _twr_timer_lock_count--;
}

inline uint16_t twr_timer_get_microseconds(void)
{
    return TIM22->CNT;
}

inline void twr_timer_delay(uint16_t microseconds)
{
    uint16_t t = twr_timer_get_microseconds() + microseconds;

    while (twr_timer_get_microseconds() < t)
    {
        continue;
    }
}

inline void twr_timer_clear(void)
{
    TIM22->CNT = 0;
}

void twr_timer_clear_irq_handler(TIM_TypeDef *tim)
{
    if (tim == TIM3)
    {
        twr_timer_tim3_irq.irq_handler = NULL;
    }
    if (tim == TIM6)
    {
        twr_timer_tim6_irq.irq_handler = NULL;
    }
}

bool twr_timer_set_irq_handler(TIM_TypeDef *tim, void (*irq_handler)(void *), void *irq_param)
{
    if (tim == TIM3)
    {
        if (twr_timer_tim3_irq.irq_handler == NULL)
        {
            twr_timer_tim3_irq.irq_handler = irq_handler;
            twr_timer_tim3_irq.irq_param = irq_param;
            return true;
        }
    }
    if (tim == TIM6)
    {
        if (twr_timer_tim6_irq.irq_handler == NULL)
        {
            twr_timer_tim6_irq.irq_handler = irq_handler;
            twr_timer_tim6_irq.irq_param = irq_param;
            return true;
        }
    }

    if (tim == TIM2)
    {
        if (twr_timer_tim2_irq.irq_handler == NULL)
        {
            twr_timer_tim2_irq.irq_handler = irq_handler;
            twr_timer_tim2_irq.irq_param = irq_param;
            return true;
        }
    }

    return false;
}

void TIM3_IRQHandler(void)
{
    if (twr_timer_tim3_irq.irq_handler)
    {
        twr_timer_tim3_irq.irq_handler(twr_timer_tim3_irq.irq_param);
    }
}

void TIM2_IRQHandler(void)
{
    if (twr_timer_tim2_irq.irq_handler)
    {
        twr_timer_tim2_irq.irq_handler(twr_timer_tim2_irq.irq_param);
    }
}

void TIM6_IRQHandler()
{
    if (twr_timer_tim6_irq.irq_handler)
    {
        twr_timer_tim6_irq.irq_handler(twr_timer_tim6_irq.irq_param);
    }
}
