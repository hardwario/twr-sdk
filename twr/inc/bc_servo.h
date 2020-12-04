#ifndef _BC_SERVO_H
#define _BC_SERVO_H

#include <bc_pwm.h>

//! @addtogroup bc_servo bc_servo
//! @brief Driver for Servo
//! @{

#define BC_SERVO_NO_ANGLE      255
#define BC_SERVO_UNKNOWN_ANGLE 250
#define BC_SERVO_DEFAULT_MIN   544
#define BC_SERVO_DEFAULT_MAX   2400

//! @brief Servo instance

typedef struct
{
    bc_pwm_channel_t _channel;
    uint16_t _min;
    uint16_t _max;
    uint8_t _angle;

} bc_servo_t;

//! @endcond

//! @brief Initialize Servo
//! @param[in] self Instance
//! @param[in] channel PWM Channel

void bc_servo_init(bc_servo_t *self, bc_pwm_channel_t channel);

//! @brief Set angle
//! @param[in] self Instance
//! @param[in] angle Angle in degrees, 0 to 180

void bc_servo_set_angle(bc_servo_t *self, uint8_t angle);

//! @brief Get angle
//! @param[in] self Instance

uint8_t bc_servo_get_angle(bc_servo_t *self);

//! @brief Set pulse limits
//! @param[in] self Instance
//! @param[in] min_us Min pulse in us (Default 544us)
//! @param[in] max_us Max pulse in us (Default 2400us)

void bc_servo_set_pulse_limits(bc_servo_t *self, uint16_t min_us, uint16_t max_us);

//! @brief Set Microseconds
//! @param[in] self Instance
//! @param[in] us Microseconds

void bc_servo_set_microseconds(bc_servo_t *self, uint16_t us);

//! @}

#endif // _BC_SERVO_H
