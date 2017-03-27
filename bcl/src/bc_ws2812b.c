#include "stm32l0xx.h"
#include <bc_ws2812b.h>
#include <bc_scheduler.h>

#define _BC_WS2812_TIMER_PERIOD 40               // 32000000 / 800000 = 20; 0,125us period (10 times lower the 1,25us period to have fixed math below)
#define _BC_WS2812_TIMER_RESET_PULSE_PERIOD 1666 // 60us just to be sure = (32000000 / (320 * 60))
#define _BC_WS2812_COMPARE_PULSE_LOGIC_0     11  //(10 * timer_period) / 36
#define _BC_WS2812_COMPARE_PULSE_LOGIC_1     26  //(10 * timer_period) / 15;

#define _BC_WS2812_BC_WS2812_RESET_PERIOD 100
#define _BC_WS2812_BC_WS2812B_PORT GPIOA
#define _BC_WS2812_BC_WS2812B_PIN GPIO_PIN_1

static struct ws2812b_t
{
    uint32_t *dma_bit_buffer;
    bc_led_strip_buffer_t *buffer;

    bool transfer;
    bc_scheduler_task_id_t task_id;
    void (*event_handler)(bc_ws2812b_event_t, void *);
    void *event_param;

} _bc_ws2812b;

DMA_HandleTypeDef _bc_ws2812b_dma_update;
TIM_HandleTypeDef _bc_ws2812b_timer2_handle;
TIM_OC_InitTypeDef _bc_ws2812b_timer2_oc1;

const uint32_t _bc_ws2812b_pulse_tab[] =
{
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_0,
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
    _BC_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
    _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _BC_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _BC_WS2812_COMPARE_PULSE_LOGIC_1,
};

static void _bc_ws2812b_dma_transfer_complete_handler(DMA_HandleTypeDef *dma_handle);
static void _bc_ws2812b_task(void *param);

bool bc_ws2812b_init(bc_led_strip_buffer_t *led_strip)
{
    memset(&_bc_ws2812b, 0, sizeof(_bc_ws2812b));

    _bc_ws2812b.buffer = led_strip;

    _bc_ws2812b.dma_bit_buffer = led_strip->buffer;

    size_t dma_bit_buffer_size = _bc_ws2812b.buffer->count * _bc_ws2812b.buffer->type * 8;

    memset(_bc_ws2812b.dma_bit_buffer, _BC_WS2812_COMPARE_PULSE_LOGIC_0, dma_bit_buffer_size);

    __HAL_RCC_GPIOA_CLK_ENABLE();

    //Init pin
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = _BC_WS2812_BC_WS2812B_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(_BC_WS2812_BC_WS2812B_PORT, &GPIO_InitStruct);

    //Init dma
    __HAL_RCC_DMA1_CLK_ENABLE();

    _bc_ws2812b_dma_update.Init.Direction = DMA_MEMORY_TO_PERIPH;
    _bc_ws2812b_dma_update.Init.PeriphInc = DMA_PINC_DISABLE;
    _bc_ws2812b_dma_update.Init.MemInc = DMA_MINC_ENABLE;
    _bc_ws2812b_dma_update.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    _bc_ws2812b_dma_update.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    _bc_ws2812b_dma_update.Init.Mode = DMA_CIRCULAR;
    _bc_ws2812b_dma_update.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    _bc_ws2812b_dma_update.Instance = DMA1_Channel2;
    _bc_ws2812b_dma_update.Init.Request = DMA_REQUEST_8;

    _bc_ws2812b_dma_update.XferCpltCallback = _bc_ws2812b_dma_transfer_complete_handler;

    __HAL_LINKDMA(&_bc_ws2812b_timer2_handle, hdma[TIM_DMA_ID_UPDATE], _bc_ws2812b_dma_update);

    HAL_DMA_Init(&_bc_ws2812b_dma_update);

    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    HAL_DMA_Start_IT(&_bc_ws2812b_dma_update, (uint32_t) _bc_ws2812b.dma_bit_buffer, (uint32_t) &(TIM2->CCR2), dma_bit_buffer_size);

     // TIM2 Periph clock enable
    __HAL_RCC_TIM2_CLK_ENABLE();

    _bc_ws2812b_timer2_handle.Instance = TIM2;
    _bc_ws2812b_timer2_handle.Init.Period = _BC_WS2812_TIMER_PERIOD;
    _bc_ws2812b_timer2_handle.Init.Prescaler = 0x00;
    _bc_ws2812b_timer2_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    _bc_ws2812b_timer2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&_bc_ws2812b_timer2_handle);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    _bc_ws2812b_timer2_oc1.OCMode = TIM_OCMODE_PWM1;
    _bc_ws2812b_timer2_oc1.OCPolarity = TIM_OCPOLARITY_HIGH;
    _bc_ws2812b_timer2_oc1.Pulse = 0;
    _bc_ws2812b_timer2_oc1.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&_bc_ws2812b_timer2_handle, &_bc_ws2812b_timer2_oc1, TIM_CHANNEL_2);

    TIM2->CR1 &= ~TIM_CR1_CEN;

    TIM2->CCER |= (uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_2);

    //HAL_TIM_PWM_Start(&_bc_ws2812b_timer2_handle, TIM_CHANNEL_2);

    TIM2->DCR = TIM_DMABASE_CCR2 | TIM_DMABURSTLENGTH_1TRANSFER;

    _bc_ws2812b.task_id = bc_scheduler_register(_bc_ws2812b_task, NULL, BC_TICK_INFINITY);

    _bc_ws2812b.transfer = false;

    return true;
}

void bc_ws2812b_set_event_handler(void (*event_handler)(bc_ws2812b_event_t, void *), void *event_param)
{
    _bc_ws2812b.event_handler = event_handler;
    _bc_ws2812b.event_param = event_param;
}

void bc_ws2812b_set_pixel_from_rgb(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{

    uint32_t calculated_position = (position * _bc_ws2812b.buffer->type * 2);

    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(green & 0xf0) >> 4];
    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[green & 0x0f];

    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(red & 0xf0) >> 4];
    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[red & 0x0f];

    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(blue & 0xf0) >> 4];
    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[blue & 0x0f];

     if (_bc_ws2812b.buffer->type == BC_LED_STRIP_TYPE_RGBW)
     {
         _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(white & 0xf0) >> 4];
         _bc_ws2812b.dma_bit_buffer[calculated_position] = _bc_ws2812b_pulse_tab[white & 0x0f];
     }
}

void bc_ws2812b_set_pixel_from_uint32(int position, uint32_t color)
{
    uint32_t calculated_position = (position * _bc_ws2812b.buffer->type * 2);

    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(color & 0x00f00000) >> 20];
    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(color & 0x000f0000) >> 16];

    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(color & 0xf0000000) >> 28];
    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(color & 0x0f000000) >> 24];

    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(color & 0x0000f000) >> 12];
    _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(color & 0x00000f00) >>  8];

     if (_bc_ws2812b.buffer->type == BC_LED_STRIP_TYPE_RGBW)
     {
         _bc_ws2812b.dma_bit_buffer[calculated_position++] = _bc_ws2812b_pulse_tab[(color & 0x000000f0) >> 4];
         _bc_ws2812b.dma_bit_buffer[calculated_position] = _bc_ws2812b_pulse_tab[color & 0x0000000f];
     }
}

bool bc_ws2812b_write(void)
{
    if (_bc_ws2812b.transfer)
    {
        return false;
    }

    // transmission complete flag
    _bc_ws2812b.transfer = true;

    bc_scheduler_disable_sleep();

    HAL_TIM_Base_Stop(&_bc_ws2812b_timer2_handle);
    (&_bc_ws2812b_timer2_handle)->Instance->CR1 &= ~((0x1U << (0U)));

    // clear all DMA flags
    __HAL_DMA_CLEAR_FLAG(&_bc_ws2812b_dma_update, DMA_FLAG_TC2 | DMA_FLAG_HT2 | DMA_FLAG_TE2);

    // clear all TIM2 flags
    __HAL_TIM_CLEAR_FLAG(&_bc_ws2812b_timer2_handle, TIM_FLAG_UPDATE | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4);

    size_t dma_bit_buffer_size = _bc_ws2812b.buffer->count * _bc_ws2812b.buffer->type * 8;

    // configure the number of bytes to be transferred by the DMA controller
    _bc_ws2812b_dma_update.Instance->CNDTR = dma_bit_buffer_size;

    TIM2->CNT = _BC_WS2812_TIMER_PERIOD - 1;

    // Set zero length for first pulse because the first bit loads after first TIM_UP
    TIM2->CCR2 = 0;

    // Enable PWM Compare 2
    (&_bc_ws2812b_timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC2M_1;

    // enable DMA channels
    __HAL_DMA_ENABLE(&_bc_ws2812b_dma_update);

    // IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
    __HAL_TIM_ENABLE_DMA(&_bc_ws2812b_timer2_handle, TIM_DMA_UPDATE);

    // start TIM2
    TIM2->CR1 |= TIM_CR1_CEN;

    return true;
}

void _bc_ws2812b_dma_transfer_complete_handler(DMA_HandleTypeDef *dma_handle)
{
    (void)dma_handle;

    // Stop timer
    TIM2->CR1 &= ~TIM_CR1_CEN;

    // Disable DMA
    __HAL_DMA_DISABLE(&_bc_ws2812b_dma_update);
    // Disable the DMA requests
    __HAL_TIM_DISABLE_DMA(&_bc_ws2812b_timer2_handle, TIM_DMA_UPDATE);

    // Disable PWM output Compare 2
    (&_bc_ws2812b_timer2_handle)->Instance->CCMR1 &= ~(TIM_CCMR1_OC2M_Msk);
    (&_bc_ws2812b_timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC2M_2;

    // Set 50us period for Treset pulse
    TIM2->ARR = _BC_WS2812_TIMER_RESET_PULSE_PERIOD;
    // Reset the timer
    TIM2->CNT = 0;

    // Generate an update event to reload the prescaler value immediately
    TIM2->EGR = TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(&_bc_ws2812b_timer2_handle, TIM_FLAG_UPDATE);

    // Enable TIM2 Update interrupt for Treset signal
    __HAL_TIM_ENABLE_IT(&_bc_ws2812b_timer2_handle, TIM_IT_UPDATE);
    // Enable timer
    TIM2->CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&_bc_ws2812b_timer2_handle);
}

// TIM2 Interrupt Handler gets executed on every TIM2 Update if enabled
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    (void)htim;

    TIM2->CR1 = 0; // disable timer

    // disable the TIM2 Update IRQ
    __HAL_TIM_DISABLE_IT(&_bc_ws2812b_timer2_handle, TIM_IT_UPDATE);

    // Set back 1,25us period
    TIM2->ARR = _BC_WS2812_TIMER_PERIOD;

    // Generate an update event to reload the Prescaler value immediatly
    TIM2->EGR = TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(&_bc_ws2812b_timer2_handle, TIM_FLAG_UPDATE);

    // set transfer_complete flag
    _bc_ws2812b.transfer = false;

    bc_scheduler_enable_sleep();
    bc_scheduler_plan_now(_bc_ws2812b.task_id);
}

void DMA1_Channel2_3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&_bc_ws2812b_dma_update);
}

static void _bc_ws2812b_task(void *param)
{
    (void) param;

    if (_bc_ws2812b.event_handler != NULL)
    {
        _bc_ws2812b.event_handler(BC_WS2812B_SEND_DONE, _bc_ws2812b.event_param);
    }
}
