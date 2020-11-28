
#include <stm32l0xx.h>
#include <hio_common.h>
#include <hio_gpio.h>
#include <hio_exti.h>
#include <hio_scheduler.h>
#include <hio_system.h>
#include "hio_ir_rx.h"
#include <hio_timer.h>

//#define _HIO_IR_RX_DEBUG

// IR receiver has to be connected on
// HIO_GPIO_P10
// No external pull-up required, internal pull-up is activated

static struct
{
    TIM_HandleTypeDef ir_timer;
    uint8_t counter;
    uint32_t ir_rx_temp;
    uint32_t ir_rx_value;

    hio_scheduler_task_id_t task_id_notify;

    void (*event_handler)(hio_ir_rx_event_t, void *);
    void *event_param;

    #ifdef _HIO_IR_RX_DEBUG
    uint16_t lengths[40];
    #endif
} _hio_ir_rx;

static void TIM6_handler(void *param);

void hio_ir_rx_get_code(uint32_t *nec_code)
{
    *nec_code = _hio_ir_rx.ir_rx_value;
}


static void _hio_ir_rx_task_notify(void *param)
{
    (void) param;

    if (_hio_ir_rx.event_handler != NULL)
    {
        _hio_ir_rx.event_handler(HIO_IR_RX_NEC_FORMAT, _hio_ir_rx.event_param);
    }
}

static void _hio_ir_rx_exti_int(hio_exti_line_t line, void *param)
{
    (void) param;
    (void) line;

    #ifdef _HIO_IR_RX_DEBUG
        hio_gpio_toggle_output(HIO_GPIO_P11);
    #endif

    if(_hio_ir_rx.counter == 0)
    {
        // Start pulse, start timer
        __HAL_TIM_DISABLE(&_hio_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_hio_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_IT(&_hio_ir_rx.ir_timer, TIM_IT_UPDATE);

        HAL_TIM_Base_Start_IT(&_hio_ir_rx.ir_timer);
        _hio_ir_rx.counter++;
        return;
    }

    // Get value and clear the timer
    uint16_t act_len = __HAL_TIM_GET_COUNTER(&_hio_ir_rx.ir_timer);
    __HAL_TIM_SET_COUNTER(&_hio_ir_rx.ir_timer, 0);

    // Array for debugging
    #ifdef _HIO_IR_RX_DEBUG
    _hio_ir_rx.lengths[_hio_ir_rx.counter] = act_len;
    #endif

    // Check start pulse length
    if(_hio_ir_rx.counter == 1)
    {
        if(act_len < 8000 || act_len > 14000)
        {
            // Ignore this packet
            volatile int a = 5;
            a++;
            return;
        }
    }

    if(_hio_ir_rx.counter == 2)
    {
        _hio_ir_rx.ir_rx_temp = 0;
    }

    // ignore first two items (first zero value and start pulse)
    if(act_len > 1500 && _hio_ir_rx.counter >= 2)
    {
        _hio_ir_rx.ir_rx_temp |= (1 << (_hio_ir_rx.counter-2));
    }

    // Received packet
    if(_hio_ir_rx.counter == 33)
    {
        #ifdef _HIO_IR_RX_DEBUG
        // Debug toggle
        hio_gpio_toggle_output(HIO_GPIO_P11);
        hio_gpio_toggle_output(HIO_GPIO_P11);
        hio_gpio_toggle_output(HIO_GPIO_P11);
        #endif

        _hio_ir_rx.counter = 0;
        _hio_ir_rx.ir_rx_value = _hio_ir_rx.ir_rx_temp;

        // The ir_rx_value should have inverted one addr and one command byte, but
        // the test of my IR remotes shows, that only the second CMD byte is inversion of the fist one

        __HAL_TIM_DISABLE(&_hio_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_hio_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_IT(&_hio_ir_rx.ir_timer, TIM_IT_UPDATE);

        hio_scheduler_plan_now(_hio_ir_rx.task_id_notify);

        return;
    }

    _hio_ir_rx.counter++;
}

void hio_ir_rx_set_event_handler(void (*event_handler)(hio_ir_rx_event_t, void *), void *event_param)
{
    _hio_ir_rx.event_handler = event_handler;
    _hio_ir_rx.event_param = event_param;
}

void hio_ir_rx_init()
{
    // TODO: Needs fix to allow low power
    hio_system_pll_enable();

    // IR input
    hio_gpio_init(HIO_GPIO_P10);
    hio_gpio_set_mode(HIO_GPIO_P10, HIO_GPIO_MODE_INPUT);
    hio_gpio_set_pull(HIO_GPIO_P10, HIO_GPIO_PULL_UP);
    // External interrupt
    hio_exti_register(HIO_EXTI_LINE_P10, HIO_EXTI_EDGE_FALLING, _hio_ir_rx_exti_int, NULL);

    #ifdef _HIO_IR_RX_DEBUG
    // Debug output
    hio_gpio_init(HIO_GPIO_P11);
    hio_gpio_set_mode(HIO_GPIO_P11, HIO_GPIO_MODE_OUTPUT);
    #endif

    // Used TIM6 so we don't have collision of IRQ handlers functions
    __TIM6_CLK_ENABLE();
    _hio_ir_rx.ir_timer.Instance = TIM6;
    // Running @ 32MHz, set timer to 1us resolution
    _hio_ir_rx.ir_timer.Init.Prescaler = SystemCoreClock / 1000000;
    _hio_ir_rx.ir_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    // Period is also the timeout in case the IR code is not complete
    _hio_ir_rx.ir_timer.Init.Period = 16000;
    _hio_ir_rx.ir_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&_hio_ir_rx.ir_timer);
    HAL_NVIC_EnableIRQ(TIM6_IRQn);

    hio_timer_set_irq_handler(TIM6, TIM6_handler, NULL);


    _hio_ir_rx.task_id_notify = hio_scheduler_register(_hio_ir_rx_task_notify, NULL, HIO_TICK_INFINITY);

}

static void TIM6_handler(void *param)
{
    (void) param;

    #ifdef _HIO_IR_RX_DEBUG
    hio_gpio_toggle_output(HIO_GPIO_P11);
    hio_gpio_toggle_output(HIO_GPIO_P11);
    hio_gpio_toggle_output(HIO_GPIO_P11);
    hio_gpio_toggle_output(HIO_GPIO_P11);
    #endif

    if(__HAL_TIM_GET_FLAG(&_hio_ir_rx.ir_timer, TIM_FLAG_UPDATE) != RESET)
    {
      if(__HAL_TIM_GET_IT_SOURCE(&_hio_ir_rx.ir_timer, TIM_IT_UPDATE) !=RESET)
      {
        // Stop timer
        __HAL_TIM_CLEAR_IT(&_hio_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_DISABLE(&_hio_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_hio_ir_rx.ir_timer, TIM_IT_UPDATE);

        _hio_ir_rx.counter = 0;
      }
    }
}
