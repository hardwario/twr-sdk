#ifndef _BC_TICK_H
#define _BC_TICK_H

#include <bc_common.h>

//! @addtogroup bc_tick bc_tick
//! @brief Timestamp functions
//! @{

//! @brief Maximum timestamp value

#define BC_TICK_INFINITY UINT64_C(0xffffffffffffffff)

//! @brief Timestamp data type

typedef uint64_t bc_tick_t;

//! @brief Get absolute timestamp since start of program
//! @return Timestamp in milliseconds

bc_tick_t bc_tick_get(void);

//! @brief Delay execution for specified amount of ticks
//! @param[in] delay Number of ticks to wait

void bc_tick_wait(bc_tick_t delay);

void bc_tick_increment_irq(bc_tick_t delta);

//! @}

#endif // _BC_TICK_H
