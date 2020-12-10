
#include <stm32l0xx.h>
#include <twr_common.h>
#include <twr_gpio.h>
#include <twr_exti.h>
#include <twr_scheduler.h>
#include <twr_system.h>
#include "twr_ir_rx.h"
#include <twr_timer.h>

//#define _TWR_IR_RX_DEBUG

// IR receiver has to be connected on
// TWR_GPIO_P10
// No external pull-up required, internal pull-up is activated

static struct
{
    TIM_HandleTypeDef ir_timer;
    uint8_t counter;
    uint32_t ir_rx_temp;
    uint32_t ir_rx_value;

    twr_scheduler_task_id_t task_id_notify;

    void (*event_handler)(twr_ir_rx_event_t, void *);
    void *event_param;

    #ifdef _TWR_IR_RX_DEBUG
    uint16_t lengths[40];
    #endif
} _twr_ir_rx;

static void TIM6_handler(void *param);

void twr_ir_rx_get_code(uint32_t *nec_code)
{
    *nec_code = _twr_ir_rx.ir_rx_value;
}


static void _twr_ir_rx_task_notify(void *param)
{
    (void) param;

    if (_twr_ir_rx.event_handler != NULL)
    {
        _twr_ir_rx.event_handler(TWR_IR_RX_NEC_FORMAT, _twr_ir_rx.event_param);
    }
}

static void _twr_ir_rx_exti_int(twr_exti_line_t line, void *param)
{
    (void) param;
    (void) line;

    #ifdef _TWR_IR_RX_DEBUG
        twr_gpio_toggle_output(TWR_GPIO_P11);
    #endif

    if(_twr_ir_rx.counter == 0)
    {
        // Start pulse, start timer
        __HAL_TIM_DISABLE(&_twr_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_twr_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_IT(&_twr_ir_rx.ir_timer, TIM_IT_UPDATE);

        HAL_TIM_Base_Start_IT(&_twr_ir_rx.ir_timer);
        _twr_ir_rx.counter++;
        return;
    }

    // Get value and clear the timer
    uint16_t act_len = __HAL_TIM_GET_COUNTER(&_twr_ir_rx.ir_timer);
    __HAL_TIM_SET_COUNTER(&_twr_ir_rx.ir_timer, 0);

    // Array for debugging
    #ifdef _TWR_IR_RX_DEBUG
    _twr_ir_rx.lengths[_twr_ir_rx.counter] = act_len;
    #endif

    // Check start pulse length
    if(_twr_ir_rx.counter == 1)
    {
        if(act_len < 8000 || act_len > 14000)
        {
            // Ignore this packet
            volatile int a = 5;
            a++;
            return;
        }
    }

    if(_twr_ir_rx.counter == 2)
    {
        _twr_ir_rx.ir_rx_temp = 0;
    }

    // ignore first two items (first zero value and start pulse)
    if(act_len > 1500 && _twr_ir_rx.counter >= 2)
    {
        _twr_ir_rx.ir_rx_temp |= (1 << (_twr_ir_rx.counter-2));
    }

    // Received packet
    if(_twr_ir_rx.counter == 33)
    {
        #ifdef _TWR_IR_RX_DEBUG
        // Debug toggle
        twr_gpio_toggle_output(TWR_GPIO_P11);
        twr_gpio_toggle_output(TWR_GPIO_P11);
        twr_gpio_toggle_output(TWR_GPIO_P11);
        #endif

        _twr_ir_rx.counter = 0;
        _twr_ir_rx.ir_rx_value = _twr_ir_rx.ir_rx_temp;

        // The ir_rx_value should have inverted one addr and one command byte, but
        // the test of my IR remotes shows, that only the second CMD byte is inversion of the fist one

        __HAL_TIM_DISABLE(&_twr_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_twr_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_IT(&_twr_ir_rx.ir_timer, TIM_IT_UPDATE);

        twr_scheduler_plan_now(_twr_ir_rx.task_id_notify);

        return;
    }

    _twr_ir_rx.counter++;
}

void twr_ir_rx_set_event_handler(void (*event_handler)(twr_ir_rx_event_t, void *), void *event_param)
{
    _twr_ir_rx.event_handler = event_handler;
    _twr_ir_rx.event_param = event_param;
}

void twr_ir_rx_init()
{
    // TODO: Needs fix to allow low power
    twr_system_pll_enable();

    // IR input
    twr_gpio_init(TWR_GPIO_P10);
    twr_gpio_set_mode(TWR_GPIO_P10, TWR_GPIO_MODE_INPUT);
    twr_gpio_set_pull(TWR_GPIO_P10, TWR_GPIO_PULL_UP);
    // External interrupt
    twr_exti_register(TWR_EXTI_LINE_P10, TWR_EXTI_EDGE_FALLING, _twr_ir_rx_exti_int, NULL);

    #ifdef _TWR_IR_RX_DEBUG
    // Debug output
    twr_gpio_init(TWR_GPIO_P11);
    twr_gpio_set_mode(TWR_GPIO_P11, TWR_GPIO_MODE_OUTPUT);
    #endif

    // Used TIM6 so we don't have collision of IRQ handlers functions
    __TIM6_CLK_ENABLE();
    _twr_ir_rx.ir_timer.Instance = TIM6;
    // Running @ 32MHz, set timer to 1us resolution
    _twr_ir_rx.ir_timer.Init.Prescaler = SystemCoreClock / 1000000;
    _twr_ir_rx.ir_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    // Period is also the timeout in case the IR code is not complete
    _twr_ir_rx.ir_timer.Init.Period = 16000;
    _twr_ir_rx.ir_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&_twr_ir_rx.ir_timer);
    HAL_NVIC_EnableIRQ(TIM6_IRQn);

    twr_timer_set_irq_handler(TIM6, TIM6_handler, NULL);


    _twr_ir_rx.task_id_notify = twr_scheduler_register(_twr_ir_rx_task_notify, NULL, TWR_TICK_INFINITY);

}

static void TIM6_handler(void *param)
{
    (void) param;

    #ifdef _TWR_IR_RX_DEBUG
    twr_gpio_toggle_output(TWR_GPIO_P11);
    twr_gpio_toggle_output(TWR_GPIO_P11);
    twr_gpio_toggle_output(TWR_GPIO_P11);
    twr_gpio_toggle_output(TWR_GPIO_P11);
    #endif

    if(__HAL_TIM_GET_FLAG(&_twr_ir_rx.ir_timer, TIM_FLAG_UPDATE) != RESET)
    {
      if(__HAL_TIM_GET_IT_SOURCE(&_twr_ir_rx.ir_timer, TIM_IT_UPDATE) !=RESET)
      {
        // Stop timer
        __HAL_TIM_CLEAR_IT(&_twr_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_DISABLE(&_twr_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_twr_ir_rx.ir_timer, TIM_IT_UPDATE);

        _twr_ir_rx.counter = 0;
      }
    }
}
