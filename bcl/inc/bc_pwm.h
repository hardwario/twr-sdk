#ifndef _BC_PWM_H
#define _BC_PWM_H

#include <bc_system.h>
#include <bc_gpio.h>
#include <stm32l0xx.h>

//! @addtogroup bc_pwm bc_pwm
//! @brief Driver for PWM generation
//! @{

//! @brief PWM channels

typedef enum
{
    BC_PWM_P0 = BC_GPIO_P0,
    BC_PWM_P1 = BC_GPIO_P1,
    BC_PWM_P2 = BC_GPIO_P2,
    BC_PWM_P3 = BC_GPIO_P3,

    BC_PWM_P6 = BC_GPIO_P6,
    BC_PWM_P7 = BC_GPIO_P7,
    BC_PWM_P8 = BC_GPIO_P8,

    BC_PWM_P12 = BC_GPIO_P12,
    BC_PWM_P14 = BC_GPIO_P14,

} bc_pwm_channel_t;

//! @brief PWM timers
//! @details Each timer controls a group of output PWM channels. The change of the specific timer affects also the other related PWM channels.

typedef enum
{
    BC_PWM_TIM2_P0_P1_P2_P3,
    BC_PWM_TIM3_P6_P7_P8,
    BC_PWM_TIM21_P12_P14

}  bc_pwm_tim_t;

//! @brief Initialize PWM timers based on GPIO pin
//! @param[in] channel GPIO channel

void bc_pwm_init(bc_pwm_channel_t channel);

//! @brief Enable PWM output on GPIO pin
//! @param[channel] channel GPIO channel

void bc_pwm_enable(bc_pwm_channel_t channel);

//! @brief Disable PWM output on GPIO pin
//! @param[channel] channel GPIO channel

void bc_pwm_disable(bc_pwm_channel_t channel);

//! @brief Set PWM value for GPIO pin
//! @param[channel] channel GPIO channel
//! @param[pwm_value] PWM duty value 0-255 in default configuration

void bc_pwm_set(bc_pwm_channel_t channel, uint16_t pwm_value);

//! @brief Reconfigure TIM3
//! @param[resolution_us] Set resolution of the timer in microseconds
//! @param[period_cycles] Set number of periods of one cycle

void bc_pwm_tim_configure(bc_pwm_tim_t tim, uint32_t resolution_us, uint32_t period_cycles);

//! @}

#endif // _BC_PWM_H
