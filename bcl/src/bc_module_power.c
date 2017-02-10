#include "bc_module_power.h"
#include "stm32l0xx.h"

#include "usb_talk.h"
#include <bc_scheduler.h>

#define BIT_BUFFER_LEDS 2

void bc_module_power_task();

// WS2812 framebuffer - buffer for 2 LEDs - two times 32 bits
uint8_t dma_bit_buffer[8 * 4 * BIT_BUFFER_LEDS];
#define BUFFER_SIZE    (sizeof(dma_bit_buffer)/sizeof(uint8_t))

TIM_HandleTypeDef timer2_handle;
TIM_OC_InitTypeDef timer2_oc1;
TIM_OC_InitTypeDef timer2_oc2;

static uint32_t timer_period;
static uint32_t timer_reset_pulse_period;
static uint8_t compare_pulse_logic_0;
static uint8_t compare_pulse_logic_1;

// RGB Framebuffer
uint8_t frameBuffer[4 * BC_MODULE_POWER_MAX_LED_STRIP_COUNT];

ws2812b_t ws2812b;

bc_module_power_t bc_module_power;

static void dma_transfer_complete_handler(DMA_HandleTypeDef *dma_handle);
static void dma_transfer_half_handler(DMA_HandleTypeDef *dma_handle);
static void ws2812b_set_pixel(uint16_t column, uint8_t red, uint8_t green, uint8_t blue, uint8_t white);

// Gamma correction table
const uint8_t bc_module_power_gamma_table[] =
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25, 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68, 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89, 90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175, 177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213, 215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255 };

static void ws2812b_gpio_init(void)
{
    WS2812B_GPIO_CLK_ENABLE()
    ;

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = WS2812B_PINS;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(WS2812B_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

static void tim2_init(void)
{
    // TIM2 Periph clock enable
    __HAL_RCC_TIM2_CLK_ENABLE();

    // This computation of pulse length should work ok,
    // at some slower core speeds it needs some tuning.
    timer_period = SystemCoreClock / 800000; // 0,125us period (10 times lower the 1,25us period to have fixed math below)
    timer_reset_pulse_period = (SystemCoreClock / (320 * 60)); // 60us just to be sure

    uint32_t logic_0 = (10 * timer_period) / 36;
    uint32_t logic_1 = (10 * timer_period) / 15;

    if (logic_0 > 255 || logic_1 > 255)
    {
        // Error, compare_pulse_logic_0 or compare_pulse_logic_1 needs to be redefined to uint16_t (but it takes more memory)
        for (;;);
    }

    compare_pulse_logic_0 = logic_0;
    compare_pulse_logic_1 = logic_1;

    timer2_handle.Instance = TIM2;
    timer2_handle.Init.Period = timer_period;
    timer2_handle.Init.Prescaler = 0x00;
    timer2_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    timer2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&timer2_handle);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    timer2_oc1.OCMode = TIM_OCMODE_PWM1;
    timer2_oc1.OCPolarity = TIM_OCPOLARITY_HIGH;
    timer2_oc1.Pulse = compare_pulse_logic_0;
    timer2_oc1.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&timer2_handle, &timer2_oc1, TIM_CHANNEL_2);

    HAL_TIM_PWM_Start(&timer2_handle, TIM_CHANNEL_2);

    TIM2->DCR = TIM_DMABASE_CCR2 | TIM_DMABURSTLENGTH_1TRANSFER;

}

DMA_HandleTypeDef dmaUpdate;

static void dma_init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    dmaUpdate.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dmaUpdate.Init.PeriphInc = DMA_PINC_DISABLE;
    dmaUpdate.Init.MemInc = DMA_MINC_ENABLE;
    dmaUpdate.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dmaUpdate.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dmaUpdate.Init.Mode = DMA_CIRCULAR;
    dmaUpdate.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    dmaUpdate.Instance = DMA1_Channel2;
    dmaUpdate.Init.Request = DMA_REQUEST_8;

    dmaUpdate.XferCpltCallback = dma_transfer_complete_handler;
    dmaUpdate.XferHalfCpltCallback = dma_transfer_half_handler;

    __HAL_LINKDMA(&timer2_handle, hdma[TIM_DMA_ID_UPDATE], dmaUpdate);

    HAL_DMA_Init(&dmaUpdate);

    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    HAL_DMA_Start_IT(&dmaUpdate, (uint32_t) dma_bit_buffer, (uint32_t) &(TIM2->CCR2), BUFFER_SIZE);
}

void DMA1_Channel2_3_IRQHandler(void)
{
#ifdef WS2812B_DEBUG
    GPIOA->BSRR = GPIO_PIN_2;
#endif
    HAL_DMA_IRQHandler(&dmaUpdate);
#ifdef WS2812B_DEBUG
    GPIOA->BRR = GPIO_PIN_2;
#endif
}

static void load_next_framebuffer_data(ws2812b_buffer_item_t *buffer_item, uint32_t row)
{
    uint8_t r = buffer_item->frame_buffer_pointer[buffer_item->frame_buffer_counter++];
    uint8_t g = buffer_item->frame_buffer_pointer[buffer_item->frame_buffer_counter++];
    uint8_t b = buffer_item->frame_buffer_pointer[buffer_item->frame_buffer_counter++];
    uint8_t w = 0;
    if (bc_module_power.led_strip_mode == BC_MODULE_POWER_RGBW)
    {
        w = buffer_item->frame_buffer_pointer[buffer_item->frame_buffer_counter++];
    }

    if (buffer_item->frame_buffer_counter >= buffer_item->frame_buffer_size)
    {
        buffer_item->frame_buffer_counter = 0;
    }

    ws2812b_set_pixel(row, r, g, b, w);
}

// Transmit the framebuffer
static void ws2812b_send()
{
    // transmission complete flag
    ws2812b.transfer_complete = 0;

    uint32_t i;
    uint32_t j;

    for (i = 0; i < WS2812_BUFFER_COUNT; i++)
    {
        ws2812b.item[i].frame_buffer_counter = 0;
        ws2812b.item[i].frame_buffer_size = bc_module_power.led_strip_mode * bc_module_power.led_strip_count;

        for (j = 0; j < BIT_BUFFER_LEDS; j++)
        {
            load_next_framebuffer_data(&ws2812b.item[i], j); // ROW j
        }
    }

    HAL_TIM_Base_Stop(&timer2_handle);
    (&timer2_handle)->Instance->CR1 &= ~((0x1U << (0U)));

    // clear all DMA flags
    __HAL_DMA_CLEAR_FLAG(&dmaUpdate, DMA_FLAG_TC2 | DMA_FLAG_HT2 | DMA_FLAG_TE2);

    // configure the number of bytes to be transferred by the DMA controller
    dmaUpdate.Instance->CNDTR = 8 * bc_module_power.led_strip_mode * BIT_BUFFER_LEDS;

    // clear all TIM2 flags
    __HAL_TIM_CLEAR_FLAG(&timer2_handle, TIM_FLAG_UPDATE | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4);

    // enable DMA channels
    __HAL_DMA_ENABLE(&dmaUpdate);

    // IMPORTANT: enable the TIM2 DMA requests AFTER enabling the DMA channels!
    __HAL_TIM_ENABLE_DMA(&timer2_handle, TIM_DMA_UPDATE);

    TIM2->CNT = timer_period - 1;

    // Set zero length for first pulse because the first bit loads after first TIM_UP
    TIM2->CCR2 = 0;

    // Enable PWM Compare 1
    //(&timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC1M_1;
    // Enable PWM Compare 2
    (&timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC2M_1;

    __HAL_DBGMCU_FREEZE_TIM2();

    // start TIM2
    __HAL_TIM_ENABLE(&timer2_handle);
}

void dma_transfer_half_handler(DMA_HandleTypeDef *dma_handle)
{
    (void)dma_handle;

    // Is this the last LED?
    if (ws2812b.repeat_counter != (bc_module_power.led_strip_count / 2 - 1))
    {
        uint32_t i;
        uint32_t j;

        for (i = 0; i < WS2812_BUFFER_COUNT; i++)
        {
            // Load first half of the bit buffer
            for (j = 0; j < (BIT_BUFFER_LEDS / 2); j++)
            {
                load_next_framebuffer_data(&ws2812b.item[i], j);
            }
        }
    }
    else
    {
        // If this is the last pixel, set the next pixel value to zeros, because
        // the DMA would not stop exactly at the last bit.
        ws2812b_set_pixel(0, 0, 0, 0, 0);
    }

}

void dma_transfer_complete_handler(DMA_HandleTypeDef *dma_handle)
{
    (void)dma_handle;

    ws2812b.repeat_counter++;

    if (ws2812b.repeat_counter == bc_module_power.led_strip_count / 2)
    {
        // Transfer of all LEDs is done, disable DMA but enable tiemr update IRQ to stop the 50us pulse
        ws2812b.repeat_counter = 0;



        // Stop timer
        TIM2->CR1 &= ~TIM_CR1_CEN;

        // Disable DMA
        __HAL_DMA_DISABLE(&dmaUpdate);
        // Disable the DMA requests
        __HAL_TIM_DISABLE_DMA(&timer2_handle, TIM_DMA_UPDATE);

        // Disable PWM output compare 1
        //(&timer2_handle)->Instance->CCMR1 &= ~(TIM_CCMR1_OC1M_Msk);
        //(&timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC1M_2;

        // Disable PWM output Compare 2
        (&timer2_handle)->Instance->CCMR1 &= ~(TIM_CCMR1_OC2M_Msk);
        (&timer2_handle)->Instance->CCMR1 |= TIM_CCMR1_OC2M_2;

        // Set 50us period for Treset pulse
        //TIM2->PSC = 1000; // For this long period we need prescaler 1000
        TIM2->ARR = timer_reset_pulse_period;
        // Reset the timer
        TIM2->CNT = 0;

        // Generate an update event to reload the prescaler value immediately
        TIM2->EGR = TIM_EGR_UG;
        __HAL_TIM_CLEAR_FLAG(&timer2_handle, TIM_FLAG_UPDATE);

        // Enable TIM2 Update interrupt for Treset signal
        __HAL_TIM_ENABLE_IT(&timer2_handle, TIM_IT_UPDATE);
        // Enable timer
        TIM2->CR1 |= TIM_CR1_CEN;

    }
    else
    {

        // Load bitbuffer with next RGB LED values
        uint32_t i;
        uint32_t j;
        for (i = 0; i < WS2812_BUFFER_COUNT; i++)
        {
            for (j = (BIT_BUFFER_LEDS / 2); j < BIT_BUFFER_LEDS; j++)
            {
                load_next_framebuffer_data(&ws2812b.item[i], j);
            }
        }

    }

}

void TIM2_IRQHandler(void)
{
#ifdef WS2812B_DEBUG
    GPIOA->BSRR = GPIO_PIN_2;
#endif
    HAL_TIM_IRQHandler(&timer2_handle);
#ifdef WS2812B_DEBUG
    GPIOA->BRR = GPIO_PIN_2;
#endif
}

// TIM2 Interrupt Handler gets executed on every TIM2 Update if enabled
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    (void)htim;

    ws2812b.timer_period_counter = 0;
    TIM2->CR1 = 0; // disable timer

    // disable the TIM2 Update IRQ
    __HAL_TIM_DISABLE_IT(&timer2_handle, TIM_IT_UPDATE);

    // Set back 1,25us period
    TIM2->ARR = timer_period;

    // Generate an update event to reload the Prescaler value immediatly
    TIM2->EGR = TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(&timer2_handle, TIM_FLAG_UPDATE);

    // set transfer_complete flag
    ws2812b.transfer_complete = 1;

}

static void ws2812b_set_pixel(uint16_t column, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
    red = bc_module_power_gamma_table[red];
    green = bc_module_power_gamma_table[green];
    blue = bc_module_power_gamma_table[blue];
    white = bc_module_power_gamma_table[white];

    uint32_t calculated_column = (column * bc_module_power.led_strip_mode * 8);

    uint8_t *bit_buffer_offset = &dma_bit_buffer[calculated_column];

    *bit_buffer_offset++ = (green & 0x80) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (green & 0x40) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (green & 0x20) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (green & 0x10) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (green & 0x08) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (green & 0x04) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (green & 0x02) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (green & 0x01) ? compare_pulse_logic_1 : compare_pulse_logic_0;

    *bit_buffer_offset++ = (red & 0x80) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (red & 0x40) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (red & 0x20) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (red & 0x10) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (red & 0x08) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (red & 0x04) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (red & 0x02) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (red & 0x01) ? compare_pulse_logic_1 : compare_pulse_logic_0;

    *bit_buffer_offset++ = (blue & 0x80) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (blue & 0x40) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (blue & 0x20) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (blue & 0x10) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (blue & 0x08) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (blue & 0x04) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (blue & 0x02) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    *bit_buffer_offset++ = (blue & 0x01) ? compare_pulse_logic_1 : compare_pulse_logic_0;

    if (bc_module_power.led_strip_mode == BC_MODULE_POWER_RGBW)
    {
        *bit_buffer_offset++ = (white & 0x80) ? compare_pulse_logic_1 : compare_pulse_logic_0;
        *bit_buffer_offset++ = (white & 0x40) ? compare_pulse_logic_1 : compare_pulse_logic_0;
        *bit_buffer_offset++ = (white & 0x20) ? compare_pulse_logic_1 : compare_pulse_logic_0;
        *bit_buffer_offset++ = (white & 0x10) ? compare_pulse_logic_1 : compare_pulse_logic_0;
        *bit_buffer_offset++ = (white & 0x08) ? compare_pulse_logic_1 : compare_pulse_logic_0;
        *bit_buffer_offset++ = (white & 0x04) ? compare_pulse_logic_1 : compare_pulse_logic_0;
        *bit_buffer_offset++ = (white & 0x02) ? compare_pulse_logic_1 : compare_pulse_logic_0;
        *bit_buffer_offset++ = (white & 0x01) ? compare_pulse_logic_1 : compare_pulse_logic_0;
    }

}

void bc_module_power_init()
{
    memset(&bc_module_power, 0, sizeof(bc_module_power_t));

    bc_module_power.led_strip_on = false;
    bc_module_power.led_strip_count = 144;
    bc_module_power.led_strip_mode = BC_MODULE_POWER_RGBW;

    ws2812b_gpio_init();
    dma_init();
    tim2_init();

    ws2812b.item[0].frame_buffer_pointer = frameBuffer;
    ws2812b.item[0].frame_buffer_size = bc_module_power.led_strip_count * bc_module_power.led_strip_mode;

    // Need to start the first transfer
    ws2812b.transfer_complete = 1;

    /* Enable clock for GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    /* Output on PA0 pin */
    GPIOA->MODER &= ~GPIO_MODER_MODE0;
    GPIOA->MODER |= GPIO_MODER_MODE0_0;

    bc_scheduler_register(bc_module_power_task, &bc_module_power, 10);
}

void bc_module_power_task()
{
    if (ws2812b.transfer_complete)
    {
        // Update your framebuffer here or swap buffers
        // Signal that buffer is changed and transfer new data

        ws2812b.start_transfer = 1;
    }

    if (ws2812b.start_transfer)
    {
        ws2812b.start_transfer = 0;
        ws2812b_send();
    }

    if (bc_module_power.relay_is_on)
    {
        GPIOA->BSRR = GPIO_BSRR_BS_0;
    }
    else
    {
        GPIOA->BSRR = GPIO_BSRR_BR_0;
    }

    bc_scheduler_plan_current_relative(10);
}
