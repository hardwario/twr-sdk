#ifndef _BC_TIMER_H
#define _BC_TIMER_H

#include <bc_system.h>

//! @addtogroup bc_timer bc_timer
//! @brief Driver for timer
//! @{

extern const uint16_t _bc_timer_prescaler_lut[3];

//! @brief Initialize timer

void bc_timer_init(void);

//! @brief Start timer

void bc_timer_start(void);

//! @brief Get actual tick of timer
//! @return Actual state of timer counter (microseconds from start)

uint16_t bc_timer_get_microseconds(void);

//! @brief Relative delay
//! @param[in] tick tick to delay in us

void bc_timer_delay(uint16_t microseconds);

//! @brief Clear timer counter

void bc_timer_clear(void);

//! @brief Stop timer

void bc_timer_stop(void);

//! @}

#endif // _BC_TIMER_H
