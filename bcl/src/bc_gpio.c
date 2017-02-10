#include <bc_gpio.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

#define BC_GPIO_CHANNEL_COUNT 20

#define BC_GPIO_PORT_P0 GPIOA
#define BC_GPIO_PORT_P1 GPIOA
#define BC_GPIO_PORT_P2 GPIOA
#define BC_GPIO_PORT_P3 GPIOA
#define BC_GPIO_PORT_P4 GPIOA
#define BC_GPIO_PORT_P5 GPIOA
#define BC_GPIO_PORT_P6 GPIOB
#define BC_GPIO_PORT_P7 GPIOA
#define BC_GPIO_PORT_P8 GPIOB
#define BC_GPIO_PORT_P9 GPIOB
#define BC_GPIO_PORT_P10 GPIOA
#define BC_GPIO_PORT_P11 GPIOA
#define BC_GPIO_PORT_P12 GPIOB
#define BC_GPIO_PORT_P13 GPIOB
#define BC_GPIO_PORT_P14 GPIOB
#define BC_GPIO_PORT_P15 GPIOB
#define BC_GPIO_PORT_P16 GPIOB
#define BC_GPIO_PORT_P17 GPIOB
#define BC_GPIO_PORT_LED GPIOH
#define BC_GPIO_PORT_BUTTON GPIOA

#define BC_GPIO_POS_P0 0UL
#define BC_GPIO_POS_P1 1UL
#define BC_GPIO_POS_P2 2UL
#define BC_GPIO_POS_P3 3UL
#define BC_GPIO_POS_P4 4UL
#define BC_GPIO_POS_P5 5UL
#define BC_GPIO_POS_P6 1UL
#define BC_GPIO_POS_P7 6UL
#define BC_GPIO_POS_P8 0UL
#define BC_GPIO_POS_P9 2UL
#define BC_GPIO_POS_P10 10UL
#define BC_GPIO_POS_P11 9UL
#define BC_GPIO_POS_P12 14UL
#define BC_GPIO_POS_P13 15UL
#define BC_GPIO_POS_P14 13UL
#define BC_GPIO_POS_P15 12UL
#define BC_GPIO_POS_P16 8UL
#define BC_GPIO_POS_P17 9UL
#define BC_GPIO_POS_LED 1UL
#define BC_GPIO_POS_BUTTON 8UL

GPIO_TypeDef * const bc_gpio_port[BC_GPIO_CHANNEL_COUNT] =
{
    BC_GPIO_PORT_P0,
    BC_GPIO_PORT_P1,
    BC_GPIO_PORT_P2,
    BC_GPIO_PORT_P3,
    BC_GPIO_PORT_P4,
    BC_GPIO_PORT_P5,
    BC_GPIO_PORT_P6,
    BC_GPIO_PORT_P7,
    BC_GPIO_PORT_P8,
    BC_GPIO_PORT_P9,
    BC_GPIO_PORT_P10,
    BC_GPIO_PORT_P11,
    BC_GPIO_PORT_P12,
    BC_GPIO_PORT_P13,
    BC_GPIO_PORT_P14,
    BC_GPIO_PORT_P15,
    BC_GPIO_PORT_P16,
    BC_GPIO_PORT_P17,
    BC_GPIO_PORT_LED,
    BC_GPIO_PORT_BUTTON
};

static const uint8_t bc_gpio_iopenr_mask[BC_GPIO_CHANNEL_COUNT] =
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
    RCC_IOPENR_GPIOAEN  // BUTTON
};

static const uint16_t bc_gpio_16_bit_pos[BC_GPIO_CHANNEL_COUNT] =
{
    BC_GPIO_POS_P0,
    BC_GPIO_POS_P1,
    BC_GPIO_POS_P2,
    BC_GPIO_POS_P3,
    BC_GPIO_POS_P4,
    BC_GPIO_POS_P5,
    BC_GPIO_POS_P6,
    BC_GPIO_POS_P7,
    BC_GPIO_POS_P8,
    BC_GPIO_POS_P9,
    BC_GPIO_POS_P10,
    BC_GPIO_POS_P11,
    BC_GPIO_POS_P12,
    BC_GPIO_POS_P13,
    BC_GPIO_POS_P14,
    BC_GPIO_POS_P15,
    BC_GPIO_POS_P16,
    BC_GPIO_POS_P17,
    BC_GPIO_POS_LED,
    BC_GPIO_POS_BUTTON
};

static const uint16_t bc_gpio_32_bit_pos[BC_GPIO_CHANNEL_COUNT] =
{
    2UL * BC_GPIO_POS_P0,
    2UL * BC_GPIO_POS_P1,
    2UL * BC_GPIO_POS_P2,
    2UL * BC_GPIO_POS_P3,
    2UL * BC_GPIO_POS_P4,
    2UL * BC_GPIO_POS_P5,
    2UL * BC_GPIO_POS_P6,
    2UL * BC_GPIO_POS_P7,
    2UL * BC_GPIO_POS_P8,
    2UL * BC_GPIO_POS_P9,
    2UL * BC_GPIO_POS_P10,
    2UL * BC_GPIO_POS_P11,
    2UL * BC_GPIO_POS_P12,
    2UL * BC_GPIO_POS_P13,
    2UL * BC_GPIO_POS_P14,
    2UL * BC_GPIO_POS_P15,
    2UL * BC_GPIO_POS_P16,
    2UL * BC_GPIO_POS_P17,
    2UL * BC_GPIO_POS_LED,
    2UL * BC_GPIO_POS_BUTTON
};

const uint16_t bc_gpio_16_bit_mask[BC_GPIO_CHANNEL_COUNT] =
{
    1UL << BC_GPIO_POS_P0,
    1UL << BC_GPIO_POS_P1,
    1UL << BC_GPIO_POS_P2,
    1UL << BC_GPIO_POS_P3,
    1UL << BC_GPIO_POS_P4,
    1UL << BC_GPIO_POS_P5,
    1UL << BC_GPIO_POS_P6,
    1UL << BC_GPIO_POS_P7,
    1UL << BC_GPIO_POS_P8,
    1UL << BC_GPIO_POS_P9,
    1UL << BC_GPIO_POS_P10,
    1UL << BC_GPIO_POS_P11,
    1UL << BC_GPIO_POS_P12,
    1UL << BC_GPIO_POS_P13,
    1UL << BC_GPIO_POS_P14,
    1UL << BC_GPIO_POS_P15,
    1UL << BC_GPIO_POS_P16,
    1UL << BC_GPIO_POS_P17,
    1UL << BC_GPIO_POS_LED,
    1UL << BC_GPIO_POS_BUTTON
};

static const uint32_t bc_gpio_32_bit_mask[4][BC_GPIO_CHANNEL_COUNT] =
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
        0UL
    },
    {
        1UL << (2UL * BC_GPIO_POS_P0),
        1UL << (2UL * BC_GPIO_POS_P1),
        1UL << (2UL * BC_GPIO_POS_P2),
        1UL << (2UL * BC_GPIO_POS_P3),
        1UL << (2UL * BC_GPIO_POS_P4),
        1UL << (2UL * BC_GPIO_POS_P5),
        1UL << (2UL * BC_GPIO_POS_P6),
        1UL << (2UL * BC_GPIO_POS_P7),
        1UL << (2UL * BC_GPIO_POS_P8),
        1UL << (2UL * BC_GPIO_POS_P9),
        1UL << (2UL * BC_GPIO_POS_P10),
        1UL << (2UL * BC_GPIO_POS_P11),
        1UL << (2UL * BC_GPIO_POS_P12),
        1UL << (2UL * BC_GPIO_POS_P13),
        1UL << (2UL * BC_GPIO_POS_P14),
        1UL << (2UL * BC_GPIO_POS_P15),
        1UL << (2UL * BC_GPIO_POS_P16),
        1UL << (2UL * BC_GPIO_POS_P17),
        1UL << (2UL * BC_GPIO_POS_LED),
        1UL << (2UL * BC_GPIO_POS_BUTTON)
    },
    {
        2UL << (2UL * BC_GPIO_POS_P0),
        2UL << (2UL * BC_GPIO_POS_P1),
        2UL << (2UL * BC_GPIO_POS_P2),
        2UL << (2UL * BC_GPIO_POS_P3),
        2UL << (2UL * BC_GPIO_POS_P4),
        2UL << (2UL * BC_GPIO_POS_P5),
        2UL << (2UL * BC_GPIO_POS_P6),
        2UL << (2UL * BC_GPIO_POS_P7),
        2UL << (2UL * BC_GPIO_POS_P8),
        2UL << (2UL * BC_GPIO_POS_P9),
        2UL << (2UL * BC_GPIO_POS_P10),
        2UL << (2UL * BC_GPIO_POS_P11),
        2UL << (2UL * BC_GPIO_POS_P12),
        2UL << (2UL * BC_GPIO_POS_P13),
        2UL << (2UL * BC_GPIO_POS_P14),
        2UL << (2UL * BC_GPIO_POS_P15),
        2UL << (2UL * BC_GPIO_POS_P16),
        2UL << (2UL * BC_GPIO_POS_P17),
        2UL << (2UL * BC_GPIO_POS_LED),
        2UL << (2UL * BC_GPIO_POS_BUTTON)
    },
    {
        3UL << (2UL * BC_GPIO_POS_P0),
        3UL << (2UL * BC_GPIO_POS_P1),
        3UL << (2UL * BC_GPIO_POS_P2),
        3UL << (2UL * BC_GPIO_POS_P3),
        3UL << (2UL * BC_GPIO_POS_P4),
        3UL << (2UL * BC_GPIO_POS_P5),
        3UL << (2UL * BC_GPIO_POS_P6),
        3UL << (2UL * BC_GPIO_POS_P7),
        3UL << (2UL * BC_GPIO_POS_P8),
        3UL << (2UL * BC_GPIO_POS_P9),
        3UL << (2UL * BC_GPIO_POS_P10),
        3UL << (2UL * BC_GPIO_POS_P11),
        3UL << (2UL * BC_GPIO_POS_P12),
        3UL << (2UL * BC_GPIO_POS_P13),
        3UL << (2UL * BC_GPIO_POS_P14),
        3UL << (2UL * BC_GPIO_POS_P15),
        3UL << (2UL * BC_GPIO_POS_P16),
        3UL << (2UL * BC_GPIO_POS_P17),
        3UL << (2UL * BC_GPIO_POS_LED),
        3UL << (2UL * BC_GPIO_POS_BUTTON)
    }
};

const uint32_t bc_gpio_32_bit_upper_mask[BC_GPIO_CHANNEL_COUNT] =
{
    (1UL << 16UL) << BC_GPIO_POS_P0,
    (1UL << 16UL) << BC_GPIO_POS_P1,
    (1UL << 16UL) << BC_GPIO_POS_P2,
    (1UL << 16UL) << BC_GPIO_POS_P3,
    (1UL << 16UL) << BC_GPIO_POS_P4,
    (1UL << 16UL) << BC_GPIO_POS_P5,
    (1UL << 16UL) << BC_GPIO_POS_P6,
    (1UL << 16UL) << BC_GPIO_POS_P7,
    (1UL << 16UL) << BC_GPIO_POS_P8,
    (1UL << 16UL) << BC_GPIO_POS_P9,
    (1UL << 16UL) << BC_GPIO_POS_P10,
    (1UL << 16UL) << BC_GPIO_POS_P11,
    (1UL << 16UL) << BC_GPIO_POS_P12,
    (1UL << 16UL) << BC_GPIO_POS_P13,
    (1UL << 16UL) << BC_GPIO_POS_P14,
    (1UL << 16UL) << BC_GPIO_POS_P15,
    (1UL << 16UL) << BC_GPIO_POS_P16,
    (1UL << 16UL) << BC_GPIO_POS_P17,
    (1UL << 16UL) << BC_GPIO_POS_LED,
    (1UL << 16UL) << BC_GPIO_POS_BUTTON
};

void bc_gpio_init(bc_gpio_channel_t channel)
{
    // Disable interrupts
    bc_irq_disable();

    // Enable GPIO clock
    RCC->IOPENR |= bc_gpio_iopenr_mask[channel];

    // Enable interrupts
    bc_irq_enable();
}

void bc_gpio_set_pull(bc_gpio_channel_t channel, bc_gpio_pull_t pull)
{
    // Disable interrupts
    bc_irq_disable();

    // Read PUPDR register
    uint32_t pupdr = bc_gpio_port[channel]->PUPDR;

    // Reset corresponding PUPDR bits
    pupdr &= ~bc_gpio_32_bit_mask[3UL][channel];

    // Set corresponding PUPDR bits
    pupdr |= bc_gpio_32_bit_mask[pull][channel];

    // Write PUPDR register
    bc_gpio_port[channel]->PUPDR = pupdr;

    // Enable interrupts
    bc_irq_enable();
}

bc_gpio_pull_t bc_gpio_get_pull(bc_gpio_channel_t channel)
{
    // Return pull setting from PUPDR register
    return (bc_gpio_pull_t) ((bc_gpio_port[channel]->PUPDR >> bc_gpio_32_bit_pos[channel]) & 3UL);
}

void bc_gpio_set_mode(bc_gpio_channel_t channel, bc_gpio_mode_t mode)
{
    // Disable interrupts
    bc_irq_disable();

    // Read OTYPER register
    uint32_t otyper = bc_gpio_port[channel]->OTYPER;

    // Read MODER register
    uint32_t moder = bc_gpio_port[channel]->MODER;

    // If mode setting is open-drain output...
    if (mode == BC_GPIO_MODE_OUTPUT_OD)
    {
        // Set corresponding OTYPER bit
        otyper |= bc_gpio_16_bit_mask[channel];

        // Override desired mode setting to output
        mode = BC_GPIO_MODE_OUTPUT;
    }
    else
    {
        // Reset corresponding OTYPER bit
        otyper &= ~bc_gpio_16_bit_mask[channel];
    }

    // Reset corresponding MODER bits
    moder &= ~bc_gpio_32_bit_mask[3UL][channel];

    // Set corresponding MODER bits
    moder |= bc_gpio_32_bit_mask[mode][channel];

    // Write OTYPER register
    bc_gpio_port[channel]->OTYPER = otyper;

    // Write MODER register
    bc_gpio_port[channel]->MODER = moder;

    // Enable interrupts
    bc_irq_enable();
}

bc_gpio_mode_t bc_gpio_get_mode(bc_gpio_channel_t channel)
{
    // Read mode setting from MODER register
    bc_gpio_mode_t mode = (bc_gpio_mode_t) ((bc_gpio_port[channel]->MODER >> bc_gpio_32_bit_pos[channel]) & 3UL);

    // If mode setting is output...
    if (mode == BC_GPIO_MODE_OUTPUT)
    {
        // If TYPER register bit indicates open-drain output...
        if (((bc_gpio_port[channel]->OTYPER >> bc_gpio_16_bit_pos[channel]) & 1UL) != 0UL)
        {
            // Override mode setting to open-drain
            mode = BC_GPIO_MODE_OUTPUT_OD;
        }
    }

    // Return mode setting
    return mode;
}

bool bc_gpio_get_input(bc_gpio_channel_t channel)
{
    // Return GPIO state from IDR register
    return (bc_gpio_port[channel]->IDR & bc_gpio_16_bit_mask[channel]) != 0UL ? true : false;
}

void bc_gpio_set_output(bc_gpio_channel_t channel, bool state)
{
    // Write GPIO state to BSRR register
    bc_gpio_port[channel]->BSRR = state ? bc_gpio_16_bit_mask[channel] : bc_gpio_32_bit_upper_mask[channel];
}

bool bc_gpio_get_output(bc_gpio_channel_t channel)
{
    // Return GPIO state from ODR register
    return (bc_gpio_port[channel]->ODR & bc_gpio_16_bit_mask[channel]) != 0UL ? true : false;
}

void bc_gpio_toggle_output(bc_gpio_channel_t channel)
{
    // Disable interrupts
    bc_irq_disable();

    // Write ODR register with inverted bit
    bc_gpio_port[channel]->ODR ^= bc_gpio_16_bit_mask[channel];

    // Enable interrupts
    bc_irq_enable();
}
