#include <twr_gpio.h>
#include <twr_irq.h>
#include <stm32l0xx.h>

#define TWR_GPIO_CHANNEL_COUNT 23

#define TWR_GPIO_PORT_P0 GPIOA
#define TWR_GPIO_PORT_P1 GPIOA
#define TWR_GPIO_PORT_P2 GPIOA
#define TWR_GPIO_PORT_P3 GPIOA
#define TWR_GPIO_PORT_P4 GPIOA
#define TWR_GPIO_PORT_P5 GPIOA
#define TWR_GPIO_PORT_P6 GPIOB
#define TWR_GPIO_PORT_P7 GPIOA
#define TWR_GPIO_PORT_P8 GPIOB
#define TWR_GPIO_PORT_P9 GPIOB
#define TWR_GPIO_PORT_P10 GPIOA
#define TWR_GPIO_PORT_P11 GPIOA
#define TWR_GPIO_PORT_P12 GPIOB
#define TWR_GPIO_PORT_P13 GPIOB
#define TWR_GPIO_PORT_P14 GPIOB
#define TWR_GPIO_PORT_P15 GPIOB
#define TWR_GPIO_PORT_P16 GPIOB
#define TWR_GPIO_PORT_P17 GPIOB
#define TWR_GPIO_PORT_LED GPIOH
#define TWR_GPIO_PORT_BUTTON GPIOA
#define TWR_GPIO_PORT_INT GPIOC
#define TWR_GPIO_PORT_SCL0 GPIOB
#define TWR_GPIO_PORT_SDA0 GPIOB

#define TWR_GPIO_POS_P0 0
#define TWR_GPIO_POS_P1 1
#define TWR_GPIO_POS_P2 2
#define TWR_GPIO_POS_P3 3
#define TWR_GPIO_POS_P4 4
#define TWR_GPIO_POS_P5 5
#define TWR_GPIO_POS_P6 1
#define TWR_GPIO_POS_P7 6
#define TWR_GPIO_POS_P8 0
#define TWR_GPIO_POS_P9 2
#define TWR_GPIO_POS_P10 10
#define TWR_GPIO_POS_P11 9
#define TWR_GPIO_POS_P12 14
#define TWR_GPIO_POS_P13 15
#define TWR_GPIO_POS_P14 13
#define TWR_GPIO_POS_P15 12
#define TWR_GPIO_POS_P16 8
#define TWR_GPIO_POS_P17 9
#define TWR_GPIO_POS_LED 1
#define TWR_GPIO_POS_BUTTON 8
#define TWR_GPIO_POS_INT 13
#define TWR_GPIO_POS_SCL0 10
#define TWR_GPIO_POS_SDA0 11

#define _TWR_GPIO_MODE_MASK 0xf
#define _TWR_GPIO_MODE_AF_POS 4
#define  _twr_gpio_64_bit_pos(__CHANNEL__) (_twr_gpio_32_bit_pos[(__CHANNEL__)] << 1)

GPIO_TypeDef * const twr_gpio_port[TWR_GPIO_CHANNEL_COUNT] =
{
    TWR_GPIO_PORT_P0,
    TWR_GPIO_PORT_P1,
    TWR_GPIO_PORT_P2,
    TWR_GPIO_PORT_P3,
    TWR_GPIO_PORT_P4,
    TWR_GPIO_PORT_P5,
    TWR_GPIO_PORT_P6,
    TWR_GPIO_PORT_P7,
    TWR_GPIO_PORT_P8,
    TWR_GPIO_PORT_P9,
    TWR_GPIO_PORT_P10,
    TWR_GPIO_PORT_P11,
    TWR_GPIO_PORT_P12,
    TWR_GPIO_PORT_P13,
    TWR_GPIO_PORT_P14,
    TWR_GPIO_PORT_P15,
    TWR_GPIO_PORT_P16,
    TWR_GPIO_PORT_P17,
    TWR_GPIO_PORT_LED,
    TWR_GPIO_PORT_BUTTON,
    TWR_GPIO_PORT_INT,
    TWR_GPIO_PORT_SCL0,
    TWR_GPIO_PORT_SDA0
};

static const uint8_t _twr_gpio_iopenr_mask[TWR_GPIO_CHANNEL_COUNT] =
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
    RCC_IOPENR_GPIOCEN, // INT
    RCC_IOPENR_GPIOBEN, // SCL0
    RCC_IOPENR_GPIOBEN, // SDA0
};

static const uint16_t _twr_gpio_16_bit_pos[TWR_GPIO_CHANNEL_COUNT] =
{
    TWR_GPIO_POS_P0,
    TWR_GPIO_POS_P1,
    TWR_GPIO_POS_P2,
    TWR_GPIO_POS_P3,
    TWR_GPIO_POS_P4,
    TWR_GPIO_POS_P5,
    TWR_GPIO_POS_P6,
    TWR_GPIO_POS_P7,
    TWR_GPIO_POS_P8,
    TWR_GPIO_POS_P9,
    TWR_GPIO_POS_P10,
    TWR_GPIO_POS_P11,
    TWR_GPIO_POS_P12,
    TWR_GPIO_POS_P13,
    TWR_GPIO_POS_P14,
    TWR_GPIO_POS_P15,
    TWR_GPIO_POS_P16,
    TWR_GPIO_POS_P17,
    TWR_GPIO_POS_LED,
    TWR_GPIO_POS_BUTTON,
    TWR_GPIO_POS_INT,
    TWR_GPIO_POS_SCL0,
    TWR_GPIO_POS_SDA0
};

static const uint16_t _twr_gpio_32_bit_pos[TWR_GPIO_CHANNEL_COUNT] =
{
    2 * TWR_GPIO_POS_P0,
    2 * TWR_GPIO_POS_P1,
    2 * TWR_GPIO_POS_P2,
    2 * TWR_GPIO_POS_P3,
    2 * TWR_GPIO_POS_P4,
    2 * TWR_GPIO_POS_P5,
    2 * TWR_GPIO_POS_P6,
    2 * TWR_GPIO_POS_P7,
    2 * TWR_GPIO_POS_P8,
    2 * TWR_GPIO_POS_P9,
    2 * TWR_GPIO_POS_P10,
    2 * TWR_GPIO_POS_P11,
    2 * TWR_GPIO_POS_P12,
    2 * TWR_GPIO_POS_P13,
    2 * TWR_GPIO_POS_P14,
    2 * TWR_GPIO_POS_P15,
    2 * TWR_GPIO_POS_P16,
    2 * TWR_GPIO_POS_P17,
    2 * TWR_GPIO_POS_LED,
    2 * TWR_GPIO_POS_BUTTON,
    2 * TWR_GPIO_POS_INT,
    2 * TWR_GPIO_POS_SCL0,
    2 * TWR_GPIO_POS_SDA0
};

const uint16_t twr_gpio_16_bit_mask[TWR_GPIO_CHANNEL_COUNT] =
{
    1 << TWR_GPIO_POS_P0,
    1 << TWR_GPIO_POS_P1,
    1 << TWR_GPIO_POS_P2,
    1 << TWR_GPIO_POS_P3,
    1 << TWR_GPIO_POS_P4,
    1 << TWR_GPIO_POS_P5,
    1 << TWR_GPIO_POS_P6,
    1 << TWR_GPIO_POS_P7,
    1 << TWR_GPIO_POS_P8,
    1 << TWR_GPIO_POS_P9,
    1 << TWR_GPIO_POS_P10,
    1 << TWR_GPIO_POS_P11,
    1 << TWR_GPIO_POS_P12,
    1 << TWR_GPIO_POS_P13,
    1 << TWR_GPIO_POS_P14,
    1 << TWR_GPIO_POS_P15,
    1 << TWR_GPIO_POS_P16,
    1 << TWR_GPIO_POS_P17,
    1 << TWR_GPIO_POS_LED,
    1 << TWR_GPIO_POS_BUTTON,
    1 << TWR_GPIO_POS_INT,
    1 << TWR_GPIO_POS_SCL0,
    1 << TWR_GPIO_POS_SDA0
};

static const uint32_t _twr_gpio_32_bit_mask[4][TWR_GPIO_CHANNEL_COUNT] =
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
        0UL,
        0UL,
        0UL,
    },
    {
        1UL << (2 * TWR_GPIO_POS_P0),
        1UL << (2 * TWR_GPIO_POS_P1),
        1UL << (2 * TWR_GPIO_POS_P2),
        1UL << (2 * TWR_GPIO_POS_P3),
        1UL << (2 * TWR_GPIO_POS_P4),
        1UL << (2 * TWR_GPIO_POS_P5),
        1UL << (2 * TWR_GPIO_POS_P6),
        1UL << (2 * TWR_GPIO_POS_P7),
        1UL << (2 * TWR_GPIO_POS_P8),
        1UL << (2 * TWR_GPIO_POS_P9),
        1UL << (2 * TWR_GPIO_POS_P10),
        1UL << (2 * TWR_GPIO_POS_P11),
        1UL << (2 * TWR_GPIO_POS_P12),
        1UL << (2 * TWR_GPIO_POS_P13),
        1UL << (2 * TWR_GPIO_POS_P14),
        1UL << (2 * TWR_GPIO_POS_P15),
        1UL << (2 * TWR_GPIO_POS_P16),
        1UL << (2 * TWR_GPIO_POS_P17),
        1UL << (2 * TWR_GPIO_POS_LED),
        1UL << (2 * TWR_GPIO_POS_BUTTON),
        1UL << (2 * TWR_GPIO_POS_INT),
        1UL << (2 * TWR_GPIO_POS_SCL0),
        1UL << (2 * TWR_GPIO_POS_SDA0)
    },
    {
        2UL << (2 * TWR_GPIO_POS_P0),
        2UL << (2 * TWR_GPIO_POS_P1),
        2UL << (2 * TWR_GPIO_POS_P2),
        2UL << (2 * TWR_GPIO_POS_P3),
        2UL << (2 * TWR_GPIO_POS_P4),
        2UL << (2 * TWR_GPIO_POS_P5),
        2UL << (2 * TWR_GPIO_POS_P6),
        2UL << (2 * TWR_GPIO_POS_P7),
        2UL << (2 * TWR_GPIO_POS_P8),
        2UL << (2 * TWR_GPIO_POS_P9),
        2UL << (2 * TWR_GPIO_POS_P10),
        2UL << (2 * TWR_GPIO_POS_P11),
        2UL << (2 * TWR_GPIO_POS_P12),
        2UL << (2 * TWR_GPIO_POS_P13),
        2UL << (2 * TWR_GPIO_POS_P14),
        2UL << (2 * TWR_GPIO_POS_P15),
        2UL << (2 * TWR_GPIO_POS_P16),
        2UL << (2 * TWR_GPIO_POS_P17),
        2UL << (2 * TWR_GPIO_POS_LED),
        2UL << (2 * TWR_GPIO_POS_BUTTON),
        2UL << (2 * TWR_GPIO_POS_INT),
        2UL << (2 * TWR_GPIO_POS_SCL0),
        2UL << (2 * TWR_GPIO_POS_SDA0)
    },
    {
        3UL << (2 * TWR_GPIO_POS_P0),
        3UL << (2 * TWR_GPIO_POS_P1),
        3UL << (2 * TWR_GPIO_POS_P2),
        3UL << (2 * TWR_GPIO_POS_P3),
        3UL << (2 * TWR_GPIO_POS_P4),
        3UL << (2 * TWR_GPIO_POS_P5),
        3UL << (2 * TWR_GPIO_POS_P6),
        3UL << (2 * TWR_GPIO_POS_P7),
        3UL << (2 * TWR_GPIO_POS_P8),
        3UL << (2 * TWR_GPIO_POS_P9),
        3UL << (2 * TWR_GPIO_POS_P10),
        3UL << (2 * TWR_GPIO_POS_P11),
        3UL << (2 * TWR_GPIO_POS_P12),
        3UL << (2 * TWR_GPIO_POS_P13),
        3UL << (2 * TWR_GPIO_POS_P14),
        3UL << (2 * TWR_GPIO_POS_P15),
        3UL << (2 * TWR_GPIO_POS_P16),
        3UL << (2 * TWR_GPIO_POS_P17),
        3UL << (2 * TWR_GPIO_POS_LED),
        3UL << (2 * TWR_GPIO_POS_BUTTON),
        3UL << (2 * TWR_GPIO_POS_INT),
        3UL << (2 * TWR_GPIO_POS_SCL0),
        3UL << (2 * TWR_GPIO_POS_SDA0)
    }
};

const uint32_t twr_gpio_32_bit_upper_mask[TWR_GPIO_CHANNEL_COUNT] =
{
    (1UL << 16) << TWR_GPIO_POS_P0,
    (1UL << 16) << TWR_GPIO_POS_P1,
    (1UL << 16) << TWR_GPIO_POS_P2,
    (1UL << 16) << TWR_GPIO_POS_P3,
    (1UL << 16) << TWR_GPIO_POS_P4,
    (1UL << 16) << TWR_GPIO_POS_P5,
    (1UL << 16) << TWR_GPIO_POS_P6,
    (1UL << 16) << TWR_GPIO_POS_P7,
    (1UL << 16) << TWR_GPIO_POS_P8,
    (1UL << 16) << TWR_GPIO_POS_P9,
    (1UL << 16) << TWR_GPIO_POS_P10,
    (1UL << 16) << TWR_GPIO_POS_P11,
    (1UL << 16) << TWR_GPIO_POS_P12,
    (1UL << 16) << TWR_GPIO_POS_P13,
    (1UL << 16) << TWR_GPIO_POS_P14,
    (1UL << 16) << TWR_GPIO_POS_P15,
    (1UL << 16) << TWR_GPIO_POS_P16,
    (1UL << 16) << TWR_GPIO_POS_P17,
    (1UL << 16) << TWR_GPIO_POS_LED,
    (1UL << 16) << TWR_GPIO_POS_BUTTON,
    (1UL << 16) << TWR_GPIO_POS_INT,
    (1UL << 16) << TWR_GPIO_POS_SCL0,
    (1UL << 16) << TWR_GPIO_POS_SDA0
};

void twr_gpio_init(twr_gpio_channel_t channel)
{
    // Disable interrupts
    twr_irq_disable();

    // Enable GPIO clock
    RCC->IOPENR |= _twr_gpio_iopenr_mask[channel];

    // Errata workaround
    RCC->IOPENR;

    // Enable interrupts
    twr_irq_enable();
}

void twr_gpio_set_pull(twr_gpio_channel_t channel, twr_gpio_pull_t pull)
{
    // Disable interrupts
    twr_irq_disable();

    // Read PUPDR register
    uint32_t pupdr = twr_gpio_port[channel]->PUPDR;

    // Reset corresponding PUPDR bits
    pupdr &= ~_twr_gpio_32_bit_mask[3][channel];

    // Set corresponding PUPDR bits
    pupdr |= _twr_gpio_32_bit_mask[pull][channel];

    // Write PUPDR register
    twr_gpio_port[channel]->PUPDR = pupdr;

    // Enable interrupts
    twr_irq_enable();
}

twr_gpio_pull_t twr_gpio_get_pull(twr_gpio_channel_t channel)
{
    // Return pull setting from PUPDR register
    return (twr_gpio_pull_t) ((twr_gpio_port[channel]->PUPDR >> _twr_gpio_32_bit_pos[channel]) & 3);
}

void twr_gpio_set_mode(twr_gpio_channel_t channel, twr_gpio_mode_t mode)
{
    // Disable interrupts
    twr_irq_disable();

    // Read OTYPER register
    uint32_t otyper = twr_gpio_port[channel]->OTYPER;

    // Read MODER register
    uint32_t moder = twr_gpio_port[channel]->MODER;

    // If mode setting is open-drain output...
    if (mode == TWR_GPIO_MODE_OUTPUT_OD)
    {
        // Set corresponding OTYPER bit
        otyper |= twr_gpio_16_bit_mask[channel];

        // Override desired mode setting to output
        mode = TWR_GPIO_MODE_OUTPUT;
    }
    else
    {
        // Reset corresponding OTYPER bit
        otyper &= ~twr_gpio_16_bit_mask[channel];
    }

    // If mode setting is alternative function ...
    if((mode & _TWR_GPIO_MODE_MASK) == TWR_GPIO_MODE_ALTERNATE)
    {
        // ... write AF number to appropriate pin of appropriate AFR register
        if(_twr_gpio_16_bit_pos[channel] >= 8)
        {
            twr_gpio_port[channel]->AFR[1] &= ~(0x0f << (_twr_gpio_64_bit_pos(channel) - 32));
            twr_gpio_port[channel]->AFR[1] |= ((uint8_t)mode >> _TWR_GPIO_MODE_AF_POS) << (_twr_gpio_64_bit_pos(channel) - 32);
        }
        else
        {
            twr_gpio_port[channel]->AFR[0] &= ~(0x0f << _twr_gpio_64_bit_pos(channel));
            twr_gpio_port[channel]->AFR[0] |= ((uint8_t)mode >> _TWR_GPIO_MODE_AF_POS) << _twr_gpio_64_bit_pos(channel);
        }

        // Mask AF number (mode is used as a coordinates in the array below ...)
        mode &= _TWR_GPIO_MODE_MASK;
    }

    // Reset corresponding MODER bits
    moder &= ~_twr_gpio_32_bit_mask[3][channel];

    // Set corresponding MODER bits
    moder |= _twr_gpio_32_bit_mask[mode][channel];

    // Write OTYPER register
    twr_gpio_port[channel]->OTYPER = otyper;

    // Write MODER register
    twr_gpio_port[channel]->MODER = moder;

    // Enable interrupts
    twr_irq_enable();
}

twr_gpio_mode_t twr_gpio_get_mode(twr_gpio_channel_t channel)
{
    // Read mode setting from MODER register
    twr_gpio_mode_t mode = (twr_gpio_mode_t) ((twr_gpio_port[channel]->MODER >> _twr_gpio_32_bit_pos[channel]) & 3);

    // If mode setting is output...
    if (mode == TWR_GPIO_MODE_OUTPUT)
    {
        // If TYPER register bit indicates open-drain output...
        if (((twr_gpio_port[channel]->OTYPER >> _twr_gpio_16_bit_pos[channel]) & 1) != 0)
        {
            // Override mode setting to open-drain
            mode = TWR_GPIO_MODE_OUTPUT_OD;
        }
    }

    // If mode setting is alternative function ...
    else if (mode == TWR_GPIO_MODE_ALTERNATE)
    {
        // Readout AF number from appropriate AFR
        if(_twr_gpio_16_bit_pos[channel] >= 8)
        {
            mode = (twr_gpio_port[channel]->AFR[1] >> (_twr_gpio_64_bit_pos(channel) - 32)) & 0x0f;
        }
        else
        {
            mode = (twr_gpio_port[channel]->AFR[0] >> _twr_gpio_64_bit_pos(channel)) & 0x0f;
        }

        // Insert number to enumeration
        mode = (mode << _TWR_GPIO_MODE_AF_POS) | TWR_GPIO_MODE_ALTERNATE;
    }

    // Return mode setting
    return mode;
}

int twr_gpio_get_input(twr_gpio_channel_t channel)
{
    // Return GPIO state from IDR register
    return (twr_gpio_port[channel]->IDR & twr_gpio_16_bit_mask[channel]) != 0 ? 1 : 0;
}

void twr_gpio_set_output(twr_gpio_channel_t channel, int state)
{
    // Write GPIO state to BSRR register
    twr_gpio_port[channel]->BSRR = state ? twr_gpio_16_bit_mask[channel] : twr_gpio_32_bit_upper_mask[channel];
}

int twr_gpio_get_output(twr_gpio_channel_t channel)
{
    // Return GPIO state from ODR register
    return (twr_gpio_port[channel]->ODR & twr_gpio_16_bit_mask[channel]) != 0 ? 1 : 0;
}

void twr_gpio_toggle_output(twr_gpio_channel_t channel)
{
    // Disable interrupts
    twr_irq_disable();

    // Write ODR register with inverted bit
    twr_gpio_port[channel]->ODR ^= twr_gpio_16_bit_mask[channel];

    // Enable interrupts
    twr_irq_enable();
}
