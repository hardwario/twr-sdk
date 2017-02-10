#ifndef _BC_GPIO_H
#define _BC_GPIO_H

#include <bc_common.h>

//! @addtogroup bc_gpio bc_gpio
//! @brief Driver for general purpose input/output
//! @{

//! @brief GPIO channels

typedef enum
{
    BC_GPIO_P0 = 0, //!< GPIO channel P0
    BC_GPIO_P1 = 1, //!< GPIO channel P1
    BC_GPIO_P2 = 2, //!< GPIO channel P2
    BC_GPIO_P3 = 3, //!< GPIO channel P3
    BC_GPIO_P4 = 4, //!< GPIO channel P4
    BC_GPIO_P5 = 5, //!< GPIO channel P5
    BC_GPIO_P6 = 6, //!< GPIO channel P6
    BC_GPIO_P7 = 7, //!< GPIO channel P7
    BC_GPIO_P8 = 8, //!< GPIO channel P8
    BC_GPIO_P9 = 9, //!< GPIO channel P9
    BC_GPIO_P10 = 10, //!< GPIO channel P10
    BC_GPIO_P11 = 11, //!< GPIO channel P11
    BC_GPIO_P12 = 12, //!< GPIO channel P12
    BC_GPIO_P13 = 13, //!< GPIO channel P13
    BC_GPIO_P14 = 14, //!< GPIO channel P14
    BC_GPIO_P15 = 15, //!< GPIO channel P15
    BC_GPIO_P16 = 16, //!< GPIO channel P16
    BC_GPIO_P17 = 17, //!< GPIO channel P17
    BC_GPIO_LED = 18, //!< GPIO channel LED
    BC_GPIO_BUTTON = 19, //!< GPIO channel BUTTON

} bc_gpio_channel_t;

//! @brief GPIO pull-up/pull-down setting

typedef enum
{
    BC_GPIO_PULL_NONE = 0, //!< GPIO channel has no pull-up/pull-down
    BC_GPIO_PULL_UP = 1, //!< GPIO channel has pull-up
    BC_GPIO_PULL_DOWN = 2 //!< GPIO channel has pull-down

} bc_gpio_pull_t;

//! @brief GPIO mode of operation

typedef enum
{
    BC_GPIO_MODE_INPUT = 0, //!< GPIO channel operates as input
    BC_GPIO_MODE_OUTPUT = 1, //!< GPIO channel operates as output
    BC_GPIO_MODE_ALTERNATE = 2, //!< GPIO channel operates in alternate mode
    BC_GPIO_MODE_ANALOG = 3, //!< GPIO channel operates in analog mode
    BC_GPIO_MODE_OUTPUT_OD = 4 //!< GPIO channel operates as open-drain output

} bc_gpio_mode_t;

//! @brief Initialize GPIO channel
//! @param channel GPIO channel

void bc_gpio_init(bc_gpio_channel_t channel);

//! @brief Set pull-up/pull-down configuration for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] pull Desired pull-up/pull-down configuration

void bc_gpio_set_pull(bc_gpio_channel_t channel, bc_gpio_pull_t pull);

//! @brief Get pull-up/pull-down configuration for GPIO channel
//! @param[in] channel GPIO channel
//! @return Pull-up/pull-down configuration

bc_gpio_pull_t bc_gpio_get_pull(bc_gpio_channel_t channel);

//! @brief Set mode of operation for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] mode Desired mode of operation

void bc_gpio_set_mode(bc_gpio_channel_t channel, bc_gpio_mode_t mode);

//! @brief Get mode of operation for GPIO channel
//! @param[in] channel GPIO channel
//! @return Mode of operation

bc_gpio_mode_t bc_gpio_get_mode(bc_gpio_channel_t channel);

//! @brief Get input state for GPIO channel
//! @param channel GPIO channel
//! @return state Input state

bool bc_gpio_get_input(bc_gpio_channel_t channel);

//! @brief Set output state for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] state Desired output state

void bc_gpio_set_output(bc_gpio_channel_t channel, bool state);

//! @brief Get output state for GPIO channel
//! @param[in] channel GPIO channel
//! @return Output state

bool bc_gpio_get_output(bc_gpio_channel_t channel);

//! @brief Toggle output state for GPIO channel
//! @param[in] channel GPIO channel

void bc_gpio_toggle_output(bc_gpio_channel_t channel);

//! @}

#endif // _BC_GPIO_H
