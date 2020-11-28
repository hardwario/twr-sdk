#include <hio_servo.h>

#define _HIO_SERVO_PERIOD_CYCLES 50000
#define _HIO_SERVO_RESOLUTION    1

void hio_servo_init(hio_servo_t *self, hio_pwm_channel_t channel)
{
    self->_channel = channel;
    self->_angle = HIO_SERVO_NO_ANGLE;
    self->_min = HIO_SERVO_DEFAULT_MIN;
    self->_max = HIO_SERVO_DEFAULT_MAX;

    hio_pwm_init(channel);

    if ((channel == HIO_PWM_P0) || (channel == HIO_PWM_P1) || (channel == HIO_PWM_P2) || (channel == HIO_PWM_P3))
    {
        hio_pwm_tim_configure(HIO_PWM_TIM2_P0_P1_P2_P3, _HIO_SERVO_RESOLUTION, _HIO_SERVO_PERIOD_CYCLES);
    }
    else if ((channel == HIO_PWM_P6) || (channel == HIO_PWM_P7) || (channel == HIO_PWM_P8))
    {
        hio_pwm_tim_configure(HIO_PWM_TIM3_P6_P7_P8, _HIO_SERVO_RESOLUTION, _HIO_SERVO_PERIOD_CYCLES);
    }
    else if ((channel == HIO_PWM_P12) || (channel == HIO_PWM_P14))
    {
        hio_pwm_tim_configure(HIO_PWM_TIM21_P12_P14, _HIO_SERVO_RESOLUTION, _HIO_SERVO_PERIOD_CYCLES);
    }
}

void hio_servo_set_angle(hio_servo_t *self, uint8_t angle)
{
    if (angle > 180)
    {
        angle = 180;
    }

    uint16_t pwm_value = ((self->_max - self->_min) * ((float) angle / 180.f)) + self->_min;

    hio_pwm_set(self->_channel, pwm_value);

    if (self->_angle == HIO_SERVO_NO_ANGLE)
    {
        hio_pwm_enable(self->_channel);
    }

    self->_angle = angle;
}

uint8_t hio_servo_get_angle(hio_servo_t *self)
{
    return self->_angle;
}

void hio_servo_set_pulse_limits(hio_servo_t *self, uint16_t min_us, uint16_t max_us)
{
    self->_min = min_us;

    self->_max = max_us;
}

void hio_servo_set_microseconds(hio_servo_t *self, uint16_t us)
{
    hio_pwm_set(self->_channel, us);

    if (self->_angle == HIO_SERVO_NO_ANGLE)
    {
        hio_pwm_enable(self->_channel);
    }

    self->_angle = HIO_SERVO_UNKNOWN_ANGLE;
}
