#ifndef _HIO_TIMER_H
#define _HIO_TIMER_H

#include <hio_system.h>
#include <stm32l0xx.h>

//! @addtogroup hio_timer hio_timer
//! @brief Driver for timer
//! @{

extern const uint16_t _hio_timer_prescaler_lut[3];

//! @brief Initialize timer

void hio_timer_init(void);

//! @brief Start timer

void hio_timer_start(void);

//! @brief Get actual tick of timer
//! @return Actual state of timer counter (microseconds from start)

uint16_t hio_timer_get_microseconds(void);

//! @brief Relative delay
//! @param[in] tick tick to delay in us

void hio_timer_delay(uint16_t microseconds);

//! @brief Clear timer counter

void hio_timer_clear(void);

//! @brief Stop timer

void hio_timer_stop(void);

//! @brief Register timer IRQ handler
//! @param[in] tim Timer, e.g. TIM3
//! @param[in] irq_handler pointer to IRQ handler function
//! @param[in] irq_param parameter

bool hio_timer_set_irq_handler(TIM_TypeDef *tim, void (*irq_handler)(void *), void *irq_param);

//! @brief Unregister timer IRQ handler
//! @param[in] tim Timer, e.g. TIM3

void hio_timer_clear_irq_handler(TIM_TypeDef *tim);

//! @}

#endif // _HIO_TIMER_H
