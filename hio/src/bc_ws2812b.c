#include "stm32l0xx.h"
#include <hio_ws2812b.h>
#include <hio_scheduler.h>
#include <hio_dma.h>
#include <hio_system.h>
#include <hio_timer.h>

#define _HIO_WS2812_TIMER_PERIOD 40               // 32000000 / 800000 = 20; 0,125us period (10 times lower the 1,25us period to have fixed math below)
#define _HIO_WS2812_TIMER_RESET_PULSE_PERIOD 1666 // 60us just to be sure = (32000000 / (320 * 60))
#define _HIO_WS2812_COMPARE_PULSE_LOGIC_0     11  //(10 * timer_period) / 36
#define _HIO_WS2812_COMPARE_PULSE_LOGIC_1     26  //(10 * timer_period) / 15;

#define _HIO_WS2812_HIO_WS2812_RESET_PERIOD 100
#define _HIO_WS2812_HIO_WS2812B_PORT GPIOA
#define _HIO_WS2812_HIO_WS2812B_PIN GPIO_PIN_1

static struct ws2812b_t
{
    uint32_t *dma_bit_buffer;
    const hio_led_strip_buffer_t *buffer;

    bool transfer;
    hio_scheduler_task_id_t task_id;
    void (*event_handler)(hio_ws2812b_event_t, void *);
    void *event_param;

} _hio_ws2812b;

static hio_dma_channel_config_t _hio_ws2812b_dma_config =
{
    .request = HIO_DMA_REQUEST_8,
    .direction = HIO_DMA_DIRECTION_TO_PERIPHERAL,
    .data_size_memory = HIO_DMA_SIZE_1,
    .data_size_peripheral = HIO_DMA_SIZE_2,
    .mode = HIO_DMA_MODE_STANDARD,
    .address_peripheral = (void *)&(TIM2->CCR2),
    .priority = HIO_DMA_PRIORITY_VERY_HIGH
};

TIM_HandleTypeDef _hio_ws2812b_timer2_handle;
TIM_OC_InitTypeDef _hio_ws2812b_timer2_oc1;

const uint32_t _hio_ws2812b_pulse_tab[] =
{
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_0 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
    _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 24 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 16 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1 << 8 | _HIO_WS2812_COMPARE_PULSE_LOGIC_1,
};

static void _hio_ws2812b_dma_event_handler(hio_dma_channel_t channel, hio_dma_event_t event, void *event_param);
static void _hio_ws2812b_TIM2_interrupt_handler(void *param);
static void _hio_ws2812b_task(void *param);

bool hio_ws2812b_init(const hio_led_strip_buffer_t *led_strip)
{
    memset(&_hio_ws2812b, 0, sizeof(_hio_ws2812b));

    _hio_ws2812b.buffer = led_strip;

    _hio_ws2812b.dma_bit_buffer = led_strip->buffer;

    size_t dma_bit_buffer_size = _hio_ws2812b.buffer->count * _hio_ws2812b.buffer->type * 8;

    memset(_hio_ws2812b.dma_bit_buffer, _HIO_WS2812_COMPARE_PULSE_LOGIC_0, dma_bit_buffer_size);

    __HAL_RCC_GPIOA_CLK_ENABLE();

    //Init pin
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = _HIO_WS2812_HIO_WS2812B_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(_HIO_WS2812_HIO_WS2812B_PORT, &GPIO_InitStruct);

    hio_dma_init();
    hio_dma_set_event_handler(HIO_DMA_CHANNEL_2, _hio_ws2812b_dma_event_handler, NULL);

     // TIM2 Periph clock enable
    __HAL_RCC_TIM2_CLK_ENABLE();

    _hio_ws2812b_timer2_handle.Instance = TIM2;
    _hio_ws2812b_timer2_handle.Init.Period = _HIO_WS2812_TIMER_PERIOD;
    _hio_ws2812b_timer2_handle.Init.Prescaler = 0x00;
    _hio_ws2812b_timer2_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    _hio_ws2812b_timer2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&_hio_ws2812b_timer2_handle);

    hio_timer_set_irq_handler(TIM2, _hio_ws2812b_TIM2_interrupt_handler, NULL);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    _hio_ws2812b_timer2_oc1.OCMode = TIM_OCMODE_PWM1;
    _hio_ws2812b_timer2_oc1.OCPolarity = TIM_OCPOLARITY_HIGH;
    _hio_ws2812b_timer2_oc1.Pulse = 0;
    _hio_ws2812b_timer2_oc1.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&_hio_ws2812b_timer2_handle, &_hio_ws2812b_timer2_oc1, TIM_CHANNEL_2);

    TIM2->CR1 &= ~TIM_CR1_CEN;

    TIM2->CCER |= (uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_2);

    //HAL_TIM_PWM_Start(&_hio_ws2812b_timer2_handle, TIM_CHANNEL_2);

    TIM2->DCR = TIM_DMABASE_CCR2 | TIM_DMABURSTLENGTH_1TRANSFER;

    _hio_ws2812b.task_id = hio_scheduler_register(_hio_ws2812b_task, NULL, HIO_TICK_INFINITY);

    _hio_ws2812b.transfer = false;

    return true;
}

void hio_ws2812b_set_event_handler(void (*event_handler)(hio_ws2812b_event_t, void *), void *event_param)
{
    _hio_ws2812b.event_handler = event_handler;
    _hio_ws2812b.event_param = event_param;
}

void hio_ws2812b_set_pixel_from_rgb(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    uint32_t calculated_position = (position * _hio_ws2812b.buffer->type * 2);

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(green & 0xf0) >> 4];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[green & 0x0f];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(red & 0xf0) >> 4];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[red & 0x0f];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(blue & 0xf0) >> 4];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[blue & 0x0f];

     if (_hio_ws2812b.buffer->type == HIO_LED_STRIP_TYPE_RGBW)
     {
         _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(white & 0xf0) >> 4];
         _hio_ws2812b.dma_bit_buffer[calculated_position] = _hio_ws2812b_pulse_tab[white & 0x0f];
     }
}

void hio_ws2812b_set_pixel_from_uint32(int position, uint32_t color)
{
    uint32_t calculated_position = (position * _hio_ws2812b.buffer->type * 2);

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x00f00000) >> 20];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x000f0000) >> 16];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0xf0000000) >> 28];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x0f000000) >> 24];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x0000f000) >> 12];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x00000f00) >>  8];

     if (_hio_ws2812b.buffer->type == HIO_LED_STRIP_TYPE_RGBW)
     {
         _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x000000f0) >> 4];
         _hio_ws2812b.dma_bit_buffer[calculated_position] = _hio_ws2812b_pulse_tab[color & 0x0000000f];
     }
}

void hio_ws2812b_set_pixel_from_rgb_swap_rg(int position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    uint32_t calculated_position = (position * _hio_ws2812b.buffer->type * 2);

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(red & 0xf0) >> 4];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[red & 0x0f];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(green & 0xf0) >> 4];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[green & 0x0f];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(blue & 0xf0) >> 4];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[blue & 0x0f];

     if (_hio_ws2812b.buffer->type == HIO_LED_STRIP_TYPE_RGBW)
     {
         _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(white & 0xf0) >> 4];
         _hio_ws2812b.dma_bit_buffer[calculated_position] = _hio_ws2812b_pulse_tab[white & 0x0f];
     }
}

void hio_ws2812b_set_pixel_from_uint32_swap_rg(int position, uint32_t color)
{
    uint32_t calculated_position = (position * _hio_ws2812b.buffer->type * 2);

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0xf0000000) >> 28];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x0f000000) >> 24];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x00f00000) >> 20];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x000f0000) >> 16];

    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x0000f000) >> 12];
    _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x00000f00) >>  8];

     if (_hio_ws2812b.buffer->type == HIO_LED_STRIP_TYPE_RGBW)
     {
         _hio_ws2812b.dma_bit_buffer[calculated_position++] = _hio_ws2812b_pulse_tab[(color & 0x000000f0) >> 4];
         _hio_ws2812b.dma_bit_buffer[calculated_position] = _hio_ws2812b_pulse_tab[color & 0x0000000f];
     }
}

bool hio_ws2812b_write(void)
{
    if (_hio_ws2812b.transfer)
    {
        return false;
    }

    // transmission complete flag
    _hio_ws2812b.transfer = true;

    hio_system_pll_enable();

    HAL_TIM_Base_Stop(&_hio_ws2812b_timer2_handle);
    (&_hio_ws2812b_timer2_handle)->Instance->CR1 &= ~((0x1U << (0U)));

    // clear all DMA flags
    __HAL_DMA_CLEAR_FLAG(&_hio_ws2812b_dma_update, DMA_FLAG_TC2 | DMA_FLAG_HT2 | DMA_FLAG_TE2);

    // clear all TIM2 flags
    __HAL_TIM_CLEAR_FLAG(&_hio_ws2812b_timer2_handle, TIM_FLAG_UPDATE | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4);

    size_t dma_bit_buffer_size = _hio_ws2812b.buffer->count * _hio_ws2812b.buffer->type * 8;

    _hio_ws2812b_dma_config.address_memory = (void *)_hio_ws2812b.dma_bit_buffer;
    _hio_ws2812b_dma_config.length = dma_bit_buffer_size;
    hio_dma_channel_config(HIO_DMA_CHANNEL_2, &_hio_ws2812b_dma_config);
    hio_dma_channel_run(HIO_DMA_CHANNEL_2);

    TIM2->CNT = _HIO_WS2812_TIMER_PERIOD - 1;

    // Set zero length for first pulse because the first bit loads after first TIM_UP
    TIM2->CCR2 = 0;

    // Enable PWM Compare 2
    (&_hio_ws2812b_timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC2M_1;

    // IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
    __HAL_TIM_ENABLE_DMA(&_hio_ws2812b_timer2_handle, TIM_DMA_UPDATE);

    // start TIM2
    TIM2->CR1 |= TIM_CR1_CEN;

    return true;
}

bool hio_ws2812b_is_ready(void)
{
    return !_hio_ws2812b.transfer;
}

static void _hio_ws2812b_dma_event_handler(hio_dma_channel_t channel, hio_dma_event_t event, void *event_param)
{
    (void) channel;
    (void) event;
    (void) event_param;

    if (event == HIO_DMA_EVENT_DONE)
    {
        // Stop timer
        TIM2->CR1 &= ~TIM_CR1_CEN;

        // Disable the DMA requests
        __HAL_TIM_DISABLE_DMA(&_hio_ws2812b_timer2_handle, TIM_DMA_UPDATE);

        // Disable PWM output Compare 2
        (&_hio_ws2812b_timer2_handle)->Instance->CCMR1 &= ~(TIM_CCMR1_OC2M_Msk);
        (&_hio_ws2812b_timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC2M_2;

        // Set 50us period for Treset pulse
        TIM2->ARR = _HIO_WS2812_TIMER_RESET_PULSE_PERIOD;
        // Reset the timer
        TIM2->CNT = 0;

        // Generate an update event to reload the prescaler value immediately
        TIM2->EGR = TIM_EGR_UG;
        __HAL_TIM_CLEAR_FLAG(&_hio_ws2812b_timer2_handle, TIM_FLAG_UPDATE);

        // Enable TIM2 Update interrupt for Treset signal
        __HAL_TIM_ENABLE_IT(&_hio_ws2812b_timer2_handle, TIM_IT_UPDATE);
        // Enable timer
        TIM2->CR1 |= TIM_CR1_CEN;
    }
}

// TIM2 Interrupt Handler gets executed on every TIM2 Update if enabled
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    (void)htim;

    TIM2->CR1 = 0; // disable timer

    // disable the TIM2 Update IRQ
    __HAL_TIM_DISABLE_IT(&_hio_ws2812b_timer2_handle, TIM_IT_UPDATE);

    // Set back 1,25us period
    TIM2->ARR = _HIO_WS2812_TIMER_PERIOD;

    // Generate an update event to reload the Prescaler value immediatly
    TIM2->EGR = TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(&_hio_ws2812b_timer2_handle, TIM_FLAG_UPDATE);

    hio_scheduler_plan_now(_hio_ws2812b.task_id);
}

static void _hio_ws2812b_TIM2_interrupt_handler(void *param)
{
    (void) param;

    HAL_TIM_IRQHandler(&_hio_ws2812b_timer2_handle);
}

static void _hio_ws2812b_task(void *param)
{
    (void) param;

    hio_system_pll_disable();

    // set transfer_complete flag
    _hio_ws2812b.transfer = false;

    if (_hio_ws2812b.event_handler != NULL)
    {
        _hio_ws2812b.event_handler(HIO_WS2812B_SEND_DONE, _hio_ws2812b.event_param);
    }
}
