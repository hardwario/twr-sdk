#include <bc_hc_sr04.h>
#include <bc_system.h>
#include <stm32l0xx.h>
#include <bc_timer.h>

// Timer resolution in microseconds
#define _BC_HC_SR04_RESOLUTION 5

static void _bc_hc_sr04_task_interval(void *param);

static void _bc_hc_sr04_task_notify(void *param);

static void _bc_hc_sr04_iqr_handler(void *param);

void bc_hc_sr04_init(bc_hc_sr04_t *self, bc_hc_sr04_echo_t echo, bc_gpio_channel_t trig)
{
    memset(self, 0, sizeof(*self));

    self->_echo = echo;
    self->_trig = trig;

    self->_task_id_interval = bc_scheduler_register(_bc_hc_sr04_task_interval, self, BC_TICK_INFINITY);

    // Pin Echo
    bc_gpio_init(BC_GPIO_P8);

    bc_gpio_set_mode(BC_GPIO_P8, BC_GPIO_MODE_ALTERNATE_2);

    // Pin Trig
    bc_gpio_init(self->_trig);

    bc_gpio_set_mode(self->_trig, BC_GPIO_MODE_OUTPUT);

    bc_gpio_set_output(self->_trig, 0);

    // Enable TIM3 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Errata workaround
    RCC->APB1ENR;

    // Capture 4 and Capture 3 is connected to TI3
    TIM3->CCMR2 |= TIM_CCMR2_CC4S_1 | TIM_CCMR2_CC3S_0;

    // Capture 4 is sensitive on falling edge
    TIM3->CCER |= 1 << TIM_CCER_CC4P_Pos;

    // Set prescaler to 5 * 32 (5 microseconds resolution)
    TIM3->PSC = _BC_HC_SR04_RESOLUTION * 32 - 1;

    bc_timer_set_irq_handler(TIM3, _bc_hc_sr04_iqr_handler, self);

    // Enable TIM3 interrupts
    NVIC_EnableIRQ(TIM3_IRQn);

    bc_timer_init();
}

void bc_hc_sr04_set_event_handler(bc_hc_sr04_t *self, void (*event_handler)(bc_hc_sr04_t *, bc_hc_sr04_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_hc_sr04_set_update_interval(bc_hc_sr04_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_hc_sr04_measure(self);
    }
}

bool bc_hc_sr04_measure(bc_hc_sr04_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    // Enable PLL
    bc_system_pll_enable();

    self->_measurement_active = true;

    self->_measurement_valid = false;

    self->_task_id_notify = bc_scheduler_register(_bc_hc_sr04_task_notify, self, BC_TICK_INFINITY);

    // Set timeout register (250 milliseconds)
    TIM3->CCR1 = 250000 / _BC_HC_SR04_RESOLUTION;

    // Trigger update
    TIM3->EGR = TIM_EGR_UG;

    // Clear Capture 4 / Capture 3 overcapture flags
    TIM3->SR &= ~(TIM_SR_CC4OF | TIM_SR_CC3OF);

    // Clear Capture 4 / Capture 3 / Compare 1 interrupt flags
    TIM3->SR &= ~(TIM_SR_CC4IF | TIM_SR_CC3IF | TIM_SR_CC1IF);

    // Enable timer counter
    TIM3->CR1 |= TIM_CR1_CEN;

    // Enable Capture 4 / Capture 3
    TIM3->CCER |= TIM_CCER_CC4E | TIM_CCER_CC3E;

    // Enable Capture 4 / Compare 1 interrupt
    TIM3->DIER |= TIM_DIER_CC4IE | TIM_DIER_CC1IE;

    bc_gpio_set_output(self->_trig, 1);

    bc_timer_start();

    bc_timer_delay(10);

    bc_timer_stop();

    bc_gpio_set_output(self->_trig, 0);

    return true;
}

bool bc_hc_sr04_get_distance_millimeter(bc_hc_sr04_t *self, float *millimeter)
{
    if (!self->_measurement_valid)
    {
        return false;
    }

    *millimeter = _BC_HC_SR04_RESOLUTION * (float) self->_echo_duration / 5.8;

    return true;
}

static void _bc_hc_sr04_task_interval(void *param)
{
    bc_hc_sr04_t *self = (bc_hc_sr04_t *) param;

    bc_hc_sr04_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_hc_sr04_task_notify(void *param)
{
    bc_hc_sr04_t *self = (bc_hc_sr04_t *) param;

    // Disable PLL
    bc_system_pll_disable();

    self->_measurement_active = false;

    bc_scheduler_unregister(self->_task_id_notify);

    if (!self->_measurement_valid)
    {
        if (self->_event_handler != NULL)
        {
            self->_event_handler(self, BC_HC_SR04_EVENT_ERROR, self->_event_param);
        }
    }
    else if (self->_event_handler != NULL)
    {
        self->_event_handler(self, BC_HC_SR04_EVENT_UPDATE, self->_event_param);
    }
}

static void _bc_hc_sr04_iqr_handler(void *param)
{
    bc_hc_sr04_t *self = (bc_hc_sr04_t *) param;

    // Disable Capture 4 / Compare 1 interrupt
    TIM3->DIER &= ~(TIM_DIER_CC4IE | TIM_DIER_CC1IE);

    // Disable Capture 4 / Capture 3
    TIM3->CCER &= ~(TIM_CCER_CC4E | TIM_CCER_CC3E);

    // Disable timer counter
    TIM3->CR1 &= ~TIM_CR1_CEN;

    // If not Compare 1 interrupt... (timeout)
    if ((TIM3->SR & TIM_SR_CC1IF) == 0)
    {
        // If not Capture 4 / Capture 3 overcapture...
        if ((TIM3->SR & TIM_SR_CC4OF) == 0 && (TIM3->SR & TIM_SR_CC3OF) == 0)
        {
            // If Capture 4 interrupt... (falling edge)
            if ((TIM3->SR & TIM_SR_CC4IF) != 0)
            {
                // Retrieve falling edge capture mark
                uint16_t falling_capture = TIM3->CCR4;

                // If Capture 3 interrupt... (rising edge)
                if ((TIM3->SR & TIM_SR_CC3IF) != 0)
                {
                    // Retrieve rising edge capture mark
                    uint16_t rising_capture = TIM3->CCR3;

                    // Calculate echo duration (distance between rising and falling edge)
                    self->_echo_duration = falling_capture - rising_capture;

                    if (self->_echo_duration <= 30000 / _BC_HC_SR04_RESOLUTION)
                    {
                        // Indicate success
                        self->_measurement_valid = true;
                    }
                }
            }
        }
    }

    // Schedule task for immediate execution
    bc_scheduler_plan_now(self->_task_id_notify);
}


