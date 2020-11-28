#ifndef _HIO_GPIO_H
#define _HIO_GPIO_H

#include <hio_common.h>

//! @addtogroup hio_gpio hio_gpio
//! @brief Driver for general purpose input/output
//! @{

//! @brief GPIO channels

typedef enum
{
    //! @brief GPIO channel P0
    HIO_GPIO_P0 = 0,

    //! @brief GPIO channel P1
    HIO_GPIO_P1 = 1,

    //! @brief GPIO channel P2
    HIO_GPIO_P2 = 2,

    //! @brief GPIO channel P3
    HIO_GPIO_P3 = 3,

    //! @brief GPIO channel P4
    HIO_GPIO_P4 = 4,

    //! @brief GPIO channel P5
    HIO_GPIO_P5 = 5,

    //! @brief GPIO channel P6
    HIO_GPIO_P6 = 6,

    //! @brief GPIO channel P7
    HIO_GPIO_P7 = 7,

    //! @brief GPIO channel P8
    HIO_GPIO_P8 = 8,

    //! @brief GPIO channel P9
    HIO_GPIO_P9 = 9,

    //! @brief GPIO channel P10
    HIO_GPIO_P10 = 10,

    //! @brief GPIO channel P11
    HIO_GPIO_P11 = 11,

    //! @brief GPIO channel P12
    HIO_GPIO_P12 = 12,

    //! @brief GPIO channel P13
    HIO_GPIO_P13 = 13,

    //! @brief GPIO channel P14
    HIO_GPIO_P14 = 14,

    //! @brief GPIO channel P15
    HIO_GPIO_P15 = 15,

    //! @brief GPIO channel P16
    HIO_GPIO_P16 = 16,

    //! @brief GPIO channel P17
    HIO_GPIO_P17 = 17,

    //! @brief GPIO channel LED
    HIO_GPIO_LED = 18,

    //! @brief GPIO channel BUTTON
    HIO_GPIO_BUTTON = 19,

    //! @brief GPIO channel BUTTON
    HIO_GPIO_INT = 20

} hio_gpio_channel_t;

//! @brief GPIO pull-up/pull-down setting

typedef enum
{
    //! @brief GPIO channel has no pull-up/pull-down
    HIO_GPIO_PULL_NONE = 0,

    //! @brief GPIO channel has pull-up
    HIO_GPIO_PULL_UP = 1,

    //! @brief GPIO channel has pull-down
    HIO_GPIO_PULL_DOWN = 2

} hio_gpio_pull_t;

//! @brief GPIO mode of operation

typedef enum
{
    //! @brief GPIO channel operates as input
    HIO_GPIO_MODE_INPUT = 0,

    //! @brief GPIO channel operates as output
    HIO_GPIO_MODE_OUTPUT = 1,

    //! @brief GPIO channel operates in alternate mode
    HIO_GPIO_MODE_ALTERNATE = 2,

    //! @brief GPIO channel operates in analog mode
    HIO_GPIO_MODE_ANALOG = 3,

    //! @brief GPIO channel operates as open-drain output
    HIO_GPIO_MODE_OUTPUT_OD = 4,

    //! @brief GPIO channel operates in alternate mode AF0
    HIO_GPIO_MODE_ALTERNATE_0 = (0 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF1
    HIO_GPIO_MODE_ALTERNATE_1 = (1 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF2
    HIO_GPIO_MODE_ALTERNATE_2 = (2 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF3
    HIO_GPIO_MODE_ALTERNATE_3 = (3 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF4
    HIO_GPIO_MODE_ALTERNATE_4 = (4 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF5
    HIO_GPIO_MODE_ALTERNATE_5 = (5 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF6
    HIO_GPIO_MODE_ALTERNATE_6 = (6 << 4) | 2,

    //! @brief GPIO channel operates in alternate mode AF7
    HIO_GPIO_MODE_ALTERNATE_7 = (7 << 4) | 2

} hio_gpio_mode_t;

//! @brief Initialize GPIO channel
//! @param channel GPIO channel

void hio_gpio_init(hio_gpio_channel_t channel);

//! @brief Set pull-up/pull-down configuration for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] pull Desired pull-up/pull-down configuration

void hio_gpio_set_pull(hio_gpio_channel_t channel, hio_gpio_pull_t pull);

//! @brief Get pull-up/pull-down configuration for GPIO channel
//! @param[in] channel GPIO channel
//! @return Pull-up/pull-down configuration

hio_gpio_pull_t hio_gpio_get_pull(hio_gpio_channel_t channel);

//! @brief Set mode of operation for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] mode Desired mode of operation

void hio_gpio_set_mode(hio_gpio_channel_t channel, hio_gpio_mode_t mode);

//! @brief Get mode of operation for GPIO channel
//! @param[in] channel GPIO channel
//! @return Mode of operation

hio_gpio_mode_t hio_gpio_get_mode(hio_gpio_channel_t channel);

//! @brief Get input state for GPIO channel
//! @param channel GPIO channel
//! @return state Input state

int hio_gpio_get_input(hio_gpio_channel_t channel);

//! @brief Set output state for GPIO channel
//! @param[in] channel GPIO channel
//! @param[in] state Desired output state

void hio_gpio_set_output(hio_gpio_channel_t channel, int state);

//! @brief Get output state for GPIO channel
//! @param[in] channel GPIO channel
//! @return Output state

int hio_gpio_get_output(hio_gpio_channel_t channel);

//! @brief Toggle output state for GPIO channel
//! @param[in] channel GPIO channel

void hio_gpio_toggle_output(hio_gpio_channel_t channel);

//! @}

#endif // _HIO_GPIO_H
