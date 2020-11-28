#ifndef _HIO_RAMP_H
#define _HIO_RAMP_H

#include <hio_tick.h>

//! @addtogroup hio_ramp hio_ramp
//! @brief Ramping algorithm library (e.g. can be used for PWM up/down ramping for LED control, motor control, etc.)
//! @{

//! @cond

typedef struct
{
    bool _active;

    hio_tick_t _duration;

    float _start;
    float _stop;
    float _now;

    hio_tick_t _tick_start;
    hio_tick_t _tick_end;

} hio_ramp_t;

//! @endcond

//! @brief Initialize ramp instance
//! @param[in] self Instance
//! @param[in] start Start point
//! @param[in] stop Stop point
//! @param[in] duration Ramp duration in ticks

void hio_ramp_init(hio_ramp_t *self, float start, float stop, hio_tick_t duration);

//! @brief Start ramp sequence
//! @param[in] self Instance

void hio_ramp_start(hio_ramp_t *self);

//! @brief Get current ramp value
//! @param[in] self Instance
//! @return Ramp point

float hio_ramp_get(hio_ramp_t *self);

//! @}

#endif // _HIO_RAMP_H
