#ifndef _HIO_PWM_H
#define _HIO_PWM_H

#include <hio_system.h>
#include <hio_gpio.h>
#include <stm32l0xx.h>

//! @addtogroup hio_pwm hio_pwm
//! @brief Driver for PWM generation
//! @{

//! @brief PWM channels

typedef enum
{
    HIO_PWM_P0 = HIO_GPIO_P0,
    HIO_PWM_P1 = HIO_GPIO_P1,
    HIO_PWM_P2 = HIO_GPIO_P2,
    HIO_PWM_P3 = HIO_GPIO_P3,

    HIO_PWM_P6 = HIO_GPIO_P6,
    HIO_PWM_P7 = HIO_GPIO_P7,
    HIO_PWM_P8 = HIO_GPIO_P8,

    HIO_PWM_P12 = HIO_GPIO_P12,
    HIO_PWM_P14 = HIO_GPIO_P14,

} hio_pwm_channel_t;

//! @brief PWM timers
//! @details Each timer controls a group of output PWM channels. The change of the specific timer affects also the other related PWM channels.

typedef enum
{
    HIO_PWM_TIM2_P0_P1_P2_P3,
    HIO_PWM_TIM3_P6_P7_P8,
    HIO_PWM_TIM21_P12_P14

}  hio_pwm_tim_t;

//! @brief Initialize PWM timers based on GPIO pin
//! @param[in] channel GPIO channel

void hio_pwm_init(hio_pwm_channel_t channel);

//! @brief Enable PWM output on GPIO pin
//! @param[channel] channel GPIO channel

void hio_pwm_enable(hio_pwm_channel_t channel);

//! @brief Disable PWM output on GPIO pin
//! @param[channel] channel GPIO channel

void hio_pwm_disable(hio_pwm_channel_t channel);

//! @brief Set PWM value for GPIO pin
//! @param[channel] channel GPIO channel
//! @param[pwm_value] PWM duty value 0-255 in default configuration

void hio_pwm_set(hio_pwm_channel_t channel, uint16_t pwm_value);

//! @brief Reconfigure TIM3
//! @param[resolution_us] Set resolution of the timer in microseconds
//! @param[period_cycles] Set number of periods of one cycle

void hio_pwm_tim_configure(hio_pwm_tim_t tim, uint32_t resolution_us, uint32_t period_cycles);

//! @}

#endif // _HIO_PWM_H
