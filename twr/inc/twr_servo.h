#ifndef _TWR_SERVO_H
#define _TWR_SERVO_H

#include <twr_pwm.h>

//! @addtogroup twr_servo twr_servo
//! @brief Driver for Servo
//! @{

#define TWR_SERVO_NO_ANGLE      255
#define TWR_SERVO_UNKNOWN_ANGLE 250
#define TWR_SERVO_DEFAULT_MIN   544
#define TWR_SERVO_DEFAULT_MAX   2400

//! @brief Servo instance

typedef struct
{
    twr_pwm_channel_t _channel;
    uint16_t _min;
    uint16_t _max;
    uint8_t _angle;

} twr_servo_t;

//! @endcond

//! @brief Initialize Servo
//! @param[in] self Instance
//! @param[in] channel PWM Channel

void twr_servo_init(twr_servo_t *self, twr_pwm_channel_t channel);

//! @brief Set angle
//! @param[in] self Instance
//! @param[in] angle Angle in degrees, 0 to 180

void twr_servo_set_angle(twr_servo_t *self, uint8_t angle);

//! @brief Get angle
//! @param[in] self Instance

uint8_t twr_servo_get_angle(twr_servo_t *self);

//! @brief Set pulse limits
//! @param[in] self Instance
//! @param[in] min_us Min pulse in us (Default 544us)
//! @param[in] max_us Max pulse in us (Default 2400us)

void twr_servo_set_pulse_limits(twr_servo_t *self, uint16_t min_us, uint16_t max_us);

//! @brief Set Microseconds
//! @param[in] self Instance
//! @param[in] us Microseconds

void twr_servo_set_microseconds(twr_servo_t *self, uint16_t us);

//! @}

#endif // _TWR_SERVO_H
