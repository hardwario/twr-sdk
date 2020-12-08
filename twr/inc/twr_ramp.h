#ifndef _TWR_RAMP_H
#define _TWR_RAMP_H

#include <twr_tick.h>

//! @addtogroup twr_ramp twr_ramp
//! @brief Ramping algorithm library (e.g. can be used for PWM up/down ramping for LED control, motor control, etc.)
//! @{

//! @cond

typedef struct
{
    bool _active;

    twr_tick_t _duration;

    float _start;
    float _stop;
    float _now;

    twr_tick_t _tick_start;
    twr_tick_t _tick_end;

} twr_ramp_t;

//! @endcond

//! @brief Initialize ramp instance
//! @param[in] self Instance
//! @param[in] start Start point
//! @param[in] stop Stop point
//! @param[in] duration Ramp duration in ticks

void twr_ramp_init(twr_ramp_t *self, float start, float stop, twr_tick_t duration);

//! @brief Start ramp sequence
//! @param[in] self Instance

void twr_ramp_start(twr_ramp_t *self);

//! @brief Get current ramp value
//! @param[in] self Instance
//! @return Ramp point

float twr_ramp_get(twr_ramp_t *self);

//! @}

#endif // _TWR_RAMP_H
