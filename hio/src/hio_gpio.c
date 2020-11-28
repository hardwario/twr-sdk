#include <hio_gpio.h>
#include <hio_irq.h>
#include <stm32l0xx.h>

#define HIO_GPIO_CHANNEL_COUNT 21

#define HIO_GPIO_PORT_P0 GPIOA
#define HIO_GPIO_PORT_P1 GPIOA
#define HIO_GPIO_PORT_P2 GPIOA
#define HIO_GPIO_PORT_P3 GPIOA
#define HIO_GPIO_PORT_P4 GPIOA
#define HIO_GPIO_PORT_P5 GPIOA
#define HIO_GPIO_PORT_P6 GPIOB
#define HIO_GPIO_PORT_P7 GPIOA
#define HIO_GPIO_PORT_P8 GPIOB
#define HIO_GPIO_PORT_P9 GPIOB
#define HIO_GPIO_PORT_P10 GPIOA
#define HIO_GPIO_PORT_P11 GPIOA
#define HIO_GPIO_PORT_P12 GPIOB
#define HIO_GPIO_PORT_P13 GPIOB
#define HIO_GPIO_PORT_P14 GPIOB
#define HIO_GPIO_PORT_P15 GPIOB
#define HIO_GPIO_PORT_P16 GPIOB
#define HIO_GPIO_PORT_P17 GPIOB
#define HIO_GPIO_PORT_LED GPIOH
#define HIO_GPIO_PORT_BUTTON GPIOA
#define HIO_GPIO_PORT_INT GPIOC

#define HIO_GPIO_POS_P0 0
#define HIO_GPIO_POS_P1 1
#define HIO_GPIO_POS_P2 2
#define HIO_GPIO_POS_P3 3
#define HIO_GPIO_POS_P4 4
#define HIO_GPIO_POS_P5 5
#define HIO_GPIO_POS_P6 1
#define HIO_GPIO_POS_P7 6
#define HIO_GPIO_POS_P8 0
#define HIO_GPIO_POS_P9 2
#define HIO_GPIO_POS_P10 10
#define HIO_GPIO_POS_P11 9
#define HIO_GPIO_POS_P12 14
#define HIO_GPIO_POS_P13 15
#define HIO_GPIO_POS_P14 13
#define HIO_GPIO_POS_P15 12
#define HIO_GPIO_POS_P16 8
#define HIO_GPIO_POS_P17 9
#define HIO_GPIO_POS_LED 1
#define HIO_GPIO_POS_BUTTON 8
#define HIO_GPIO_POS_INT 13

#define _HIO_GPIO_MODE_MASK 0xf
#define _HIO_GPIO_MODE_AF_POS 4
#define  _hio_gpio_64_bit_pos(__CHANNEL__) (_hio_gpio_32_bit_pos[(__CHANNEL__)] << 1)

GPIO_TypeDef * const hio_gpio_port[HIO_GPIO_CHANNEL_COUNT] =
{
    HIO_GPIO_PORT_P0,
    HIO_GPIO_PORT_P1,
    HIO_GPIO_PORT_P2,
    HIO_GPIO_PORT_P3,
    HIO_GPIO_PORT_P4,
    HIO_GPIO_PORT_P5,
    HIO_GPIO_PORT_P6,
    HIO_GPIO_PORT_P7,
    HIO_GPIO_PORT_P8,
    HIO_GPIO_PORT_P9,
    HIO_GPIO_PORT_P10,
    HIO_GPIO_PORT_P11,
    HIO_GPIO_PORT_P12,
    HIO_GPIO_PORT_P13,
    HIO_GPIO_PORT_P14,
    HIO_GPIO_PORT_P15,
    HIO_GPIO_PORT_P16,
    HIO_GPIO_PORT_P17,
    HIO_GPIO_PORT_LED,
    HIO_GPIO_PORT_BUTTON,
    HIO_GPIO_PORT_INT
};

static const uint8_t _hio_gpio_iopenr_mask[HIO_GPIO_CHANNEL_COUNT] =
{
    RCC_IOPENR_GPIOAEN, // P0
    RCC_IOPENR_GPIOAEN, // P1
    RCC_IOPENR_GPIOAEN, // P2
    RCC_IOPENR_GPIOAEN, // P3
    RCC_IOPENR_GPIOAEN, // P4
    RCC_IOPENR_GPIOAEN, // P5
    RCC_IOPENR_GPIOBEN, // P6
    RCC_IOPENR_GPIOAEN, // P7
    RCC_IOPENR_GPIOBEN, // P8
    RCC_IOPENR_GPIOBEN, // P9
    RCC_IOPENR_GPIOAEN, // P10
    RCC_IOPENR_GPIOAEN, // P11
    RCC_IOPENR_GPIOBEN, // P12
    RCC_IOPENR_GPIOBEN, // P13
    RCC_IOPENR_GPIOBEN, // P14
    RCC_IOPENR_GPIOBEN, // P15
    RCC_IOPENR_GPIOBEN, // P16
    RCC_IOPENR_GPIOBEN, // P17
    RCC_IOPENR_GPIOHEN, // LED
    RCC_IOPENR_GPIOAEN, // BUTTON
    RCC_IOPENR_GPIOCEN  // INT
};

static const uint16_t _hio_gpio_16_bit_pos[HIO_GPIO_CHANNEL_COUNT] =
{
    HIO_GPIO_POS_P0,
    HIO_GPIO_POS_P1,
    HIO_GPIO_POS_P2,
    HIO_GPIO_POS_P3,
    HIO_GPIO_POS_P4,
    HIO_GPIO_POS_P5,
    HIO_GPIO_POS_P6,
    HIO_GPIO_POS_P7,
    HIO_GPIO_POS_P8,
    HIO_GPIO_POS_P9,
    HIO_GPIO_POS_P10,
    HIO_GPIO_POS_P11,
    HIO_GPIO_POS_P12,
    HIO_GPIO_POS_P13,
    HIO_GPIO_POS_P14,
    HIO_GPIO_POS_P15,
    HIO_GPIO_POS_P16,
    HIO_GPIO_POS_P17,
    HIO_GPIO_POS_LED,
    HIO_GPIO_POS_BUTTON,
    HIO_GPIO_POS_INT
};

static const uint16_t _hio_gpio_32_bit_pos[HIO_GPIO_CHANNEL_COUNT] =
{
    2 * HIO_GPIO_POS_P0,
    2 * HIO_GPIO_POS_P1,
    2 * HIO_GPIO_POS_P2,
    2 * HIO_GPIO_POS_P3,
    2 * HIO_GPIO_POS_P4,
    2 * HIO_GPIO_POS_P5,
    2 * HIO_GPIO_POS_P6,
    2 * HIO_GPIO_POS_P7,
    2 * HIO_GPIO_POS_P8,
    2 * HIO_GPIO_POS_P9,
    2 * HIO_GPIO_POS_P10,
    2 * HIO_GPIO_POS_P11,
    2 * HIO_GPIO_POS_P12,
    2 * HIO_GPIO_POS_P13,
    2 * HIO_GPIO_POS_P14,
    2 * HIO_GPIO_POS_P15,
    2 * HIO_GPIO_POS_P16,
    2 * HIO_GPIO_POS_P17,
    2 * HIO_GPIO_POS_LED,
    2 * HIO_GPIO_POS_BUTTON,
    2 * HIO_GPIO_POS_INT
};

const uint16_t hio_gpio_16_bit_mask[HIO_GPIO_CHANNEL_COUNT] =
{
    1 << HIO_GPIO_POS_P0,
    1 << HIO_GPIO_POS_P1,
    1 << HIO_GPIO_POS_P2,
    1 << HIO_GPIO_POS_P3,
    1 << HIO_GPIO_POS_P4,
    1 << HIO_GPIO_POS_P5,
    1 << HIO_GPIO_POS_P6,
    1 << HIO_GPIO_POS_P7,
    1 << HIO_GPIO_POS_P8,
    1 << HIO_GPIO_POS_P9,
    1 << HIO_GPIO_POS_P10,
    1 << HIO_GPIO_POS_P11,
    1 << HIO_GPIO_POS_P12,
    1 << HIO_GPIO_POS_P13,
    1 << HIO_GPIO_POS_P14,
    1 << HIO_GPIO_POS_P15,
    1 << HIO_GPIO_POS_P16,
    1 << HIO_GPIO_POS_P17,
    1 << HIO_GPIO_POS_LED,
    1 << HIO_GPIO_POS_BUTTON,
    1 << HIO_GPIO_POS_INT
};

static const uint32_t _hio_gpio_32_bit_mask[4][HIO_GPIO_CHANNEL_COUNT] =
{
    {
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL,
        0UL
    },
    {
        1UL << (2 * HIO_GPIO_POS_P0),
        1UL << (2 * HIO_GPIO_POS_P1),
        1UL << (2 * HIO_GPIO_POS_P2),
        1UL << (2 * HIO_GPIO_POS_P3),
        1UL << (2 * HIO_GPIO_POS_P4),
        1UL << (2 * HIO_GPIO_POS_P5),
        1UL << (2 * HIO_GPIO_POS_P6),
        1UL << (2 * HIO_GPIO_POS_P7),
        1UL << (2 * HIO_GPIO_POS_P8),
        1UL << (2 * HIO_GPIO_POS_P9),
        1UL << (2 * HIO_GPIO_POS_P10),
        1UL << (2 * HIO_GPIO_POS_P11),
        1UL << (2 * HIO_GPIO_POS_P12),
        1UL << (2 * HIO_GPIO_POS_P13),
        1UL << (2 * HIO_GPIO_POS_P14),
        1UL << (2 * HIO_GPIO_POS_P15),
        1UL << (2 * HIO_GPIO_POS_P16),
        1UL << (2 * HIO_GPIO_POS_P17),
        1UL << (2 * HIO_GPIO_POS_LED),
        1UL << (2 * HIO_GPIO_POS_BUTTON),
        1UL << (2 * HIO_GPIO_POS_INT)
    },
    {
        2UL << (2 * HIO_GPIO_POS_P0),
        2UL << (2 * HIO_GPIO_POS_P1),
        2UL << (2 * HIO_GPIO_POS_P2),
        2UL << (2 * HIO_GPIO_POS_P3),
        2UL << (2 * HIO_GPIO_POS_P4),
        2UL << (2 * HIO_GPIO_POS_P5),
        2UL << (2 * HIO_GPIO_POS_P6),
        2UL << (2 * HIO_GPIO_POS_P7),
        2UL << (2 * HIO_GPIO_POS_P8),
        2UL << (2 * HIO_GPIO_POS_P9),
        2UL << (2 * HIO_GPIO_POS_P10),
        2UL << (2 * HIO_GPIO_POS_P11),
        2UL << (2 * HIO_GPIO_POS_P12),
        2UL << (2 * HIO_GPIO_POS_P13),
        2UL << (2 * HIO_GPIO_POS_P14),
        2UL << (2 * HIO_GPIO_POS_P15),
        2UL << (2 * HIO_GPIO_POS_P16),
        2UL << (2 * HIO_GPIO_POS_P17),
        2UL << (2 * HIO_GPIO_POS_LED),
        2UL << (2 * HIO_GPIO_POS_BUTTON),
        2UL << (2 * HIO_GPIO_POS_INT)
    },
    {
        3UL << (2 * HIO_GPIO_POS_P0),
        3UL << (2 * HIO_GPIO_POS_P1),
        3UL << (2 * HIO_GPIO_POS_P2),
        3UL << (2 * HIO_GPIO_POS_P3),
        3UL << (2 * HIO_GPIO_POS_P4),
        3UL << (2 * HIO_GPIO_POS_P5),
        3UL << (2 * HIO_GPIO_POS_P6),
        3UL << (2 * HIO_GPIO_POS_P7),
        3UL << (2 * HIO_GPIO_POS_P8),
        3UL << (2 * HIO_GPIO_POS_P9),
        3UL << (2 * HIO_GPIO_POS_P10),
        3UL << (2 * HIO_GPIO_POS_P11),
        3UL << (2 * HIO_GPIO_POS_P12),
        3UL << (2 * HIO_GPIO_POS_P13),
        3UL << (2 * HIO_GPIO_POS_P14),
        3UL << (2 * HIO_GPIO_POS_P15),
        3UL << (2 * HIO_GPIO_POS_P16),
        3UL << (2 * HIO_GPIO_POS_P17),
        3UL << (2 * HIO_GPIO_POS_LED),
        3UL << (2 * HIO_GPIO_POS_BUTTON),
        3UL << (2 * HIO_GPIO_POS_INT)
    }
};

const uint32_t hio_gpio_32_bit_upper_mask[HIO_GPIO_CHANNEL_COUNT] =
{
    (1UL << 16) << HIO_GPIO_POS_P0,
    (1UL << 16) << HIO_GPIO_POS_P1,
    (1UL << 16) << HIO_GPIO_POS_P2,
    (1UL << 16) << HIO_GPIO_POS_P3,
    (1UL << 16) << HIO_GPIO_POS_P4,
    (1UL << 16) << HIO_GPIO_POS_P5,
    (1UL << 16) << HIO_GPIO_POS_P6,
    (1UL << 16) << HIO_GPIO_POS_P7,
    (1UL << 16) << HIO_GPIO_POS_P8,
    (1UL << 16) << HIO_GPIO_POS_P9,
    (1UL << 16) << HIO_GPIO_POS_P10,
    (1UL << 16) << HIO_GPIO_POS_P11,
    (1UL << 16) << HIO_GPIO_POS_P12,
    (1UL << 16) << HIO_GPIO_POS_P13,
    (1UL << 16) << HIO_GPIO_POS_P14,
    (1UL << 16) << HIO_GPIO_POS_P15,
    (1UL << 16) << HIO_GPIO_POS_P16,
    (1UL << 16) << HIO_GPIO_POS_P17,
    (1UL << 16) << HIO_GPIO_POS_LED,
    (1UL << 16) << HIO_GPIO_POS_BUTTON,
    (1UL << 16) << HIO_GPIO_POS_INT
};

void hio_gpio_init(hio_gpio_channel_t channel)
{
    // Disable interrupts
    hio_irq_disable();

    // Enable GPIO clock
    RCC->IOPENR |= _hio_gpio_iopenr_mask[channel];

    // Enable interrupts
    hio_irq_enable();
}

void hio_gpio_set_pull(hio_gpio_channel_t channel, hio_gpio_pull_t pull)
{
    // Disable interrupts
    hio_irq_disable();

    // Read PUPDR register
    uint32_t pupdr = hio_gpio_port[channel]->PUPDR;

    // Reset corresponding PUPDR bits
    pupdr &= ~_hio_gpio_32_bit_mask[3][channel];

    // Set corresponding PUPDR bits
    pupdr |= _hio_gpio_32_bit_mask[pull][channel];

    // Write PUPDR register
    hio_gpio_port[channel]->PUPDR = pupdr;

    // Enable interrupts
    hio_irq_enable();
}

hio_gpio_pull_t hio_gpio_get_pull(hio_gpio_channel_t channel)
{
    // Return pull setting from PUPDR register
    return (hio_gpio_pull_t) ((hio_gpio_port[channel]->PUPDR >> _hio_gpio_32_bit_pos[channel]) & 3);
}

void hio_gpio_set_mode(hio_gpio_channel_t channel, hio_gpio_mode_t mode)
{
    // Disable interrupts
    hio_irq_disable();

    // Read OTYPER register
    uint32_t otyper = hio_gpio_port[channel]->OTYPER;

    // Read MODER register
    uint32_t moder = hio_gpio_port[channel]->MODER;

    // If mode setting is open-drain output...
    if (mode == HIO_GPIO_MODE_OUTPUT_OD)
    {
        // Set corresponding OTYPER bit
        otyper |= hio_gpio_16_bit_mask[channel];

        // Override desired mode setting to output
        mode = HIO_GPIO_MODE_OUTPUT;
    }
    else
    {
        // Reset corresponding OTYPER bit
        otyper &= ~hio_gpio_16_bit_mask[channel];
    }

    // If mode setting is alternative function ...
    if((mode & _HIO_GPIO_MODE_MASK) == HIO_GPIO_MODE_ALTERNATE)
    {
        // ... write AF number to appropriate pin of appropriate AFR register
        if(_hio_gpio_16_bit_pos[channel] >= 8)
        {
            hio_gpio_port[channel]->AFR[1] |= ((uint8_t)mode >> _HIO_GPIO_MODE_AF_POS) << (_hio_gpio_64_bit_pos(channel) - 32);
        }
        else
        {
            hio_gpio_port[channel]->AFR[0] |= ((uint8_t)mode >> _HIO_GPIO_MODE_AF_POS) << _hio_gpio_64_bit_pos(channel);
        }

        // Mask AF number (mode is used as a coordinates in the array below ...)
        mode &= _HIO_GPIO_MODE_MASK;
    }

    // Reset corresponding MODER bits
    moder &= ~_hio_gpio_32_bit_mask[3][channel];

    // Set corresponding MODER bits
    moder |= _hio_gpio_32_bit_mask[mode][channel];

    // Write OTYPER register
    hio_gpio_port[channel]->OTYPER = otyper;

    // Write MODER register
    hio_gpio_port[channel]->MODER = moder;

    // Enable interrupts
    hio_irq_enable();
}

hio_gpio_mode_t hio_gpio_get_mode(hio_gpio_channel_t channel)
{
    // Read mode setting from MODER register
    hio_gpio_mode_t mode = (hio_gpio_mode_t) ((hio_gpio_port[channel]->MODER >> _hio_gpio_32_bit_pos[channel]) & 3);

    // If mode setting is output...
    if (mode == HIO_GPIO_MODE_OUTPUT)
    {
        // If TYPER register bit indicates open-drain output...
        if (((hio_gpio_port[channel]->OTYPER >> _hio_gpio_16_bit_pos[channel]) & 1) != 0)
        {
            // Override mode setting to open-drain
            mode = HIO_GPIO_MODE_OUTPUT_OD;
        }
    }

    // If mode setting is alternative function ...
    else if (mode == HIO_GPIO_MODE_ALTERNATE)
    {
        // Readout AF number from appropriate AFR
        if(_hio_gpio_16_bit_pos[channel] >= 8)
        {
            mode = (hio_gpio_port[channel]->AFR[1] >> (_hio_gpio_64_bit_pos(channel) - 32)) & 0x0f;
        }
        else
        {
            mode = (hio_gpio_port[channel]->AFR[0] >> _hio_gpio_64_bit_pos(channel)) & 0x0f;
        }

        // Insert number to enumeration
        mode = (mode << _HIO_GPIO_MODE_AF_POS) | HIO_GPIO_MODE_ALTERNATE;
    }

    // Return mode setting
    return mode;
}

int hio_gpio_get_input(hio_gpio_channel_t channel)
{
    // Return GPIO state from IDR register
    return (hio_gpio_port[channel]->IDR & hio_gpio_16_bit_mask[channel]) != 0 ? 1 : 0;
}

void hio_gpio_set_output(hio_gpio_channel_t channel, int state)
{
    // Write GPIO state to BSRR register
    hio_gpio_port[channel]->BSRR = state ? hio_gpio_16_bit_mask[channel] : hio_gpio_32_bit_upper_mask[channel];
}

int hio_gpio_get_output(hio_gpio_channel_t channel)
{
    // Return GPIO state from ODR register
    return (hio_gpio_port[channel]->ODR & hio_gpio_16_bit_mask[channel]) != 0 ? 1 : 0;
}

void hio_gpio_toggle_output(hio_gpio_channel_t channel)
{
    // Disable interrupts
    hio_irq_disable();

    // Write ODR register with inverted bit
    hio_gpio_port[channel]->ODR ^= hio_gpio_16_bit_mask[channel];

    // Enable interrupts
    hio_irq_enable();
}
