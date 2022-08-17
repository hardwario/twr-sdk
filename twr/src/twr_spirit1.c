#include <twr_spirit1.h>
#include <twr_scheduler.h>
#include <twr_exti.h>
#include <twr_system.h>
#include <twr_timer.h>
#include <stm32l0xx.h>
#include "SPIRIT_Config.h"
#include "SDK_Configuration_Common.h"
#include "MCU_Interface.h"

typedef enum
{
    TWR_SPIRIT1_STATE_INIT = 0,
    TWR_SPIRIT1_STATE_SLEEP = 1,
    TWR_SPIRIT1_STATE_TX = 2,
    TWR_SPIRIT1_STATE_RX = 3

} twr_spirit1_state_t;

typedef struct
{
    int initialized_semaphore;
    void (*event_handler)(twr_spirit1_event_t, void *);
    void *event_param;
    twr_scheduler_task_id_t task_id;
    twr_spirit1_state_t desired_state;
    twr_spirit1_state_t current_state;
    uint8_t tx_buffer[TWR_SPIRIT1_MAX_PACKET_SIZE];
    size_t tx_length;
    uint8_t  rx_buffer[TWR_SPIRIT1_MAX_PACKET_SIZE];
    size_t rx_length;
    int rx_rssi;
    twr_tick_t rx_timeout;
    twr_tick_t rx_tick_timeout;

} twr_spirit1_t;

static twr_spirit1_t _twr_spirit1;

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

static void _twr_spirit1_enter_state_tx(void);
static void _twr_spirit1_check_state_tx(void);
static void _twr_spirit1_enter_state_rx(void);
static void _twr_spirit1_check_state_rx(void);
static void _twr_spirit1_enter_state_sleep(void);

void twr_spirit1_hal_chip_select_low(void);
void twr_spirit1_hal_chip_select_high(void);
uint8_t twr_spirit1_hal_transfer_byte(uint8_t value);
static void twr_spirit1_hal_init_gpio(void);
static void twr_spirit1_hal_deinit_gpio(void);
static void twr_spirit1_hal_init_spi(void);
static void twr_spirit1_hal_deinit_spi(void);

static void _twr_spirit1_task(void *param);
static void _twr_spirit1_interrupt(twr_exti_line_t line, void *param);

bool twr_spirit1_init(void)
{
    if (_twr_spirit1.initialized_semaphore > 0)
    {
        _twr_spirit1.initialized_semaphore++;

        return true;
    }

    memset(&_twr_spirit1, 0, sizeof(_twr_spirit1));

    SpiritRadioSetXtalFrequency(XTAL_FREQUENCY);

    SpiritSpiInit();

    /* Spirit ON */
    SpiritEnterShutdown();

    SpiritExitShutdown();

    SpiritManagementWaExtraCurrent();

    /* Spirit IRQ config */
    SpiritGpioInit(&xGpioIRQ);

    /* Spirit Radio config */
    if (SpiritRadioInit(&xRadioInit) != 0)
    {

        twr_spirit1_hal_shutdown_high();

        twr_spirit1_hal_deinit_spi();

        twr_spirit1_hal_deinit_gpio();

        return false;
    }

    /* Spirit Packet config */
    SpiritPktBasicInit(&xBasicInit);

    SpiritPktBasicAddressesInit(&xAddressInit);

    _twr_spirit1.desired_state = TWR_SPIRIT1_STATE_SLEEP;

    _twr_spirit1.task_id = twr_scheduler_register(_twr_spirit1_task, NULL, 0);

    _twr_spirit1.initialized_semaphore++;

    return true;
}

bool twr_spirit1_deinit(void)
{
    if (--_twr_spirit1.initialized_semaphore != 0)
    {
        return false;
    }

    twr_spirit1_hal_shutdown_high();

    twr_spirit1_hal_deinit_spi();

    twr_spirit1_hal_deinit_gpio();

    twr_scheduler_unregister(_twr_spirit1.task_id);

    twr_exti_unregister(TWR_EXTI_LINE_PA7);

    return true;
}

void twr_spirit1_set_event_handler(void (*event_handler)(twr_spirit1_event_t, void *), void *event_param)
{
    _twr_spirit1.event_handler = event_handler;
    _twr_spirit1.event_param = event_param;
}

void *twr_spirit1_get_tx_buffer(void)
{
    return _twr_spirit1.tx_buffer;
}

void twr_spirit1_set_tx_length(size_t length)
{
    _twr_spirit1.tx_length = length;
}

size_t twr_spirit1_get_tx_length(void)
{
    return _twr_spirit1.tx_length;
}

void *twr_spirit1_get_rx_buffer(void)
{
    return _twr_spirit1.rx_buffer;
}

size_t twr_spirit1_get_rx_length(void)
{
    return _twr_spirit1.rx_length;
}

int twr_spirit1_get_rx_rssi(void)
{
    return _twr_spirit1.rx_rssi;
}

void twr_spirit1_set_rx_timeout(twr_tick_t timeout)
{
    _twr_spirit1.rx_timeout = timeout;

    if (_twr_spirit1.current_state == TWR_SPIRIT1_STATE_RX)
    {
        if (_twr_spirit1.rx_timeout == TWR_TICK_INFINITY)
        {
            _twr_spirit1.rx_tick_timeout = TWR_TICK_INFINITY;
        }
        else
        {
            _twr_spirit1.rx_tick_timeout = twr_tick_get() + _twr_spirit1.rx_timeout;
        }

        if (_twr_spirit1.initialized_semaphore > 0)
        {
            twr_scheduler_plan_absolute(_twr_spirit1.task_id, _twr_spirit1.rx_tick_timeout);
        }
    }
}

void twr_spirit1_tx(void)
{
    _twr_spirit1.desired_state = TWR_SPIRIT1_STATE_TX;

    if (_twr_spirit1.initialized_semaphore > 0)
    {
        twr_scheduler_plan_now(_twr_spirit1.task_id);
    }
}

void twr_spirit1_rx(void)
{
    _twr_spirit1.desired_state = TWR_SPIRIT1_STATE_RX;

    if (_twr_spirit1.initialized_semaphore > 0)
    {
        twr_scheduler_plan_now(_twr_spirit1.task_id);
    }
}

void twr_spirit1_sleep(void)
{
    _twr_spirit1.desired_state = TWR_SPIRIT1_STATE_SLEEP;

    if (_twr_spirit1.initialized_semaphore > 0)
    {
        twr_scheduler_plan_now(_twr_spirit1.task_id);
    }
}

static void _twr_spirit1_task(void *param)
{
    (void) param;

    if ((_twr_spirit1.current_state == TWR_SPIRIT1_STATE_RX) && (twr_tick_get() >= _twr_spirit1.rx_tick_timeout))
    {
        if (_twr_spirit1.event_handler != NULL)
        {
            _twr_spirit1.event_handler(TWR_SPIRIT1_EVENT_RX_TIMEOUT, _twr_spirit1.event_param);
        }
    }

    if (_twr_spirit1.desired_state != _twr_spirit1.current_state)
    {
        if (_twr_spirit1.desired_state == TWR_SPIRIT1_STATE_TX)
        {
            _twr_spirit1_enter_state_tx();

            return;
        }
        else if (_twr_spirit1.desired_state == TWR_SPIRIT1_STATE_RX)
        {
            _twr_spirit1_enter_state_rx();

            return;
        }
        else if (_twr_spirit1.desired_state == TWR_SPIRIT1_STATE_SLEEP)
        {
            _twr_spirit1_enter_state_sleep();

            return;
        }

        return;
    }

    if (_twr_spirit1.current_state == TWR_SPIRIT1_STATE_TX)
    {
        _twr_spirit1_check_state_tx();

        return;
    }
    else if (_twr_spirit1.current_state == TWR_SPIRIT1_STATE_RX)
    {
        _twr_spirit1_check_state_rx();

        return;
    }
}

static void _twr_spirit1_enter_state_tx(void)
{
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD7_1;

    _twr_spirit1.current_state = TWR_SPIRIT1_STATE_TX;

    SpiritCmdStrobeSabort();
    SpiritCmdStrobeReady();
    SpiritCmdStrobeFlushTxFifo();

    SpiritIrqDeInit(NULL);
    SpiritIrqClearStatus();
    SpiritIrq(TX_DATA_SENT, S_ENABLE);

    SpiritPktBasicSetPayloadLength(_twr_spirit1.tx_length);

    // TODO Why needed?
    SpiritPktBasicSetDestinationAddress(0x35);

    SpiritSpiWriteLinearFifo(_twr_spirit1.tx_length, _twr_spirit1.tx_buffer);

    twr_exti_register(TWR_EXTI_LINE_PA7, TWR_EXTI_EDGE_FALLING, _twr_spirit1_interrupt, NULL);

    SpiritCmdStrobeTx();
}

static void _twr_spirit1_check_state_tx(void)
{
    SpiritIrqs xIrqStatus;

    SpiritIrqGetStatus(&xIrqStatus);

    if (xIrqStatus.IRQ_TX_DATA_SENT)
    {
        SpiritIrqClearStatus();

        _twr_spirit1.desired_state = TWR_SPIRIT1_STATE_SLEEP;

        if (_twr_spirit1.event_handler != NULL)
        {
            _twr_spirit1.event_handler(TWR_SPIRIT1_EVENT_TX_DONE, _twr_spirit1.event_param);
        }

        if (_twr_spirit1.desired_state == TWR_SPIRIT1_STATE_RX)
        {
            _twr_spirit1_enter_state_rx();
        }
        else if (_twr_spirit1.desired_state == TWR_SPIRIT1_STATE_SLEEP)
        {
            _twr_spirit1_enter_state_sleep();
        }
        else if (_twr_spirit1.desired_state == TWR_SPIRIT1_STATE_TX)
        {
            _twr_spirit1_enter_state_tx();
        }
    }
}

static void _twr_spirit1_enter_state_rx(void)
{
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD7_1;

    _twr_spirit1.current_state = TWR_SPIRIT1_STATE_RX;

    if (_twr_spirit1.rx_timeout == TWR_TICK_INFINITY)
    {
        _twr_spirit1.rx_tick_timeout = TWR_TICK_INFINITY;
    }
    else
    {
        _twr_spirit1.rx_tick_timeout = twr_tick_get() + _twr_spirit1.rx_timeout;
    }

    twr_scheduler_plan_current_absolute(_twr_spirit1.rx_tick_timeout);

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

    twr_exti_register(TWR_EXTI_LINE_PA7, TWR_EXTI_EDGE_FALLING, _twr_spirit1_interrupt, NULL);

    /* RX command */
    SpiritCmdStrobeRx();
}

static void _twr_spirit1_check_state_rx(void)
{
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

        if (cRxData <= TWR_SPIRIT1_MAX_PACKET_SIZE)
        {
            /* Read the RX FIFO */
            SpiritSpiReadLinearFifo(cRxData, _twr_spirit1.rx_buffer);

            _twr_spirit1.rx_length = cRxData;

            uint8_t rssi_level;

            twr_spirit1_read(RSSI_LEVEL_BASE, &rssi_level, 1);

            _twr_spirit1.rx_rssi = ((int) rssi_level) / 2 - 130;

            if (_twr_spirit1.rx_timeout == TWR_TICK_INFINITY)
            {
                _twr_spirit1.rx_tick_timeout = TWR_TICK_INFINITY;
            }
            else
            {
                _twr_spirit1.rx_tick_timeout = twr_tick_get() + _twr_spirit1.rx_timeout;
            }

            twr_scheduler_plan_current_absolute(_twr_spirit1.rx_tick_timeout);

            if (_twr_spirit1.event_handler != NULL)
            {
                _twr_spirit1.event_handler(TWR_SPIRIT1_EVENT_RX_DONE, _twr_spirit1.event_param);
            }
        }
    }

    /* Flush the RX FIFO */
    SpiritCmdStrobeFlushRxFifo();

    /* RX command - to ensure the device will be ready for the next reception */
    SpiritCmdStrobeRx();
}

static void _twr_spirit1_enter_state_sleep(void)
{
    _twr_spirit1.current_state = TWR_SPIRIT1_STATE_SLEEP;

    SpiritCmdStrobeSabort();
    SpiritCmdStrobeReady();
    SpiritIrqDeInit(NULL);
    SpiritIrqClearStatus();
    SpiritCmdStrobeStandby();

    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD7_1;
}

twr_spirit_status_t twr_spirit1_command(uint8_t command)
{
    // Enable PLL
    twr_system_pll_enable();

    // Set chip select low
    twr_spirit1_hal_chip_select_low();

    // Write header byte and read status bits (MSB)
    uint16_t status_value = twr_spirit1_hal_transfer_byte(0x80) << 8;

    // Write memory map address and read status bits (LSB)
    status_value |= twr_spirit1_hal_transfer_byte(command);

    // Set chip select high
    twr_spirit1_hal_chip_select_high();

    // Disable PLL
    twr_system_pll_disable();

    twr_spirit_status_t *status = ((twr_spirit_status_t *) &status_value);

    // TODO Why this cast?
    return *status;
}

twr_spirit_status_t twr_spirit1_write(uint8_t address, const void *buffer, size_t length)
{
    // Enable PLL
    twr_system_pll_enable();

    // Set chip select low
    twr_spirit1_hal_chip_select_low();

    // Write header byte and read status bits (MSB)
    uint16_t status_value = twr_spirit1_hal_transfer_byte(0) << 8;

    // Write memory map address and read status bits (LSB)
    status_value |= twr_spirit1_hal_transfer_byte(address);

    // Write buffer
    for (size_t i = 0; i < length; i++)
    {
        // Write data
        twr_spirit1_hal_transfer_byte(*((uint8_t *) buffer + i));
    }

    // Set chip select high
    twr_spirit1_hal_chip_select_high();

    // Disable PLL
    twr_system_pll_disable();

    twr_spirit_status_t *status = ((twr_spirit_status_t *) &status_value);

    // TODO Why this cast?
    return *status;
}

twr_spirit_status_t twr_spirit1_read(uint8_t address, void *buffer, size_t length)
{
    // Enable PLL
    twr_system_pll_enable();

    // Set chip select low
    twr_spirit1_hal_chip_select_low();

    // Write header byte and read status bits (MSB)
    uint16_t status_value = twr_spirit1_hal_transfer_byte(1) << 8;

    // Write memory map address and read status bits (LSB)
    status_value |= twr_spirit1_hal_transfer_byte(address);

    // Read buffer
    for (size_t i = 0; i < length; i++)
    {
        // Write dummy byte and read data
        *((uint8_t *) buffer + i) = twr_spirit1_hal_transfer_byte(0);
    }

    // Set chip select high
    twr_spirit1_hal_chip_select_high();

    // Disable PLL
    twr_system_pll_disable();

    twr_spirit_status_t *status = ((twr_spirit_status_t *) &status_value);

    // TODO Why this cast?
    return *status;
}

void twr_spirit1_hal_init(void)
{
    // Initialize timer
    twr_timer_init();

    // Initialize GPIO
    twr_spirit1_hal_init_gpio();

    // Initialize SPI
    twr_spirit1_hal_init_spi();

    // Activate shutdown (forces delay)
    twr_spirit1_hal_shutdown_high();
}

void twr_spirit1_hal_shutdown_low(void)
{
    // Output log. 0 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BR_7;

    // Output log. 1 on CS pin
    GPIOA->BSRR = GPIO_BSRR_BS_15;

    // TODO This delay might not exist (maybe poll GPIO?)...

    twr_timer_start();

    twr_timer_delay(10000);

    twr_timer_stop();
}

void twr_spirit1_hal_shutdown_high(void)
{
    // Enable PLL
    twr_system_pll_enable();

    // Output log. 0 on CS pin
    GPIOA->BSRR = GPIO_BSRR_BR_15;

    // Output log. 1 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BS_7;

    twr_timer_start();

    twr_timer_delay(50000);

    twr_timer_clear();

    twr_timer_delay(50000);

    twr_timer_stop();

    // Disable PLL
    twr_system_pll_disable();
}

void twr_spirit1_hal_chip_select_low(void)
{
    // Set CS pin to log. 0
    GPIOA->BSRR = GPIO_BSRR_BR_15;

    twr_timer_start();

    while(twr_timer_get_microseconds() < 4)
    {
        continue;
    }

    twr_timer_stop();
}

void twr_spirit1_hal_chip_select_high(void)
{
    twr_timer_start();

    while(twr_timer_get_microseconds() < 4)
    {
        continue;
    }

    // Set CS pin to log. 1
    GPIOA->BSRR = GPIO_BSRR_BS_15;

    while(twr_timer_get_microseconds() < 8)
    {
        continue;
    }

    twr_timer_stop();
}

uint8_t twr_spirit1_hal_transfer_byte(uint8_t value)
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

static void twr_spirit1_hal_init_gpio(void)
{
    // Enable clock for GPIOH, GPIOB and GPIOA
    RCC->IOPENR |= RCC_IOPENR_GPIOHEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOAEN;

    // Errata workaround
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

static void twr_spirit1_hal_deinit_gpio(void)
{
    // Low speed on CS pin
    GPIOA->OSPEEDR &= ~GPIO_OSPEEDER_OSPEED15_Msk;

    // Low speed on CS pin, input on GPIO_0 pin
    GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEED5_Msk | GPIO_OSPEEDER_OSPEED3_Msk);

    // Analog on CS pin, input on GPIO_0 pin
    GPIOA->MODER |= GPIO_MODER_MODE15_Msk | GPIO_MODER_MODE7_Msk;

    // Analog on SDN pin, alternate function on MOSI, MISO and SCLK pins
    GPIOB->MODER |= GPIO_MODER_MODE7_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE3_Msk;

    // Analog on GPIO_1 pin
    GPIOH->MODER |= GPIO_MODER_MODE0_Msk;

    // Output log. 0 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BR_7;

    // No pull-down on GPIO_0 pin
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD7_Msk;

    // No pull-down on MISO pin
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD4_Msk;

    // No pull-down on GPIO_1 pin
    GPIOH->PUPDR &= ~GPIO_PUPDR_PUPD0_Msk;
}

static void twr_spirit1_hal_init_spi(void)
{
    // Enable clock for SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Software slave management, baud rate control = fPCLK / 8, master configuration
    SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1 | SPI_CR1_MSTR;

    // Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

static void twr_spirit1_hal_deinit_spi(void)
{
    // Disable SPI
    SPI1->CR1 = 0;

    // Disable clock for SPI1
    RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
}

static void _twr_spirit1_interrupt(twr_exti_line_t line, void *param)
{
    (void) line;
    (void) param;

    twr_scheduler_plan_now(_twr_spirit1.task_id);
}
