#include <bc_servo.h>

#define _BC_SERVO_PERIOD_CYCLES 50000
#define _BC_SERVO_RESOLUTION    1

void bc_servo_init(bc_servo_t *self, bc_pwm_channel_t channel)
{
    self->_channel = channel;
    self->_angle = BC_SERVO_NO_ANGLE;
    self->_min = BC_SERVO_DEFAULT_MIN;
    self->_max = BC_SERVO_DEFAULT_MAX;

    bc_pwm_init(channel);

    if ((channel == BC_PWM_P0) || (channel == BC_PWM_P1) || (channel == BC_PWM_P2) || (channel == BC_PWM_P3))
    {
        bc_pwm_tim_configure(BC_PWM_TIM2_P0_P1_P2_P3, _BC_SERVO_RESOLUTION, _BC_SERVO_PERIOD_CYCLES);
    }
    else if ((channel == BC_PWM_P6) || (channel == BC_PWM_P7) || (channel == BC_PWM_P8))
    {
        bc_pwm_tim_configure(BC_PWM_TIM3_P6_P7_P8, _BC_SERVO_RESOLUTION, _BC_SERVO_PERIOD_CYCLES);
    }
    else if ((channel == BC_PWM_P12) || (channel == BC_PWM_P14))
    {
        bc_pwm_tim_configure(BC_PWM_TIM21_P12_P14, _BC_SERVO_RESOLUTION, _BC_SERVO_PERIOD_CYCLES);
    }
}

void bc_servo_set_angle(bc_servo_t *self, uint8_t angle)
{
    if (angle > 180)
    {
        angle = 180;
    }

    uint16_t pwm_value = ((self->_max - self->_min) * ((float) angle / 180.f)) + self->_min;

    bc_pwm_set(self->_channel, pwm_value);

    if (self->_angle == BC_SERVO_NO_ANGLE)
    {
        bc_pwm_enable(self->_channel);
    }

    self->_angle = angle;
}

uint8_t bc_servo_get_angle(bc_servo_t *self)
{
    return self->_angle;
}

void bc_servo_set_pulse_limits(bc_servo_t *self, uint16_t min_us, uint16_t max_us)
{
    self->_min = min_us;

    self->_max = max_us;
}

void bc_servo_set_microseconds(bc_servo_t *self, uint16_t us)
{
    bc_pwm_set(self->_channel, us);

    if (self->_angle == BC_SERVO_NO_ANGLE)
    {
        bc_pwm_enable(self->_channel);
    }

    self->_angle = BC_SERVO_UNKNOWN_ANGLE;
}
