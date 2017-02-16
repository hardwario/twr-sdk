#include <bc_exti.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

static bool _bc_exti_initialized = false;

static struct
{
    bc_exti_line_t line;
    void (*callback)(bc_exti_line_t, void *);
    void *param;

} _bc_exti[16];

static inline void _bc_exti_irq_handler(void);

void bc_exti_register(bc_exti_line_t line, bc_exti_edge_t edge, void (*callback)(bc_exti_line_t, void *), void *param)
{
    // Extract port number
    uint8_t port = ((uint8_t) line >> 4) & 7;

    // Extract pin number
    uint8_t pin = (uint8_t) line & 15;

    // Extract mask
    uint16_t mask = 1 << pin;

    // Disable interrupts
    bc_irq_disable();

    // Store line identifier
    _bc_exti[pin].line = line;

    // Store callback function
    _bc_exti[pin].callback = callback;

    // Store callback parameter
    _bc_exti[pin].param = param;

    // If this is the first call...
    if (!_bc_exti_initialized)
    {
        // Remember it has been already initialized
        _bc_exti_initialized = true;

        // Enable SYSCFG clock
        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

        // Enable EXTI interrupt requests
        NVIC_EnableIRQ(EXTI0_1_IRQn);
        NVIC_EnableIRQ(EXTI2_3_IRQn);
        NVIC_EnableIRQ(EXTI4_15_IRQn);
    }

    // Configure port selection for given line
    SYSCFG->EXTICR[pin >> 2] |= port << ((pin & 3) << 2);

    if (edge == BC_EXTI_EDGE_RISING)
    {
        // Enable rising edge trigger
        EXTI->RTSR |= mask;

        // Disable falling edge trigger
        EXTI->FTSR &= ~mask;
    }
    else if (edge == BC_EXTI_EDGE_FALLING)
    {
        // Disable rising edge trigger
        EXTI->RTSR &= ~mask;

        // Enable falling edge trigger
        EXTI->FTSR |= mask;
    }
    else if (edge == BC_EXTI_EDGE_RISING_AND_FALLING)
    {
        // Enable rising edge trigger
        EXTI->RTSR |= mask;

        // Enable falling edge trigger
        EXTI->FTSR |= mask;
    }

    // Unmask interrupt request
    EXTI->IMR |= mask;

    // Enable interrupts
    bc_irq_enable();
}

void bc_exti_unregister(bc_exti_line_t line)
{
    // Extract pin number
    uint8_t pin = (uint8_t) line & 15;

    // Extract mask
    uint16_t mask = 1 << pin;

    // Disable interrupts
    bc_irq_disable();

    // If line identifier matches record...
    if (line == _bc_exti[pin].line)
    {
        // Mask interrupt request
        EXTI->IMR &= ~mask;

        // Clear pending interrupt
        EXTI->PR = mask;
    }

    // Enable interrupts
    bc_irq_enable();
}

static inline void _bc_exti_irq_handler(void)
{
    // Determine source of interrupt and call appropriate callback
    if ((EXTI->PR & 0x0001) != 0) { EXTI->PR = 0x0001; _bc_exti[0].callback(_bc_exti[0].line, _bc_exti[0].param); return; }
    if ((EXTI->PR & 0x0002) != 0) { EXTI->PR = 0x0002; _bc_exti[1].callback(_bc_exti[1].line, _bc_exti[1].param); return; }
    if ((EXTI->PR & 0x0004) != 0) { EXTI->PR = 0x0004; _bc_exti[2].callback(_bc_exti[2].line, _bc_exti[2].param); return; }
    if ((EXTI->PR & 0x0008) != 0) { EXTI->PR = 0x0008; _bc_exti[3].callback(_bc_exti[3].line, _bc_exti[3].param); return; }
    if ((EXTI->PR & 0x0010) != 0) { EXTI->PR = 0x0010; _bc_exti[4].callback(_bc_exti[4].line, _bc_exti[4].param); return; }
    if ((EXTI->PR & 0x0020) != 0) { EXTI->PR = 0x0020; _bc_exti[5].callback(_bc_exti[5].line, _bc_exti[5].param); return; }
    if ((EXTI->PR & 0x0040) != 0) { EXTI->PR = 0x0040; _bc_exti[6].callback(_bc_exti[6].line, _bc_exti[6].param); return; }
    if ((EXTI->PR & 0x0080) != 0) { EXTI->PR = 0x0080; _bc_exti[7].callback(_bc_exti[7].line, _bc_exti[7].param); return; }
    if ((EXTI->PR & 0x0100) != 0) { EXTI->PR = 0x0100; _bc_exti[8].callback(_bc_exti[8].line, _bc_exti[8].param); return; }
    if ((EXTI->PR & 0x0200) != 0) { EXTI->PR = 0x0200; _bc_exti[9].callback(_bc_exti[9].line, _bc_exti[9].param); return; }
    if ((EXTI->PR & 0x0400) != 0) { EXTI->PR = 0x0400; _bc_exti[10].callback(_bc_exti[10].line, _bc_exti[10].param); return; }
    if ((EXTI->PR & 0x0800) != 0) { EXTI->PR = 0x0800; _bc_exti[11].callback(_bc_exti[11].line, _bc_exti[11].param); return; }
    if ((EXTI->PR & 0x1000) != 0) { EXTI->PR = 0x1000; _bc_exti[12].callback(_bc_exti[12].line, _bc_exti[12].param); return; }
    if ((EXTI->PR & 0x2000) != 0) { EXTI->PR = 0x2000; _bc_exti[13].callback(_bc_exti[13].line, _bc_exti[13].param); return; }
    if ((EXTI->PR & 0x4000) != 0) { EXTI->PR = 0x4000; _bc_exti[14].callback(_bc_exti[14].line, _bc_exti[14].param); return; }
    if ((EXTI->PR & 0x8000) != 0) { EXTI->PR = 0x8000; _bc_exti[15].callback(_bc_exti[15].line, _bc_exti[15].param); return; }
}

void EXTI0_1_IRQHandler(void)
{
    _bc_exti_irq_handler();
}

void EXTI2_3_IRQHandler(void)
{
    _bc_exti_irq_handler();
}

void EXTI4_15_IRQHandler(void)
{
    _bc_exti_irq_handler();
}
