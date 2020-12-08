#include <twr_servo.h>

#define _TWR_SERVO_PERIOD_CYCLES 50000
#define _TWR_SERVO_RESOLUTION    1

void twr_servo_init(twr_servo_t *self, twr_pwm_channel_t channel)
{
    self->_channel = channel;
    self->_angle = TWR_SERVO_NO_ANGLE;
    self->_min = TWR_SERVO_DEFAULT_MIN;
    self->_max = TWR_SERVO_DEFAULT_MAX;

    twr_pwm_init(channel);

    if ((channel == TWR_PWM_P0) || (channel == TWR_PWM_P1) || (channel == TWR_PWM_P2) || (channel == TWR_PWM_P3))
    {
        twr_pwm_tim_configure(TWR_PWM_TIM2_P0_P1_P2_P3, _TWR_SERVO_RESOLUTION, _TWR_SERVO_PERIOD_CYCLES);
    }
    else if ((channel == TWR_PWM_P6) || (channel == TWR_PWM_P7) || (channel == TWR_PWM_P8))
    {
        twr_pwm_tim_configure(TWR_PWM_TIM3_P6_P7_P8, _TWR_SERVO_RESOLUTION, _TWR_SERVO_PERIOD_CYCLES);
    }
    else if ((channel == TWR_PWM_P12) || (channel == TWR_PWM_P14))
    {
        twr_pwm_tim_configure(TWR_PWM_TIM21_P12_P14, _TWR_SERVO_RESOLUTION, _TWR_SERVO_PERIOD_CYCLES);
    }
}

void twr_servo_set_angle(twr_servo_t *self, uint8_t angle)
{
    if (angle > 180)
    {
        angle = 180;
    }

    uint16_t pwm_value = ((self->_max - self->_min) * ((float) angle / 180.f)) + self->_min;

    twr_pwm_set(self->_channel, pwm_value);

    if (self->_angle == TWR_SERVO_NO_ANGLE)
    {
        twr_pwm_enable(self->_channel);
    }

    self->_angle = angle;
}

uint8_t twr_servo_get_angle(twr_servo_t *self)
{
    return self->_angle;
}

void twr_servo_set_pulse_limits(twr_servo_t *self, uint16_t min_us, uint16_t max_us)
{
    self->_min = min_us;

    self->_max = max_us;
}

void twr_servo_set_microseconds(twr_servo_t *self, uint16_t us)
{
    twr_pwm_set(self->_channel, us);

    if (self->_angle == TWR_SERVO_NO_ANGLE)
    {
        twr_pwm_enable(self->_channel);
    }

    self->_angle = TWR_SERVO_UNKNOWN_ANGLE;
}
