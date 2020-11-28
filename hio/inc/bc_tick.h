#ifndef _HIO_TICK_H
#define _HIO_TICK_H

#include <hio_common.h>

//! @addtogroup hio_tick hio_tick
//! @brief Timestamp functions
//! @{

//! @brief Maximum timestamp value

#define HIO_TICK_INFINITY UINT64_C(0xffffffffffffffff)

//! @brief Timestamp data type

typedef uint64_t hio_tick_t;

//! @brief Get absolute timestamp since start of program
//! @return Timestamp in milliseconds

hio_tick_t hio_tick_get(void);

//! @brief Delay execution for specified amount of ticks
//! @param[in] delay Number of ticks to wait

void hio_tick_wait(hio_tick_t delay);

void hio_tick_increment_irq(hio_tick_t delta);

//! @}

#endif // _HIO_TICK_H
