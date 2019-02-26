#include <bc_hc_sr04.h>
#include <bc_scheduler.h>
#include <bc_system.h>
#include <stm32l0xx.h>
#include <bc_gpio.h>
#include <bc_timer.h>

// Timer resolution in microseconds
#define _BC_HC_SR04_RESOLUTION 5

static struct
{
    bc_scheduler_task_id_t task_id_interval;
    bc_scheduler_task_id_t task_id_notify;
    void (*event_handler)(bc_hc_sr04_event_t, void *);
    void *event_param;
    bc_tick_t update_interval;
    bool measurement_active;
    bool measurement_valid;
    uint16_t echo_duration;

} _bc_hc_sr04;

static void _bc_hc_sr04_task_interval(void *param);

static void _bc_hc_sr04_task_notify(void *param);

static void _bc_hc_sr04_iqr_handler(void *param);

void bc_hc_sr04_init(void)
{
    memset(&_bc_hc_sr04, 0, sizeof(_bc_hc_sr04));

    _bc_hc_sr04.task_id_interval = bc_scheduler_register(_bc_hc_sr04_task_interval, NULL, BC_TICK_INFINITY);

    // Pin Echo
    bc_gpio_init(BC_GPIO_P8);

    bc_gpio_set_mode(BC_GPIO_P8, BC_GPIO_MODE_ALTERNATE_2);

    // Pin Trig
    bc_gpio_init(BC_GPIO_P9);

    bc_gpio_set_mode(BC_GPIO_P9, BC_GPIO_MODE_OUTPUT);

    bc_gpio_set_output(BC_GPIO_P9, 0);

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

    bc_timer_set_irq_handler(TIM3, _bc_hc_sr04_iqr_handler, NULL);

    // Enable TIM3 interrupts
    NVIC_EnableIRQ(TIM3_IRQn);

    bc_timer_init();
}

void bc_hc_sr04_set_event_handler(void (*event_handler)(bc_hc_sr04_event_t, void *), void *event_param)
{
    _bc_hc_sr04.event_handler = event_handler;
    _bc_hc_sr04.event_param = event_param;
}

void bc_hc_sr04_set_update_interval(bc_tick_t interval)
{
    _bc_hc_sr04.update_interval = interval;

    if (_bc_hc_sr04.update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(_bc_hc_sr04.task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(_bc_hc_sr04.task_id_interval, _bc_hc_sr04.update_interval);

        bc_hc_sr04_measure();
    }
}

bool bc_hc_sr04_measure(void)
{
    if (_bc_hc_sr04.measurement_active)
    {
        return false;
    }

    // Enable PLL
    bc_system_pll_enable();

    _bc_hc_sr04.measurement_active = true;

    _bc_hc_sr04.measurement_valid = false;

    _bc_hc_sr04.task_id_notify = bc_scheduler_register(_bc_hc_sr04_task_notify, NULL, BC_TICK_INFINITY);

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

    bc_gpio_set_output(BC_GPIO_P9, 1);

    bc_timer_start();

    bc_timer_delay(10);

    bc_timer_stop();

    bc_gpio_set_output(BC_GPIO_P9, 0);

    return true;
}

bool bc_hc_sr04_get_distance_millimeter(float *millimeter)
{
    if (!_bc_hc_sr04.measurement_valid)
    {
        return false;
    }

    *millimeter = _BC_HC_SR04_RESOLUTION * (float) _bc_hc_sr04.echo_duration / 5.8;

    return true;
}

static void _bc_hc_sr04_task_interval(void *param)
{
    (void) param;

    bc_hc_sr04_measure();

    bc_scheduler_plan_current_relative(_bc_hc_sr04.update_interval);
}

static void _bc_hc_sr04_task_notify(void *param)
{
    (void) param;

    // Disable PLL
    bc_system_pll_disable();

    _bc_hc_sr04.measurement_active = false;

    bc_scheduler_unregister(_bc_hc_sr04.task_id_notify);

    if (!_bc_hc_sr04.measurement_valid)
    {
        if (_bc_hc_sr04.event_handler != NULL)
        {
            _bc_hc_sr04.event_handler(BC_HC_SR04_EVENT_ERROR, _bc_hc_sr04.event_param);
        }
    }
    else if (_bc_hc_sr04.event_handler != NULL)
    {
        _bc_hc_sr04.event_handler(BC_HC_SR04_EVENT_UPDATE, _bc_hc_sr04.event_param);
    }
}

static void _bc_hc_sr04_iqr_handler(void *param)
{
    (void) param;

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
                    _bc_hc_sr04.echo_duration = falling_capture - rising_capture;

                    if (_bc_hc_sr04.echo_duration <= 30000 / _BC_HC_SR04_RESOLUTION)
                    {
                        // Indicate success
                        _bc_hc_sr04.measurement_valid = true;
                    }
                }
            }
        }
    }

    // Schedule task for immediate execution
    bc_scheduler_plan_now(_bc_hc_sr04.task_id_notify);
}


