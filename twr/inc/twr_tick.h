#ifndef _TWR_TICK_H
#define _TWR_TICK_H

#include <twr_common.h>

//! @addtogroup twr_tick twr_tick
//! @brief Timestamp functions
//! @{

//! @brief Maximum timestamp value

#define TWR_TICK_INFINITY UINT64_C(0xffffffffffffffff)

//! @brief Timestamp data type

typedef uint64_t twr_tick_t;

//! @brief Get absolute timestamp since start of program
//! @return Timestamp in milliseconds

twr_tick_t twr_tick_get(void);

//! @brief Delay execution for specified amount of ticks
//! @param[in] delay Number of ticks to wait

void twr_tick_wait(twr_tick_t delay);

void twr_tick_increment_irq(twr_tick_t delta);

//! @}

#endif // _TWR_TICK_H
