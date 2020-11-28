#include <hio_timer.h>
#include <hio_error.h>

typedef struct hio_timer_irq_t
{
    void (*irq_handler)(void *);
    void *irq_param;
} hio_timer_irq_t;

hio_timer_irq_t hio_timer_tim2_irq;
hio_timer_irq_t hio_timer_tim3_irq;
hio_timer_irq_t hio_timer_tim6_irq;

const uint16_t _hio_timer_prescaler_lut[3] =
{
    2,
    15,
    31,
};

static int _hio_timer_lock_count = 0;

inline void hio_timer_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;
}

inline void hio_timer_start(void)
{
    if (_hio_timer_lock_count++ == 0)
    {
        TIM22->PSC = _hio_timer_prescaler_lut[hio_system_clock_get()]; // 7 instructions

        TIM22->CNT = 0;

        TIM22->EGR = TIM_EGR_UG;

        TIM22->CR1 |= TIM_CR1_CEN;
    }
}

inline void hio_timer_stop(void)
{
    if (_hio_timer_lock_count < 1) hio_error(HIO_ERROR_ERROR_UNLOCK);

    if (_hio_timer_lock_count == 1)
    {
        TIM22->CR1 &= ~TIM_CR1_CEN;
    }

    _hio_timer_lock_count--;
}

inline uint16_t hio_timer_get_microseconds(void)
{
    return TIM22->CNT;
}

inline void hio_timer_delay(uint16_t microseconds)
{
    uint16_t t = hio_timer_get_microseconds() + microseconds;

    while (hio_timer_get_microseconds() < t)
    {
        continue;
    }
}

inline void hio_timer_clear(void)
{
    TIM22->CNT = 0;
}

void hio_timer_clear_irq_handler(TIM_TypeDef *tim)
{
    if (tim == TIM3)
    {
        hio_timer_tim3_irq.irq_handler = NULL;
    }
    if (tim == TIM6)
    {
        hio_timer_tim6_irq.irq_handler = NULL;
    }
}

bool hio_timer_set_irq_handler(TIM_TypeDef *tim, void (*irq_handler)(void *), void *irq_param)
{
    if (tim == TIM3)
    {
        if (hio_timer_tim3_irq.irq_handler == NULL)
        {
            hio_timer_tim3_irq.irq_handler = irq_handler;
            hio_timer_tim3_irq.irq_param = irq_param;
            return true;
        }
    }
    if (tim == TIM6)
    {
        if (hio_timer_tim6_irq.irq_handler == NULL)
        {
            hio_timer_tim6_irq.irq_handler = irq_handler;
            hio_timer_tim6_irq.irq_param = irq_param;
            return true;
        }
    }

    if (tim == TIM2)
    {
        if (hio_timer_tim2_irq.irq_handler == NULL)
        {
            hio_timer_tim2_irq.irq_handler = irq_handler;
            hio_timer_tim2_irq.irq_param = irq_param;
            return true;
        }
    }

    return false;
}

void TIM3_IRQHandler(void)
{
    if (hio_timer_tim3_irq.irq_handler)
    {
        hio_timer_tim3_irq.irq_handler(hio_timer_tim3_irq.irq_param);
    }
}

void TIM2_IRQHandler(void)
{
    if (hio_timer_tim2_irq.irq_handler)
    {
        hio_timer_tim2_irq.irq_handler(hio_timer_tim2_irq.irq_param);
    }
}

void TIM6_IRQHandler()
{
    if (hio_timer_tim6_irq.irq_handler)
    {
        hio_timer_tim6_irq.irq_handler(hio_timer_tim6_irq.irq_param);
    }
}
