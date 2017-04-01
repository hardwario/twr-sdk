#include <bc_pyq1648.h>
#include <bc_irq.h>
#include <bc_gpio.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

#define BC_PYQ1648_BPF 0x00
#define BC_PYQ1648_LPF 0x01
#define BC_PYQ1648_WAKE_UP_MODE 0x02

#define BC_PYQ1648_DELAY_RUN 50
#define BC_PYQ1648_DELAY_INITIALIZATION 10
#define BC_PYQ1648_UPDATE_INTERVAL 100

static inline void _bc_pyq1648_msp_init(bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl);
static inline void _bc_pyq1648_dev_init(bc_pyq1648_t *self);
static inline void _bc_pyq1648_compose_event_unit_config(bc_pyq1648_t *self);
static inline void _bc_pyq1648_delay_100us();
static void _bc_pyq1648_task(void *param);

static const uint8_t _bc_pyq1648_sensitivity_table[4] =
{
    [BC_PYQ1648_SENSITIVITY_LOW] = 150,
    [BC_PYQ1648_SENSITIVITY_MEDIUM] = 70,
    [BC_PYQ1648_SENSITIVITY_HIGH] = 20,
    [BC_PYQ1648_SENSITIVITY_VERY_HIGH] = 9 };

extern GPIO_TypeDef * bc_gpio_port[];
extern uint16_t bc_gpio_16_bit_mask[];
extern uint32_t bc_gpio_32_bit_upper_mask[];

static GPIO_TypeDef **_pyq1648_gpiox_table = bc_gpio_port;
static uint16_t *_pyq1648_set_mask = bc_gpio_16_bit_mask;
static uint32_t *_pyq1648_reset_mask = bc_gpio_32_bit_upper_mask;

void bc_pyq1648_init(bc_pyq1648_t *self, bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl)
{
    // Initialize structure
    memset(self, 0, sizeof(*self));

    // Initialize default values
    self->_sensitivity = _bc_pyq1648_sensitivity_table[BC_PYQ1648_SENSITIVITY_HIGH];
    self->_blank_period = 1000;

    // Initialize actually used pins
    self->_gpio_channel_serin = gpio_channel_serin;
    self->_gpio_channel_dl = gpio_channel_dl;

    // Initialize event_unit_configuration register value
    // _bc_pyq1648_compose_event_unit_config(self);

    // Register task
    self->_task_id = bc_scheduler_register(_bc_pyq1648_task, self, BC_PYQ1648_DELAY_RUN);

    // Enable clock for TIM6
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    // Enable one-pulse mode
    TIM6->CR1 |= TIM_CR1_OPM;
}

void bc_pyq1648_set_event_handler(bc_pyq1648_t *self, void (*event_handler)(bc_pyq1648_t *, bc_pyq1648_event_t, void *), void *event_param)
{
    // Set event handler
    self->_event_handler = event_handler;

    // Set event param
    self->_event_param = event_param;
}

void bc_pyq1648_set_sensitivity(bc_pyq1648_t *self, bc_pyq1648_sensitivity_t sensitivity)
{
    // Set sensitivity to desired value
    self->_sensitivity = _bc_pyq1648_sensitivity_table[sensitivity];

    self->_state = BC_PYQ1648_STATE_INITIALIZE;

    bc_scheduler_plan_now(self->_task_id);
}

void bc_pyq1648_set_blank_period(bc_pyq1648_t *self, bc_tick_t blank_period)
{
    // Set blank period
    self->_blank_period = blank_period;
}

void _bc_pyq1648_compose_event_unit_config(bc_pyq1648_t *self)
{
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |                                                         Event unit configuration                                                     |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // | 7bit sensitivity | 4bit blind time | 2bit pulse counter | 2bit window time | 2bit operatin mode | 2bit filter source | 5bit reserved |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |     from self    |  handled by SW  |        0x00        |       0x00       |    wake up mode    |  Band pass filter  | has to be 16  |
    //  --------------------------------------------------------------------------------------------------------------------------------------

    self->_config = 0x00000000;
    self->_config |= (self->_sensitivity << 17) | (BC_PYQ1648_WAKE_UP_MODE << 7) | (BC_PYQ1648_BPF << 5) |0x10;
}

static inline void _bc_pyq1648_msp_init(bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl)
{
    // Initialize SerialIn (SERIN) GPIO pin
    bc_gpio_init(gpio_channel_serin);
    bc_gpio_set_mode(gpio_channel_serin, BC_GPIO_MODE_OUTPUT);

    // Initialize DirectLink (DL) GPIO pin
    bc_gpio_init(gpio_channel_dl);
    bc_gpio_set_mode(gpio_channel_dl, BC_GPIO_MODE_INPUT);
    bc_gpio_set_pull(gpio_channel_dl, BC_GPIO_PULL_DOWN);
}

static void _bc_pyq1648_clear_event(bc_pyq1648_t *self)
{
    // Clear event by pull down DL
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(self->_gpio_channel_dl, false);
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);
}

static inline void _bc_pyq1648_dev_init(bc_pyq1648_t *self)
{
    // Disable interrupts
    bc_irq_disable();

    // Enable PLL
    bc_module_core_pll_enable();

    // Load desired event unit configuration
    uint32_t regval = self->_config;
    uint32_t regmask = 0x1000000;
    bool next_bit;

    // Prepare fast GPIO access
    uint32_t bsrr_mask[2] =
    {
        [0] = _pyq1648_reset_mask[self->_gpio_channel_serin],
        [1] = _pyq1648_set_mask[self->_gpio_channel_serin]
    };
    GPIO_TypeDef *GPIOx = _pyq1648_gpiox_table[self->_gpio_channel_serin];
    volatile uint32_t *GPIOx_BSRR = &GPIOx->BSRR;

    // Low level pin initialization
    _bc_pyq1648_msp_init(self->_gpio_channel_serin, self->_gpio_channel_dl);

    // Transmit event unit configuration
    for (int i = 0; i < 25; i++)
    {
        next_bit = (regval & regmask) != 0 ? true : false;
        regmask >>= 1;

        *GPIOx_BSRR = bsrr_mask[0];
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        *GPIOx_BSRR = bsrr_mask[1];
        __asm("nop");
        __asm("nop");
        __asm("nop");
        *GPIOx_BSRR = bsrr_mask[next_bit];

        _bc_pyq1648_delay_100us(1);
    }

    bc_gpio_set_output(self->_gpio_channel_serin, false);

    // Disable PLL
    bc_module_core_pll_disable();

    // Enable interrupts
    bc_irq_enable();
}

static inline void _bc_pyq1648_delay_100us()
{
    // Set prescaler
    TIM6->PSC = 0;

    // Set auto-reload register - period 100 us
    TIM6->ARR = 3000;

    // Generate update of registers
    TIM6->EGR = TIM_EGR_UG;

    // Enable counter
    TIM6->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM6->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }
}

static void _bc_pyq1648_task(void *param)
{
    bc_pyq1648_t *self = param;

start:

    switch (self->_state)
    {
        case BC_PYQ1648_STATE_ERROR:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_PYQ1648_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_PYQ1648_STATE_INITIALIZE;

            bc_scheduler_plan_current_relative(BC_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        case BC_PYQ1648_STATE_INITIALIZE:
        {
            self->_state = BC_PYQ1648_STATE_ERROR;

            _bc_pyq1648_compose_event_unit_config(self);

            _bc_pyq1648_dev_init(self);

            self->_state = BC_PYQ1648_STATE_IGNORE;

            bc_scheduler_plan_current_relative(BC_PYQ1648_DELAY_INITIALIZATION);

            return;
        }
        case BC_PYQ1648_STATE_IGNORE:
        {
            self->_state = BC_PYQ1648_STATE_ERROR;

            if (self->_ignore_untill == 0)
            {
                // TODO ... acquire !!!
                self->_ignore_untill = bc_tick_get() + (75000 / 15);
            }

            if (bc_gpio_get_input(self->_gpio_channel_dl))
            {
                _bc_pyq1648_clear_event(self);
            }

            if (bc_tick_get() >= self->_ignore_untill)
            {
                self->_state = BC_PYQ1648_STATE_CHECK;
            }
            else
            {
                self->_state = BC_PYQ1648_STATE_IGNORE;
            }

            bc_scheduler_plan_current_relative(BC_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        case BC_PYQ1648_STATE_CHECK:
        {
            bc_tick_t tick_now = bc_tick_get();

            self->_state = BC_PYQ1648_STATE_ERROR;

            if (bc_gpio_get_input(self->_gpio_channel_dl))
            {
                if (tick_now >= self->_aware_time)
                {
                    self->_event_handler(self, BC_PYQ1648_EVENT_MOTION, self->_event_param);
                    self->_aware_time = tick_now + self->_blank_period;
                }
                _bc_pyq1648_clear_event(self);
            }

            self->_state = BC_PYQ1648_STATE_CHECK;

            bc_scheduler_plan_current_relative(BC_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        default:
        {
            self->_state = BC_PYQ1648_STATE_ERROR;

            goto start;
        }
    }
}
