#include <twr_exti.h>
#include <twr_irq.h>
#include <stm32l0xx.h>

static bool _twr_exti_initialized = false;

static struct
{
    twr_exti_line_t line;
    void (*callback)(twr_exti_line_t, void *);
    void *param;

} _twr_exti[16];

static inline void _twr_exti_irq_handler(void);

void twr_exti_register(twr_exti_line_t line, twr_exti_edge_t edge, void (*callback)(twr_exti_line_t, void *), void *param)
{
    // Extract port number
    uint8_t port = ((uint8_t) line >> 4) & 7;

    // Extract pin number
    uint8_t pin = (uint8_t) line & 15;

    // Extract mask
    uint16_t mask = 1 << pin;

    // Disable interrupts
    twr_irq_disable();

    // Store line identifier
    _twr_exti[pin].line = line;

    // Store callback function
    _twr_exti[pin].callback = callback;

    // Store callback parameter
    _twr_exti[pin].param = param;

    // If this is the first call...
    if (!_twr_exti_initialized)
    {
        // Remember it has been already initialized
        _twr_exti_initialized = true;

        // Enable SYSCFG clock
        RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

        // Enable EXTI interrupt requests
        NVIC_EnableIRQ(EXTI0_1_IRQn);
        NVIC_EnableIRQ(EXTI2_3_IRQn);
        NVIC_EnableIRQ(EXTI4_15_IRQn);
    }

    // Configure port selection for given line
    SYSCFG->EXTICR[pin >> 2] |= port << ((pin & 3) << 2);

    if (edge == TWR_EXTI_EDGE_RISING)
    {
        // Enable rising edge trigger
        EXTI->RTSR |= mask;

        // Disable falling edge trigger
        EXTI->FTSR &= ~mask;
    }
    else if (edge == TWR_EXTI_EDGE_FALLING)
    {
        // Disable rising edge trigger
        EXTI->RTSR &= ~mask;

        // Enable falling edge trigger
        EXTI->FTSR |= mask;
    }
    else if (edge == TWR_EXTI_EDGE_RISING_AND_FALLING)
    {
        // Enable rising edge trigger
        EXTI->RTSR |= mask;

        // Enable falling edge trigger
        EXTI->FTSR |= mask;
    }

    // Unmask interrupt request
    EXTI->IMR |= mask;

    // Enable interrupts
    twr_irq_enable();
}

void twr_exti_unregister(twr_exti_line_t line)
{
    // Extract pin number
    uint8_t pin = (uint8_t) line & 15;

    // Extract mask
    uint16_t mask = 1 << pin;

    // Disable interrupts
    twr_irq_disable();

    // If line identifier matches record...
    if (line == _twr_exti[pin].line)
    {
        // Mask interrupt request
        EXTI->IMR &= ~mask;

        // Clear pending interrupt
        EXTI->PR = mask;
    }

    // Enable interrupts
    twr_irq_enable();
}

static inline void _twr_exti_irq_handler(void)
{
    // Determine source of interrupt and call appropriate callback
    if ((EXTI->PR & 0x0001) != 0) { EXTI->PR = 0x0001; _twr_exti[0].callback(_twr_exti[0].line, _twr_exti[0].param); return; }
    if ((EXTI->PR & 0x0002) != 0) { EXTI->PR = 0x0002; _twr_exti[1].callback(_twr_exti[1].line, _twr_exti[1].param); return; }
    if ((EXTI->PR & 0x0004) != 0) { EXTI->PR = 0x0004; _twr_exti[2].callback(_twr_exti[2].line, _twr_exti[2].param); return; }
    if ((EXTI->PR & 0x0008) != 0) { EXTI->PR = 0x0008; _twr_exti[3].callback(_twr_exti[3].line, _twr_exti[3].param); return; }
    if ((EXTI->PR & 0x0010) != 0) { EXTI->PR = 0x0010; _twr_exti[4].callback(_twr_exti[4].line, _twr_exti[4].param); return; }
    if ((EXTI->PR & 0x0020) != 0) { EXTI->PR = 0x0020; _twr_exti[5].callback(_twr_exti[5].line, _twr_exti[5].param); return; }
    if ((EXTI->PR & 0x0040) != 0) { EXTI->PR = 0x0040; _twr_exti[6].callback(_twr_exti[6].line, _twr_exti[6].param); return; }
    if ((EXTI->PR & 0x0080) != 0) { EXTI->PR = 0x0080; _twr_exti[7].callback(_twr_exti[7].line, _twr_exti[7].param); return; }
    if ((EXTI->PR & 0x0100) != 0) { EXTI->PR = 0x0100; _twr_exti[8].callback(_twr_exti[8].line, _twr_exti[8].param); return; }
    if ((EXTI->PR & 0x0200) != 0) { EXTI->PR = 0x0200; _twr_exti[9].callback(_twr_exti[9].line, _twr_exti[9].param); return; }
    if ((EXTI->PR & 0x0400) != 0) { EXTI->PR = 0x0400; _twr_exti[10].callback(_twr_exti[10].line, _twr_exti[10].param); return; }
    if ((EXTI->PR & 0x0800) != 0) { EXTI->PR = 0x0800; _twr_exti[11].callback(_twr_exti[11].line, _twr_exti[11].param); return; }
    if ((EXTI->PR & 0x1000) != 0) { EXTI->PR = 0x1000; _twr_exti[12].callback(_twr_exti[12].line, _twr_exti[12].param); return; }
    if ((EXTI->PR & 0x2000) != 0) { EXTI->PR = 0x2000; _twr_exti[13].callback(_twr_exti[13].line, _twr_exti[13].param); return; }
    if ((EXTI->PR & 0x4000) != 0) { EXTI->PR = 0x4000; _twr_exti[14].callback(_twr_exti[14].line, _twr_exti[14].param); return; }
    if ((EXTI->PR & 0x8000) != 0) { EXTI->PR = 0x8000; _twr_exti[15].callback(_twr_exti[15].line, _twr_exti[15].param); return; }
}

void EXTI0_1_IRQHandler(void)
{
    _twr_exti_irq_handler();
}

void EXTI2_3_IRQHandler(void)
{
    _twr_exti_irq_handler();
}

void EXTI4_15_IRQHandler(void)
{
    _twr_exti_irq_handler();
}
