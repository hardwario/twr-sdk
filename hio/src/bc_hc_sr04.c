#include <hio_hc_sr04.h>
#include <hio_system.h>
#include <stm32l0xx.h>
#include <hio_timer.h>

// Timer resolution in microseconds
#define _HIO_HC_SR04_RESOLUTION 5

static void _hio_hc_sr04_task_interval(void *param);
static void _hio_hc_sr04_task_notify(void *param);
static void _hio_hc_sr04_tim2_iqr_handler(void *param);
static void _hio_hc_sr04_tim3_iqr_handler(void *param);

void hio_hc_sr04_init_sensor_module(hio_hc_sr04_t *self)
{
    hio_hc_sr04_init(self, HIO_GPIO_P4, HIO_HC_SR04_ECHO_P5);
}

void hio_hc_sr04_init(hio_hc_sr04_t *self, hio_gpio_channel_t trig, hio_hc_sr04_echo_t echo)
{
    memset(self, 0, sizeof(*self));

    self->_echo = echo;
    self->_trig = trig;

    self->_task_id_interval = hio_scheduler_register(_hio_hc_sr04_task_interval, self, HIO_TICK_INFINITY);

    // Pin Trig
    hio_gpio_init(self->_trig);

    hio_gpio_set_mode(self->_trig, HIO_GPIO_MODE_OUTPUT);

    hio_gpio_set_output(self->_trig, 0);

    // Pin Echo
    if (self->_echo == HIO_HC_SR04_ECHO_P5)
    {
        hio_gpio_init(HIO_GPIO_P5);

        hio_gpio_set_mode(HIO_GPIO_P5, HIO_GPIO_MODE_ALTERNATE_5);

        // Enable TIM2 clock
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

        // Errata workaround
        RCC->APB1ENR;

        // Capture 1 and Capture 2 is connected to TI1
        TIM2->CCMR1 |= TIM_CCMR1_CC2S_1 | TIM_CCMR1_CC1S_0;

        // Capture 2 is sensitive on falling edge
        TIM2->CCER |= 1 << TIM_CCER_CC2P_Pos;

        // Set prescaler to 5 * 32 (5 microseconds resolution)
        TIM2->PSC = _HIO_HC_SR04_RESOLUTION * 32 - 1;

        hio_timer_set_irq_handler(TIM2, _hio_hc_sr04_tim2_iqr_handler, self);

        // Enable TIM2 interrupts
        NVIC_EnableIRQ(TIM2_IRQn);
    }
    else if (self->_echo == HIO_HC_SR04_ECHO_P8)
    {
        hio_gpio_init(HIO_GPIO_P8);

        hio_gpio_set_mode(HIO_GPIO_P8, HIO_GPIO_MODE_ALTERNATE_2);

        // Enable TIM3 clock
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

        // Errata workaround
        RCC->APB1ENR;

        // Capture 4 and Capture 3 is connected to TI3
        TIM3->CCMR2 |= TIM_CCMR2_CC4S_1 | TIM_CCMR2_CC3S_0;

        // Capture 4 is sensitive on falling edge
        TIM3->CCER |= 1 << TIM_CCER_CC4P_Pos;

        // Set prescaler to 5 * 32 (5 microseconds resolution)
        TIM3->PSC = _HIO_HC_SR04_RESOLUTION * 32 - 1;

        hio_timer_set_irq_handler(TIM3, _hio_hc_sr04_tim3_iqr_handler, self);

        // Enable TIM3 interrupts
        NVIC_EnableIRQ(TIM3_IRQn);
    }

    hio_timer_init();
}

void hio_hc_sr04_set_event_handler(hio_hc_sr04_t *self, void (*event_handler)(hio_hc_sr04_t *, hio_hc_sr04_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void hio_hc_sr04_set_update_interval(hio_hc_sr04_t *self, hio_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(self->_task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        hio_hc_sr04_measure(self);
    }
}

bool hio_hc_sr04_measure(hio_hc_sr04_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    // Enable PLL
    hio_system_pll_enable();

    self->_measurement_active = true;

    self->_measurement_valid = false;

    self->_task_id_notify = hio_scheduler_register(_hio_hc_sr04_task_notify, self, HIO_TICK_INFINITY);

    if (self->_echo == HIO_HC_SR04_ECHO_P5)
    {
        // Set timeout register (250 milliseconds)
        TIM2->CCR3 = 250000 / _HIO_HC_SR04_RESOLUTION;

        // Trigger update
        TIM2->EGR = TIM_EGR_UG;

        // Clear Capture 1 / Capture 2 overcapture flags
        TIM2->SR &= ~(TIM_SR_CC1OF | TIM_SR_CC2OF);

        // Clear Capture 1 / Capture 2 / Compare 3 interrupt flags
        TIM2->SR &= ~(TIM_SR_CC1IF | TIM_SR_CC2IF | TIM_SR_CC3IF);

        // Enable timer counter
        TIM2->CR1 |= TIM_CR1_CEN;

        // Enable Capture 1 / Capture 2
        TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

        // Enable Capture 2 / Compare 3 interrupt
        TIM2->DIER |= TIM_DIER_CC2IE | TIM_DIER_CC3IE;
    }
    else if (self->_echo == HIO_HC_SR04_ECHO_P8)
    {
        // Set timeout register (250 milliseconds)
        TIM3->CCR1 = 250000 / _HIO_HC_SR04_RESOLUTION;

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
    }

    hio_gpio_set_output(self->_trig, 1);

    hio_timer_start();

    hio_timer_delay(10);

    hio_timer_stop();

    hio_gpio_set_output(self->_trig, 0);

    return true;
}

bool hio_hc_sr04_get_distance_millimeter(hio_hc_sr04_t *self, float *millimeter)
{
    if (!self->_measurement_valid)
    {
        *millimeter = NAN;

        return false;
    }

    *millimeter = _HIO_HC_SR04_RESOLUTION * (float) self->_echo_duration / 5.8;

    return true;
}

static void _hio_hc_sr04_task_interval(void *param)
{
    hio_hc_sr04_t *self = (hio_hc_sr04_t *) param;

    hio_hc_sr04_measure(self);

    hio_scheduler_plan_current_relative(self->_update_interval);
}

static void _hio_hc_sr04_task_notify(void *param)
{
    hio_hc_sr04_t *self = (hio_hc_sr04_t *) param;

    // Disable PLL
    hio_system_pll_disable();

    self->_measurement_active = false;

    hio_scheduler_unregister(self->_task_id_notify);

    if (!self->_measurement_valid)
    {
        if (self->_event_handler != NULL)
        {
            self->_event_handler(self, HIO_HC_SR04_EVENT_ERROR, self->_event_param);
        }
    }
    else if (self->_event_handler != NULL)
    {
        self->_event_handler(self, HIO_HC_SR04_EVENT_UPDATE, self->_event_param);
    }
}

static void _hio_hc_sr04_tim3_iqr_handler(void *param)
{
    hio_hc_sr04_t *self = (hio_hc_sr04_t *) param;

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

                    if (self->_echo_duration <= 30000 / _HIO_HC_SR04_RESOLUTION)
                    {
                        // Indicate success
                        self->_measurement_valid = true;
                    }
                }
            }
        }
    }

    // Schedule task for immediate execution
    hio_scheduler_plan_now(self->_task_id_notify);
}

static void _hio_hc_sr04_tim2_iqr_handler(void *param)
{
    hio_hc_sr04_t *self = (hio_hc_sr04_t *) param;

    // Disable Capture 2 / Compare 3 interrupt
    TIM2->DIER &= ~(TIM_DIER_CC2IE | TIM_DIER_CC3IE);

    // Disable Capture 1 / Capture 2
    TIM2->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E);

    // Disable timer counter
    TIM2->CR1 &= ~TIM_CR1_CEN;

    // If not Compare 3 interrupt... (timeout)
    if ((TIM2->SR & TIM_SR_CC3IF) == 0)
    {
        // If not Capture 1 / Capture 2 overcapture...
        if ((TIM2->SR & TIM_SR_CC1OF) == 0 && (TIM2->SR & TIM_SR_CC2OF) == 0)
        {
            // If Capture 2 interrupt... (falling edge)
            if ((TIM2->SR & TIM_SR_CC2IF) != 0)
            {
                // Retrieve falling edge capture mark
                uint16_t falling_capture = TIM2->CCR2;

                // If Capture 3 interrupt... (rising edge)
                if ((TIM2->SR & TIM_SR_CC1IF) != 0)
                {
                    // Retrieve rising edge capture mark
                    uint16_t rising_capture = TIM2->CCR1;

                    // Calculate echo duration (distance between rising and falling edge)
                    self->_echo_duration = falling_capture - rising_capture;

                    if (self->_echo_duration <= 30000 / _HIO_HC_SR04_RESOLUTION)
                    {
                        // Indicate success
                        self->_measurement_valid = true;
                    }
                }
            }
        }
    }

    // Schedule task for immediate execution
    hio_scheduler_plan_now(self->_task_id_notify);
}
