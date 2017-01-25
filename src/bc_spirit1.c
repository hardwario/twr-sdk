#include <bc_spirit1.h>
#include <bc_scheduler.h>
#include <stm32l0xx.h>
#include "SPIRIT_Config.h"
#include "SDK_Configuration_Common.h"
#include "MCU_Interface.h"

typedef enum
{
    BC_SPIRIT1_STATE_SLEEP = 0,
    BC_SPIRIT1_STATE_TX = 1,
    BC_SPIRIT1_STATE_RX = 2

} bc_spirit1_state_t;

typedef struct
{
    void (*event_handler)(bc_spirit1_event_t);
    bc_scheduler_task_id_t task_id;
    bc_spirit1_state_t desired_state;
    bc_spirit1_state_t current_state;
    uint8_t tx_buffer[BC_SPIRIT1_MAX_PACKET_SIZE];
    size_t tx_length;
    uint8_t  rx_buffer[BC_SPIRIT1_MAX_PACKET_SIZE];
    size_t rx_length;
    bc_tick_t rx_timeout;
    bc_tick_t rx_tick_timeout;

} bc_spirit1_t;

static bc_spirit1_t _bc_spirit1;

#define XTAL_FREQUENCY 50000000

SRadioInit xRadioInit = {
  XTAL_OFFSET_PPM,
  BASE_FREQUENCY,
  CHANNEL_SPACE,
  CHANNEL_NUMBER,
  MODULATION_SELECT,
  DATARATE,
  FREQ_DEVIATION,
  BANDWIDTH
};

PktBasicInit xBasicInit={
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
  LENGTH_TYPE,
  LENGTH_WIDTH,
  CRC_MODE,
  CONTROL_LENGTH,
  EN_ADDRESS,
  EN_FEC,
  EN_WHITENING
};

PktBasicAddressesInit xAddressInit={
  EN_FILT_MY_ADDRESS,
  MY_ADDRESS,
  EN_FILT_MULTICAST_ADDRESS,
  MULTICAST_ADDRESS,
  EN_FILT_BROADCAST_ADDRESS,
  BROADCAST_ADDRESS
};

SGpioInit xGpioIRQ={
  SPIRIT_GPIO_0,
  SPIRIT_GPIO_MODE_DIGITAL_OUTPUT_LP,
  SPIRIT_GPIO_DIG_OUT_IRQ
};


static bc_tick_t _bc_spirit1_enter_state_tx(void);
static bc_tick_t _bc_spirit1_check_state_tx(void);
static bc_tick_t _bc_spirit1_enter_state_rx(void);
static bc_tick_t _bc_spirit1_check_state_rx(void);
static bc_tick_t _bc_spirit1_enter_state_sleep(void);

void bc_spirit1_hal_chip_select_low(void);
void bc_spirit1_hal_chip_select_high(void);
uint8_t bc_spirit1_hal_transfer_byte(uint8_t value);
static void bc_spirit1_hal_init_gpio(void);
static void bc_spirit1_hal_init_spi(void);
static void bc_spirit1_hal_init_timer(void);

static bc_tick_t _bc_spirit1_task(void *param, bc_tick_t tick_now);

void EXTI4_15_IRQHandler(void)
{
    // Clear pending interrupt flag for line 7
    EXTI->PR = EXTI_PR_PIF7;

    bc_scheduler_plan_now(_bc_spirit1.task_id);
}

void bc_spirit1_init(void)
{
    memset(&_bc_spirit1, 0, sizeof(_bc_spirit1));

    SpiritRadioSetXtalFrequency(XTAL_FREQUENCY);
    SpiritSpiInit();

    /* Spirit ON */
    SpiritEnterShutdown();
    SpiritExitShutdown();
    SpiritManagementWaExtraCurrent();

    /* Spirit IRQ config */
    SpiritGpioInit(&xGpioIRQ);

    /* Spirit Radio config */
    SpiritRadioInit(&xRadioInit);

    /* Spirit Packet config */
    SpiritPktBasicInit(&xBasicInit);
    SpiritPktBasicAddressesInit(&xAddressInit);

    _bc_spirit1.task_id = bc_scheduler_register(_bc_spirit1_task, NULL, BC_TICK_INFINITY);
}

void bc_spirit1_set_event_handler(void (*event_handler)(bc_spirit1_event_t))
{
    _bc_spirit1.event_handler = event_handler;
}

void *bc_spirit1_get_tx_buffer(void)
{
    return _bc_spirit1.tx_buffer;
}

void bc_spirit1_set_tx_length(size_t length)
{
    _bc_spirit1.tx_length = length;
}

void *bc_spirit1_get_rx_buffer(void)
{
    return _bc_spirit1.rx_buffer;
}

size_t bc_spirit1_get_rx_length(void)
{
    return _bc_spirit1.rx_length;
}

void bc_spirit1_set_rx_timeout(bc_tick_t timeout)
{
    _bc_spirit1.rx_timeout = timeout;
}

void bc_spirit1_tx(void)
{
    _bc_spirit1.desired_state = BC_SPIRIT1_STATE_TX;

    bc_scheduler_plan_now(_bc_spirit1.task_id);
}

void bc_spirit1_rx(void)
{
    _bc_spirit1.desired_state = BC_SPIRIT1_STATE_RX;

    bc_scheduler_plan_now(_bc_spirit1.task_id);
}

void bc_spirit1_sleep(void)
{
    _bc_spirit1.desired_state = BC_SPIRIT1_STATE_SLEEP;

    bc_scheduler_plan_now(_bc_spirit1.task_id);
}

static bc_tick_t _bc_spirit1_task(void *param, bc_tick_t tick_now)
{
    (void) param;
    (void) tick_now;

    if (_bc_spirit1.desired_state != _bc_spirit1.current_state)
    {
        if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_TX)
        {
            return _bc_spirit1_enter_state_tx();
        }
        else if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_RX)
        {
            return _bc_spirit1_enter_state_rx();
        }
        else if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_SLEEP)
        {
            return _bc_spirit1_enter_state_sleep();
        }

        return BC_TICK_INFINITY;
    }

    if (_bc_spirit1.current_state == BC_SPIRIT1_STATE_TX)
    {
        return _bc_spirit1_check_state_tx();
    }
    else if (_bc_spirit1.current_state == BC_SPIRIT1_STATE_RX)
    {
        return _bc_spirit1_check_state_rx();
    }

    return BC_TICK_INFINITY;
}

static bc_tick_t _bc_spirit1_enter_state_tx(void)
{
    _bc_spirit1.current_state = BC_SPIRIT1_STATE_TX;

    SpiritCmdStrobeSabort();
    SpiritCmdStrobeReady();
    SpiritCmdStrobeFlushTxFifo();

    SpiritIrqDeInit(NULL);
    SpiritIrqClearStatus();
    SpiritIrq(TX_DATA_SENT, S_ENABLE);

    SpiritPktBasicSetPayloadLength(_bc_spirit1.tx_length);

    // TODO Why needed?
    SpiritPktBasicSetDestinationAddress(0x35);

    SpiritSpiWriteLinearFifo(_bc_spirit1.tx_length, _bc_spirit1.tx_buffer);

    // Interrupt request for EXTI line 7 is masked
    EXTI->IMR &= ~EXTI_IMR_IM7;

    // Clear pending interrupt flag for EXTI line 7
    EXTI->PR = EXTI_PR_PIF7;

    // Falling trigger for EXTI line 7 is enabled
    EXTI->FTSR |= EXTI_FTSR_FT7;

    // Interrupt request for EXTI line 7 is not masked
    EXTI->IMR |= EXTI_IMR_IM7;

    NVIC_EnableIRQ(EXTI4_15_IRQn);

    SpiritCmdStrobeTx();

    return BC_TICK_INFINITY;
}

static bc_tick_t _bc_spirit1_check_state_tx(void)
{
    SpiritIrqs xIrqStatus;

    SpiritIrqGetStatus(&xIrqStatus);

    if (xIrqStatus.IRQ_TX_DATA_SENT)
    {
        SpiritIrqClearStatus();

        _bc_spirit1.desired_state = BC_SPIRIT1_STATE_SLEEP;

        if (_bc_spirit1.event_handler != NULL)
        {
            _bc_spirit1.event_handler(BC_SPIRIT1_EVENT_TX_DONE);
        }

        if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_RX)
        {
            return _bc_spirit1_enter_state_rx();
        }
        else if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_SLEEP)
        {
            return _bc_spirit1_enter_state_sleep();
        }
    }

    return BC_TICK_INFINITY;
}

static bc_tick_t _bc_spirit1_enter_state_rx(void)
{
    _bc_spirit1.current_state = BC_SPIRIT1_STATE_RX;

    if (_bc_spirit1.rx_timeout == 0)
    {
        _bc_spirit1.rx_tick_timeout = BC_TICK_INFINITY;
    }
    else
    {
        _bc_spirit1.rx_tick_timeout = bc_tick_get() + _bc_spirit1.rx_timeout;
    }

    SpiritIrqs xIrqStatus;

    SpiritIrqDeInit(&xIrqStatus);
    SpiritIrq(RX_DATA_DISC, S_ENABLE);
    SpiritIrq(RX_DATA_READY, S_ENABLE);

    /* payload length config */
    SpiritPktBasicSetPayloadLength(20);

    /* enable SQI check */
    SpiritQiSetSqiThreshold(SQI_TH_0);
    SpiritQiSqiCheck(S_ENABLE);

    /* RX timeout config */
    SpiritTimerSetRxTimeoutMs(1000.0);
    SpiritTimerSetRxTimeoutStopCondition(SQI_ABOVE_THRESHOLD);

    /* IRQ registers blanking */
    SpiritIrqClearStatus();

    // Interrupt request for EXTI line 7 is masked
    EXTI->IMR &= ~EXTI_IMR_IM7;

    // Clear pending interrupt flag for EXTI line 7
    EXTI->PR = EXTI_PR_PIF7;

    // Falling trigger for EXTI line 7 is enabled
    EXTI->FTSR |= EXTI_FTSR_FT7;

    // Interrupt request for EXTI line 7 is not masked
    EXTI->IMR |= EXTI_IMR_IM7;

    NVIC_EnableIRQ(EXTI4_15_IRQn);

    /* RX command */
    SpiritCmdStrobeRx();

    return _bc_spirit1.rx_tick_timeout;
}

static bc_tick_t _bc_spirit1_check_state_rx(void)
{
    if (bc_tick_get() >= _bc_spirit1.rx_tick_timeout)
    {
        if (_bc_spirit1.event_handler != NULL)
        {
            _bc_spirit1.event_handler(BC_SPIRIT1_EVENT_RX_TIMEOUT);
        }
    }

    SpiritIrqs xIrqStatus;

    /* Get the IRQ status */
    SpiritIrqGetStatus(&xIrqStatus);

    /* Check the SPIRIT RX_DATA_DISC IRQ flag */
    if (xIrqStatus.IRQ_RX_DATA_DISC)
    {
      /* RX command - to ensure the device will be ready for the next reception */
      SpiritCmdStrobeRx();
    }

    /* Check the SPIRIT RX_DATA_READY IRQ flag */
    if (xIrqStatus.IRQ_RX_DATA_READY)
    {
      /* Get the RX FIFO size */
      uint8_t cRxData = SpiritLinearFifoReadNumElementsRxFifo();

      /* Read the RX FIFO */
      SpiritSpiReadLinearFifo(cRxData, _bc_spirit1.rx_buffer);

      _bc_spirit1.rx_length = cRxData;

      if (_bc_spirit1.event_handler != NULL)
      {
          _bc_spirit1.event_handler(BC_SPIRIT1_EVENT_RX_DONE);
      }
    }

    /* Flush the RX FIFO */
    SpiritCmdStrobeFlushRxFifo();

    /* RX command - to ensure the device will be ready for the next reception */
    SpiritCmdStrobeRx();

    return BC_TICK_INFINITY;
}

static bc_tick_t _bc_spirit1_enter_state_sleep(void)
{
    _bc_spirit1.current_state = BC_SPIRIT1_STATE_SLEEP;

    SpiritCmdStrobeSabort();
    SpiritCmdStrobeReady();
    SpiritIrqDeInit(NULL);
    SpiritIrqClearStatus();
    SpiritCmdStrobeStandby();

    return BC_TICK_INFINITY;
}

bc_spirit_status_t bc_spirit1_command(uint8_t command)
{
    // Set chip select low
    bc_spirit1_hal_chip_select_low();

    // Write header byte and read status bits (MSB)
    uint16_t status = bc_spirit1_hal_transfer_byte(0x80) << 8;

    // Write memory map address and read status bits (LSB)
    status |= bc_spirit1_hal_transfer_byte(command);

    // Set chip select high
    bc_spirit1_hal_chip_select_high();

    // TODO Why this cast?
    return *((bc_spirit_status_t *) &status);
}

bc_spirit_status_t bc_spirit1_write(uint8_t address, const void *buffer, size_t length)
{
    // Set chip select low
    bc_spirit1_hal_chip_select_low();

    // Write header byte and read status bits (MSB)
    uint16_t status = bc_spirit1_hal_transfer_byte(0) << 8;

    // Write memory map address and read status bits (LSB)
    status |= bc_spirit1_hal_transfer_byte(address);

    // Write buffer
    for (size_t i = 0; i < length; i++)
    {
        // Write data
        bc_spirit1_hal_transfer_byte(*((uint8_t *) buffer + i));
    }

    // Set chip select high
    bc_spirit1_hal_chip_select_high();

    // TODO Why this cast?
    return *((bc_spirit_status_t *) &status);
}

bc_spirit_status_t bc_spirit1_read(uint8_t address, void *buffer, size_t length)
{
    // Set chip select low
    bc_spirit1_hal_chip_select_low();

    // Write header byte and read status bits (MSB)
    uint16_t status = bc_spirit1_hal_transfer_byte(1) << 8;

    // Write memory map address and read status bits (LSB)
    status |= bc_spirit1_hal_transfer_byte(address);

    // Read buffer
    for (size_t i = 0; i < length; i++)
    {
        // Write dummy byte and read data
        *((uint8_t *) buffer + i) = bc_spirit1_hal_transfer_byte(0);
    }

    // Set chip select high
    bc_spirit1_hal_chip_select_high();

    // TODO Why this cast?
    return *((bc_spirit_status_t *) &status);
}

void bc_spirit1_hal_init(void)
{
    // Initialize timer
    bc_spirit1_hal_init_timer();

    // Initialize GPIO
    bc_spirit1_hal_init_gpio();

    // Initialize SPI
    bc_spirit1_hal_init_spi();

    // Activate shutdown (forces delay)
    bc_spirit1_hal_shutdown_high();
}

void bc_spirit1_hal_shutdown_low(void)
{
    // Output log. 0 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BR_7;

    // Output log. 1 on CS pin
    GPIOA->BSRR = GPIO_BSRR_BS_15;

    // TODO This delay might not exist (maybe poll GPIO?)...

    // Set prescaler
    TIM7->PSC = 32 - 1;

    // Set auto-reload register - period 10 ms
    TIM7->ARR = 10000 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }
}

void bc_spirit1_hal_shutdown_high(void)
{
    // Output log. 0 on CS pin
    GPIOA->BSRR = GPIO_BSRR_BR_15;

    // Output log. 1 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BS_7;

    // Set prescaler
    TIM7->PSC = 64 - 1;

    // Set auto-reload register - period 100 ms
    TIM7->ARR = 50000 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }
}

void bc_spirit1_hal_chip_select_low(void)
{
    // Set CS pin to log. 0
    GPIOA->BSRR = GPIO_BSRR_BR_15;

    // Set prescaler
    TIM7->PSC = 0;

    // Set auto-reload register - period 4 us
    TIM7->ARR = 128 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }
}

void bc_spirit1_hal_chip_select_high(void)
{
    // Set prescaler
    TIM7->PSC = 0;

    // Set auto-reload register - period 4 us
    TIM7->ARR = 128 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }

    // Set CS pin to log. 1
    GPIOA->BSRR = GPIO_BSRR_BS_15;

    // Set prescaler
    TIM7->PSC = 0;

    // Set auto-reload register - period 4 us
    TIM7->ARR = 128 - 1;

    // Generate update of registers
    TIM7->EGR = TIM_EGR_UG;

    // Enable counter
    TIM7->CR1 |= TIM_CR1_CEN;

    // Wait until update event occurs
    while ((TIM7->CR1 & TIM_CR1_CEN) != 0)
    {
        continue;
    }
}

uint8_t bc_spirit1_hal_transfer_byte(uint8_t value)
{
    // Wait until transmit buffer is empty
    while ((SPI1->SR & SPI_SR_TXE) == 0)
    {
        continue;
    }

    // Write data register
    SPI1->DR = value;

    // Wait until receive buffer is full
    while ((SPI1->SR & SPI_SR_RXNE) == 0)
    {
        continue;
    }

    // Read data register
    value = SPI1->DR;

    return value;
}

static void bc_spirit1_hal_init_gpio(void)
{
    // Enable clock for GPIOH, GPIOB and GPIOA
    RCC->IOPENR |= RCC_IOPENR_GPIOHEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOAEN;

    // TODO Errata workaround
    RCC->IOPENR;

    // Output log. 1 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BS_7;

    // Pull-down on GPIO_0 pin
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD7_1;

    // Pull-down on MISO pin
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD4_1;

    // Pull-down on GPIO_1 pin
    GPIOH->PUPDR |= GPIO_PUPDR_PUPD0_1;

    // Very high speed on CS pin
    GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEED15_1 | GPIO_OSPEEDER_OSPEED15_0;

    // Very high speed on MOSI and SCLK pins
    GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEED5_1 | GPIO_OSPEEDER_OSPEED5_0 | GPIO_OSPEEDER_OSPEED3_1 | GPIO_OSPEEDER_OSPEED3_0;

    // General purpose output on CS pin, input on GPIO_0 pin
    GPIOA->MODER &= ~(GPIO_MODER_MODE15_1 | GPIO_MODER_MODE7_1 | GPIO_MODER_MODE7_0);

    // General purpose output on SDN pin, alternate function on MOSI, MISO and SCLK pins
    GPIOB->MODER &= ~(GPIO_MODER_MODE7_1 | GPIO_MODER_MODE5_0 | GPIO_MODER_MODE4_0 | GPIO_MODER_MODE3_0);

    // Input on GPIO_1 pin
    GPIOH->MODER &= ~(GPIO_MODER_MODE0_1 | GPIO_MODER_MODE0_0);
}

static void bc_spirit1_hal_init_spi(void)
{
    // Enable clock for SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Software slave management, baud rate control = fPCLK / 8, master configuration
    SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1 | SPI_CR1_MSTR;

    // Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

static void bc_spirit1_hal_init_timer(void)
{
    // Enable clock for TIM7
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;

    // Enable one-pulse mode
    TIM7->CR1 |= TIM_CR1_OPM;
}
