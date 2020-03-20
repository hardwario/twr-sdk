#ifndef _BC_RAMP_H
#define _BC_RAMP_H

#include <bc_tick.h>

//! @addtogroup bc_ramp bc_ramp
//! @brief Ramping algorithm library (e.g. can be used for PWM up/down ramping for LED control, motor control, etc.)
//! @{

//! @cond

typedef struct
{
    bool _active;

    bc_tick_t _duration;

    float _start;
    float _stop;
    float _now;

    bc_tick_t _tick_start;
    bc_tick_t _tick_end;

} bc_ramp_t;

//! @endcond

//! @brief Initialize ramp instance
//! @param[in] self Instance
//! @param[in] start Start point
//! @param[in] stop Stop point
//! @param[in] duration Ramp duration in ticks

void bc_ramp_init(bc_ramp_t *self, float start, float stop, bc_tick_t duration);

//! @brief Start ramp sequence
//! @param[in] self Instance

void bc_ramp_start(bc_ramp_t *self);

//! @brief Get current ramp value
//! @param[in] self Instance
//! @return Ramp point

float bc_ramp_get(bc_ramp_t *self);

//! @}

#endif // _BC_RAMP_H
