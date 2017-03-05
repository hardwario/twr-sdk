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

void bc_tick_inrement_irq(bc_tick_t delta);

//! @}

#endif // _BC_TICK_H
