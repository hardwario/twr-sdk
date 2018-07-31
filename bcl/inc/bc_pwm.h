#ifndef _BC_PWM_H
#define _BC_PWM_H

#include <bc_system.h>
#include <bc_gpio.h>
#include <stm32l0xx.h>

/* example code

    For Pavel: will be removed in the final version ;)

    bc_pwm_init(BC_GPIO_P0);
    bc_pwm_set(BC_GPIO_P0, 10);
    bc_pwm_enable(BC_GPIO_P0);

    bc_pwm_init(BC_GPIO_P1);
    bc_pwm_set(BC_GPIO_P1, 40);
    bc_pwm_enable(BC_GPIO_P1);

    bc_pwm_init(BC_GPIO_P2);
    bc_pwm_set(BC_GPIO_P2, 80);
    bc_pwm_enable(BC_GPIO_P2);

    bc_pwm_init(BC_GPIO_P3);
    bc_pwm_set(BC_GPIO_P3, 120);
    bc_pwm_enable(BC_GPIO_P3);

    // TIM3
    bc_pwm_init(BC_GPIO_P6);
    bc_pwm_set(BC_GPIO_P6, 180);
    bc_pwm_enable(BC_GPIO_P6);

    bc_pwm_init(BC_GPIO_P7);
    bc_pwm_set(BC_GPIO_P7, 210);
    bc_pwm_enable(BC_GPIO_P7);

    bc_pwm_init(BC_GPIO_P8);
    bc_pwm_set(BC_GPIO_P8, 255);
    bc_pwm_enable(BC_GPIO_P8);

*/

//! @addtogroup bc_pwm bc_pwm
//! @brief Driver for timer
//! @{

//! @brief Initialize PWM timers

void bc_pwm_init(bc_gpio_channel_t channel);

//! @brief Enable PWM output

void bc_pwm_enable(bc_gpio_channel_t channel);

//! @brief Set PWM value

void bc_pwm_set(bc_gpio_channel_t channel, uint16_t pwm_value);

void bc_pwm_tim3_init(uint32_t resolution_us, uint32_t period_cycles);

void bc_pwm_tim2_init(uint32_t resolution_us, uint32_t period_cycles);



//! @}

#endif // _BC_PWM_H
