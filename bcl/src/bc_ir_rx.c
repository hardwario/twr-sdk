
#include <stm32l0xx.h>
#include <bc_common.h>
#include <bc_gpio.h>
#include <bc_exti.h>
#include <bc_scheduler.h>
#include <bc_system.h>
#include "bc_ir_rx.h"

//#define _BC_IR_RX_DEBUG

// IR receiver has to be connected on
// BC_GPIO_P10
// No external pull-up required, internal pull-up is activated

static struct
{
    TIM_HandleTypeDef ir_timer;
    uint8_t counter;
    uint32_t ir_rx_temp;
    uint32_t ir_rx_value;

    bc_scheduler_task_id_t task_id_notify;

    void (*event_handler)(bc_ir_rx_event_t, void *);
    void *event_param;

    #ifdef _BC_IR_RX_DEBUG
    uint16_t lengths[40];
    #endif
} _bc_ir_rx;


void bc_ir_rx_get_code(uint32_t *nec_code)
{
    *nec_code = _bc_ir_rx.ir_rx_value;
}


static void _bc_ir_rx_task_notify(void *param)
{
    (void) param;

    if (_bc_ir_rx.event_handler != NULL)
    {
        _bc_ir_rx.event_handler(BC_IR_RX_NEC_FORMAT, _bc_ir_rx.event_param);
    }
}

static void _bc_ir_rx_exti_int(bc_exti_line_t line, void *param)
{
    (void) param;
    (void) line;

    #ifdef _BC_IR_RX_DEBUG
        bc_gpio_toggle_output(BC_GPIO_P11);
    #endif

    if(_bc_ir_rx.counter == 0)
    {
        // Start pulse, start timer
        __HAL_TIM_DISABLE(&_bc_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_bc_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_IT(&_bc_ir_rx.ir_timer, TIM_IT_UPDATE);

        HAL_TIM_Base_Start_IT(&_bc_ir_rx.ir_timer);
        _bc_ir_rx.counter++;
        return;
    }

    // Get value and clear the timer
    uint16_t act_len = __HAL_TIM_GET_COUNTER(&_bc_ir_rx.ir_timer);
    __HAL_TIM_SET_COUNTER(&_bc_ir_rx.ir_timer, 0);

    // Array for debugging
    #ifdef _BC_IR_RX_DEBUG
    _bc_ir_rx.lengths[_bc_ir_rx.counter] = act_len;
    #endif

    // Check start pulse length
    if(_bc_ir_rx.counter == 1)
    {
        if(act_len < 8000 || act_len > 14000)
        {
            // Ignore this packet
            volatile int a = 5;
            a++;
            return;
        }
    }

    if(_bc_ir_rx.counter == 2)
    {
        _bc_ir_rx.ir_rx_temp = 0;
    }

    // ignore first two items (first zero value and start pulse)
    if(act_len > 1500 && _bc_ir_rx.counter >= 2)
    {
        _bc_ir_rx.ir_rx_temp |= (1 << (_bc_ir_rx.counter-2));
    }

    // Received packet
    if(_bc_ir_rx.counter == 33)
    {
        #ifdef _BC_IR_RX_DEBUG
        // Debug toggle
        bc_gpio_toggle_output(BC_GPIO_P11);
        bc_gpio_toggle_output(BC_GPIO_P11);
        bc_gpio_toggle_output(BC_GPIO_P11);
        #endif

        _bc_ir_rx.counter = 0;
        _bc_ir_rx.ir_rx_value = _bc_ir_rx.ir_rx_temp;

        // The ir_rx_value should have inverted one addr and one command byte, but
        // the test of my IR remotes shows, that only the second CMD byte is inversion of the fist one

        __HAL_TIM_DISABLE(&_bc_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_bc_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_CLEAR_IT(&_bc_ir_rx.ir_timer, TIM_IT_UPDATE);

        bc_scheduler_plan_now(_bc_ir_rx.task_id_notify);

        return;
    }

    _bc_ir_rx.counter++;
}

void bc_ir_rx_set_event_handler(void (*event_handler)(bc_ir_rx_event_t, void *), void *event_param)
{
    _bc_ir_rx.event_handler = event_handler;
    _bc_ir_rx.event_param = event_param;
}

void bc_ir_rx_init()
{
    // TODO: Needs fix to allow low power
    bc_system_pll_enable();

    // IR input
    bc_gpio_init(BC_GPIO_P10);
    bc_gpio_set_mode(BC_GPIO_P10, BC_GPIO_MODE_INPUT);
    bc_gpio_set_pull(BC_GPIO_P10, BC_GPIO_PULL_UP);
    // External interrupt
    bc_exti_register(BC_EXTI_LINE_P10, BC_EXTI_EDGE_FALLING, _bc_ir_rx_exti_int, NULL);

    #ifdef _BC_IR_RX_DEBUG
    // Debug output
    bc_gpio_init(BC_GPIO_P11);
    bc_gpio_set_mode(BC_GPIO_P11, BC_GPIO_MODE_OUTPUT);
    #endif

    // Used TIM7 so we don't have collision of IRQ handlers functions
    __TIM6_CLK_ENABLE();
    _bc_ir_rx.ir_timer.Instance = TIM6;
    // Running @ 32MHz, set timer to 1us resolution
    _bc_ir_rx.ir_timer.Init.Prescaler = SystemCoreClock / 1000000;
    _bc_ir_rx.ir_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    // Period is also the timeout in case the IR code is not complete
    _bc_ir_rx.ir_timer.Init.Period = 16000;
    _bc_ir_rx.ir_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&_bc_ir_rx.ir_timer);
    HAL_NVIC_EnableIRQ(TIM6_IRQn);

    _bc_ir_rx.task_id_notify = bc_scheduler_register(_bc_ir_rx_task_notify, NULL, BC_TICK_INFINITY);

}


void TIM6_IRQHandler()
{
    #ifdef _BC_IR_RX_DEBUG
    bc_gpio_toggle_output(BC_GPIO_P11);
    bc_gpio_toggle_output(BC_GPIO_P11);
    bc_gpio_toggle_output(BC_GPIO_P11);
    bc_gpio_toggle_output(BC_GPIO_P11);
    #endif

    if(__HAL_TIM_GET_FLAG(&_bc_ir_rx.ir_timer, TIM_FLAG_UPDATE) != RESET)
    {
      if(__HAL_TIM_GET_IT_SOURCE(&_bc_ir_rx.ir_timer, TIM_IT_UPDATE) !=RESET)
      {
        // Stop timer
        __HAL_TIM_CLEAR_IT(&_bc_ir_rx.ir_timer, TIM_IT_UPDATE);
        __HAL_TIM_DISABLE(&_bc_ir_rx.ir_timer);
        __HAL_TIM_DISABLE_IT(&_bc_ir_rx.ir_timer, TIM_IT_UPDATE);

        _bc_ir_rx.counter = 0;
      }
    }
}
