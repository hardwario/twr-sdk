#ifndef _TWR_GPIO_H
#define _TWR_GPIO_H

#include <twr_common.h>

//! @addtogroup twr_gpio twr_gpio
//! @brief Driver for general purpose input/output
//! @{

//! @brief GPIO channels

typedef enum
{
    //! @brief GPIO channel P0, A0, TXD0
    TWR_GPIO_P0 = 0,

    //! @brief GPIO channel P1, A1, RXD0
    TWR_GPIO_P1 = 1,

    //! @brief GPIO channel P2, A2, TXD1
    TWR_GPIO_P2 = 2,

    //! @brief GPIO channel P3, A3, RXD1
    TWR_GPIO_P3 = 3,

    //! @brief GPIO channel P4, A4, DAC0
    TWR_GPIO_P4 = 4,

    //! @brief GPIO channel P5, A5, DAC1
    TWR_GPIO_P5 = 5,

    //! @brief GPIO channel P6
    TWR_GPIO_P6 = 6,

    //! @brief GPIO channel P7, A6
    TWR_GPIO_P7 = 7,

    //! @brief GPIO channel P8
    TWR_GPIO_P8 = 8,

    //! @brief GPIO channel P9
    TWR_GPIO_P9 = 9,

    //! @brief GPIO channel P10, RXD2
    TWR_GPIO_P10 = 10,

    //! @brief GPIO channel P11, TXD2
    TWR_GPIO_P11 = 11,

    //! @brief GPIO channel P12, MISO
    TWR_GPIO_P12 = 12,

    //! @brief GPIO channel P13, MOSI
    TWR_GPIO_P13 = 13,

    //! @brief GPIO channel P14, SCLK
    TWR_GPIO_P14 = 14,

    //! @brief GPIO channel P15, CS
    TWR_GPIO_P15 = 15,

    //! @brief GPIO channel P16, SCL1
    TWR_GPIO_P16 = 16,

    //! @brief GPIO channel P17, SDA1
    TWR_GPIO_P17 = 17,

    //! @brief GPIO channel LED
    TWR_GPIO_LED = 18,

    //! @brief GPIO channel BUTTON
    TWR_GPIO_BUTTON = 19,

    //! @brief GPIO channel INT
    TWR_GPIO_INT = 20,

    //! @brief GPIO channel SCL0
    TWR_GPIO_SCL0 = 21,

    //! @brief GPIO channel SDA0
    TWR_GPIO_SDA0 = 22

} twr_gpio_channel_t;

//! @brief GPIO pull-up/pull-down setting

typedef enum
{
    //! @brief GPIO channel has no pull-up/pull-down
    TWR_GPIO_PULL_NONE = 0,

    //! @brief GPIO channel has pull-up
    TWR_GPIO_PULL_UP = 1,

    //! @brief GPIO channel has pull-down
    TWR_GPIO_PULL_DOWN = 2

} twr_gpio_pull_t;

//! @brief GPIO mode of operation

typedef enum
{
    //! @brief GPIO channel operates as input
    TWR_GPIO_MODE_INPUT = 0,

    //! @brief GPIO channel operates as output
    TWR_GPIO_MODE_OUTPUT = 1,

    //! @brief GPIO channel operates in alternate mode
    TWR_GPIO_MODE_ALTERNATE = 2,

    //! @brief GPIO channel operates in analog mode
    TWR_GPIO_MODE_ANALOG = 3,

    //! @brief GPIO channel operates as open-drain output
    TWR_GPIO_MODE_OUTPUT_OD = 4,

    //! @brief GPIO channel operates in alternate mode AF0
    TWR_GPIO_MODE_ALTERNATE_0 = (0 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF1
    TWR_GPIO_MODE_ALTERNATE_1 = (1 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF2
    TWR_GPIO_MODE_ALTERNATE_2 = (2 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF3
    TWR_GPIO_MODE_ALTERNATE_3 = (3 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF4
    TWR_GPIO_MODE_ALTERNATE_4 = (4 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF5
    TWR_GPIO_MODE_ALTERNATE_5 = (5 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF6
    TWR_GPIO_MODE_ALTERNATE_6 = (6 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF7
    TWR_GPIO_MODE_ALTERNATE_7 = (7 << 4) | 2

} twr_gpio_mode_t;

//! @brief Initialize GPIO channel
//! @param channel GPIO channel

void twr_gpio_init(twr_gpio_channel_t channel);

//! @brief Set pull-up/pull-down configuration for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] pull Desired pull-up/pull-down configuration

void twr_gpio_set_pull(twr_gpio_channel_t channel, twr_gpio_pull_t pull);

//! @brief Get pull-up/pull-down configuration for GPIO channel
//! @param[in] channel GPIO channel
//! @return Pull-up/pull-down configuration

twr_gpio_pull_t twr_gpio_get_pull(twr_gpio_channel_t channel);

//! @brief Set mode of operation for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] mode Desired mode of operation

void twr_gpio_set_mode(twr_gpio_channel_t channel, twr_gpio_mode_t mode);

//! @brief Get mode of operation for GPIO channel
//! @param[in] channel GPIO channel
//! @return Mode of operation

twr_gpio_mode_t twr_gpio_get_mode(twr_gpio_channel_t channel);

//! @brief Get input state for GPIO channel
//! @param channel GPIO channel
//! @return state Input state

int twr_gpio_get_input(twr_gpio_channel_t channel);

//! @brief Set output state for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] state Desired output state

void twr_gpio_set_output(twr_gpio_channel_t channel, int state);

//! @brief Get output state for GPIO channel
//! @param[in] channel GPIO channel
//! @return Output state

int twr_gpio_get_output(twr_gpio_channel_t channel);

//! @brief Toggle output state for GPIO channel
//! @param[in] channel GPIO channel

void twr_gpio_toggle_output(twr_gpio_channel_t channel);

//! @}

#endif // _TWR_GPIO_H
