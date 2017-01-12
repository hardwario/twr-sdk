#ifndef _BC_TICK_H
#define _BC_TICK_H

#include <bc_common.h>

//! @addtogroup bc_tick bc_tick
//! @brief Time measuring function
//! @{

//! @brief Maximum timestamp value
#define BC_TICK_INFINITY 0xffffffff

//! @brief Timestamp data type
typedef int32_t bc_tick_t;

//! @brief Get timestamp since the code started
//! @return time in milliseconds
bc_tick_t bc_tick_get(void);

//! @}

#endif /* _BC_TICK_H */
