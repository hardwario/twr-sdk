#include <bc_pyq1648.h>
#include <bc_irq.h>
#include <bc_gpio.h>
#include <bc_system.h>
#include <stm32l0xx.h>

#define BC_PYQ1648_BPF 0x00
#define BC_PYQ1648_LPF 0x01
#define BC_PYQ1648_WAKE_UP_MODE 0x02

#define BC_PYQ1648_DELAY_RUN 50
#define BC_PYQ1648_DELAY_PREINIT 10
#define BC_PYQ1648_DELAY_INITIALIZATION 10
#define BC_PYQ1648_UPDATE_INTERVAL 100

#define BC_PYQ1648_CONFIG_BIT_COUNT 24

static void _bc_pyq1648_task(void *param);

static void _bc_pyq1648_clear_event(bc_pyq1648_t *self);
static void _bc_pyq1648_compose_bit_buffer(bc_pyq1648_t *self, uint16_t *bit_buffer);
static void _bc_pyq1648_preinit(bc_pyq1648_t *self);
static void _bc_pyq1648_init(bc_pyq1648_t *self);

static const uint8_t _bc_pyq1648_sensitivity_table[4] =
{
    [BC_PYQ1648_SENSITIVITY_LOW] = 150,
    [BC_PYQ1648_SENSITIVITY_MEDIUM] = 70,
    [BC_PYQ1648_SENSITIVITY_HIGH] = 20,
    [BC_PYQ1648_SENSITIVITY_VERY_HIGH] = 9
};

static const uint16_t _bc_pyq1648_pulse_count[2] =
{
    [0] = 32,  // < 1us>
    [1] = 3168 // <99us>
};

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

    // Register task
    self->_task_id = bc_scheduler_register(_bc_pyq1648_task, self, BC_PYQ1648_DELAY_RUN);
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

    // Re-initialize
    self->_state = BC_PYQ1648_STATE_INIT;
    bc_scheduler_plan_now(self->_task_id);
}

void bc_pyq1648_set_blank_period(bc_pyq1648_t *self, bc_tick_t blank_period)
{
    // Set blank period
    self->_blank_period = blank_period;
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

            self->_state = BC_PYQ1648_STATE_PREINIT;

            bc_scheduler_plan_current_relative(BC_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        case BC_PYQ1648_STATE_PREINIT:
        {
            _bc_pyq1648_preinit(self);

            self->_state = BC_PYQ1648_STATE_INIT;

            bc_scheduler_plan_current_relative(BC_PYQ1648_DELAY_PREINIT);

            return;
        }
        case BC_PYQ1648_STATE_INIT:
        {
            _bc_pyq1648_init(self);

            self->_state = BC_PYQ1648_STATE_IGNORE;

            goto start;
        }
        case BC_PYQ1648_STATE_IGNORE:
        {
            if (self->_ignore_untill == 0)
            {
                // TODO ... acquire !!!
                self->_ignore_untill = bc_tick_get() + (75000 / 15);
            }

            if (bc_gpio_get_input(self->_gpio_channel_dl) != 0)
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

            if (bc_gpio_get_input(self->_gpio_channel_dl) != 0)
            {
                if (tick_now >= self->_aware_time)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_PYQ1648_EVENT_MOTION, self->_event_param);
                    }

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

static void _bc_pyq1648_clear_event(bc_pyq1648_t *self)
{
    // Clear event by pull down DL
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(self->_gpio_channel_dl, 0);
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);
}

static void _bc_pyq1648_compose_bit_buffer(bc_pyq1648_t *self, uint16_t *bit_buffer)
{
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |                                                         Event unit configuration                                                     |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // | 7bit sensitivity | 4bit blind time | 2bit pulse counter | 2bit window time | 2bit operatin mode | 2bit filter source | 5bit reserved |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |     From self    |  Handled by SW  |        0x00        |       0x00       |    Wake up mode    |  Band pass filter  | Has to be 16  |
    //  --------------------------------------------------------------------------------------------------------------------------------------

    uint32_t config = (self->_sensitivity << 17) | (BC_PYQ1648_WAKE_UP_MODE << 7) | (BC_PYQ1648_BPF << 5) | 0x10;

    // Mask every single valid bit in config by mask (top to bottom)
    for (uint32_t mask = 1 << (BC_PYQ1648_CONFIG_BIT_COUNT - 1); mask != 0; mask >>= 1)
    {
        // ...If logical 1...
        if ((config & mask) != 0)
        {
            // ...Set long period
            *bit_buffer++ = _bc_pyq1648_pulse_count[1];
        }
        // ...If logical 0...
        else
        {
            // ...Set short period
            *bit_buffer++ = _bc_pyq1648_pulse_count[0];
        }
    }
}

static void _bc_pyq1648_preinit(bc_pyq1648_t *self)
{
    // Set inactive level
    bc_gpio_init(self->_gpio_channel_serin);
    bc_gpio_set_output(self->_gpio_channel_serin, 0);
    bc_gpio_set_mode(self->_gpio_channel_serin, BC_GPIO_MODE_OUTPUT);
}

static void _bc_pyq1648_init(bc_pyq1648_t *self)
{
    // Initialize SerialIn (SERIN) GPIO pin
    bc_gpio_init(self->_gpio_channel_serin);
    bc_gpio_set_mode(self->_gpio_channel_serin, BC_GPIO_MODE_ALTERNATE_2);

    // Initialize DirectLink (DL) GPIO pin
    bc_gpio_init(self->_gpio_channel_dl);
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);
    bc_gpio_set_pull(self->_gpio_channel_dl, BC_GPIO_PULL_DOWN);

    uint16_t bit_buffer[BC_PYQ1648_CONFIG_BIT_COUNT];
    _bc_pyq1648_compose_bit_buffer(self, bit_buffer);

    bc_irq_disable();

    // Enable PLL
    bc_system_pll_enable();

    // Enable peripheral
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Default state
    TIM3->CR2 = 0x00;
    TIM3->DIER = 0x00;
    TIM3->EGR = 0x00;
    TIM3->CCMR1 = 0x00;
    TIM3->DCR = 0x00;
    TIM3->DMAR = 0x00;
    TIM3->OR = 0x00;

    // Dirretion = up, clock division = 1
    TIM3->CR1 = 0x00;

    // Use internal clock
    TIM3->SMCR = 0x00;

    // Enable channel 3 compare value pre-load, mode = 1
    TIM3->CCMR2 = TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2;

    // Enable capture-compare on channel 3
    TIM3->CCER = TIM_CCER_CC3E;

    // Tick period = 31.25ns (Prescaler value = 0)
    TIM3->PSC = 0;

    // Update period = 100us
    TIM3->ARR = 3200;

    // Clear update flag
    TIM3->SR &= ~TIM_SR_UIF;

    // Run timer
    TIM3->CR1 |= TIM_CR1_CEN;

    for (int n = 0; n < BC_PYQ1648_CONFIG_BIT_COUNT; n++)
    {
        // Update compare value for next update
        TIM3->CCR3 = bit_buffer[n];

        // While update not occurred...
        while ((TIM3->SR & TIM_SR_UIF) == 0)
        {
            // ...do nothing
        }

        // Clear update flag
        TIM3->SR &= ~TIM_SR_UIF;
    }

    // Stop timer
    TIM3->CR1 &= ~(TIM_CR1_CEN);

    // Disable PLL
    bc_system_pll_disable();

    bc_irq_enable();

    // Set inactive level
    bc_gpio_set_output(self->_gpio_channel_serin, 0);
    bc_gpio_set_mode(self->_gpio_channel_serin, BC_GPIO_MODE_OUTPUT);
}
