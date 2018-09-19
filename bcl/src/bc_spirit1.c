#include <bc_spirit1.h>
#include <bc_scheduler.h>
#include <bc_exti.h>
#include <bc_system.h>
#include <bc_timer.h>
#include <stm32l0xx.h>
#include "SPIRIT_Config.h"
#include "SDK_Configuration_Common.h"
#include "MCU_Interface.h"

#define _BC_SPIRIT1_XTAL_FREQUENCY   50000000
#define _BC_SPIRIT1_XTAL_OFFSET_PPM  0
#ifdef USE_HIGH_BAND
  #if BAND == 915
    #define _BC_SPIRIT1_BASE_FREQUENCY              915.0e6
  #else
    #define _BC_SPIRIT1_BASE_FREQUENCY              868.0e6
  #endif
#endif
#define _BC_SPIRIT1_BANDWIDTH                   100e3
#define _BC_SPIRIT1_CHANNEL_SPACE               20e3
#define _BC_SPIRIT1_CHANNEL_NUMBER              0
#define _BC_SPIRIT1_DATARATE                    19200
#define _BC_SPIRIT1_MODULATION_SELECT           GFSK_BT1
#define _BC_SPIRIT1_FREQ_DEVIATION              20e3

#define _BC_SPIRIT1_PREAMBLE_LENGTH             PKT_PREAMBLE_LENGTH_04BYTES
#define _BC_SPIRIT1_SYNC_LENGTH                 PKT_SYNC_LENGTH_4BYTES
#define _BC_SPIRIT1_SYNC_WORD                   0x88888888
#define _BC_SPIRIT1_LENGTH_TYPE                 PKT_LENGTH_VAR
#define _BC_SPIRIT1_CONTROL_LENGTH              PKT_CONTROL_LENGTH_0BYTES
#define _BC_SPIRIT1_LENGTH_WIDTH                8
#define _BC_SPIRIT1_CRC_MODE                    PKT_CRC_MODE_8BITS

#define FBASE_DIVIDER 262144
#define ROUND(A)                                  (((A-(uint32_t)A)> 0.5)? (uint32_t)A+1:(uint32_t)A)

typedef enum
{
    BC_SPIRIT1_STATE_INIT = 0,
    BC_SPIRIT1_STATE_SLEEP = 1,
    BC_SPIRIT1_STATE_TX = 2,
    BC_SPIRIT1_STATE_RX = 3

} bc_spirit1_state_t;

typedef struct
{
  uint8_t XO_ON:1;          // This one bit field notifies if XO is operating (XO_ON is 1) or not (XO_On is 0)
  SpiritState MC_STATE: 7;  // This 7 bits field indicates the state
  uint8_t ERROR_LOCK: 1;    // < This one bit field notifies if there is an error on RCO calibration (ERROR_LOCK is 1) or not (ERROR_LOCK is 0)/
  uint8_t RX_FIFO_EMPTY: 1; // This one bit field notifies if RX FIFO is empty (RX_FIFO_EMPTY is 1) or not (RX_FIFO_EMPTY is 0) */
  uint8_t TX_FIFO_FULL: 1;  // This one bit field notifies if TX FIFO is full (TX_FIFO_FULL is 1) or not (TX_FIFO_FULL is 0)
  uint8_t ANT_SELECT: 1;    // This one bit field notifies the currently selected antenna
  uint8_t CONST: 4;         // This 4 bits field are reserved and equal to 5

} bc_spirit1_status_t;

typedef struct
{
    int initialized_semaphore;
    void (*event_handler)(bc_spirit1_event_t, void *);
    void *event_param;
    bc_scheduler_task_id_t task_id;
    bc_spirit1_state_t desired_state;
    bc_spirit1_state_t current_state;
    uint8_t tx_buffer[BC_SPIRIT1_MAX_PACKET_SIZE];
    size_t tx_length;
    uint8_t  rx_buffer[BC_SPIRIT1_MAX_PACKET_SIZE];
    size_t rx_length;
    bc_tick_t rx_timeout;
    bc_tick_t rx_tick_timeout;
    bc_spirit1_status_t status;

} bc_spirit1_t;

static bc_spirit1_t _bc_spirit1;

static void _bc_spirit1_enter_state_tx(void);
static void _bc_spirit1_check_state_tx(void);
static void _bc_spirit1_enter_state_rx(void);
static void _bc_spirit1_check_state_rx(void);
static void _bc_spirit1_enter_state_sleep(void);

static void _bc_spirit1_write_register(uint8_t address, uint8_t value);
static uint8_t _bc_spirit1_read_register(uint8_t address);
static bool _bc_spirit1_refresh_status(void);
static void _bc_spirit1_shutdown_low(void);
static void _bc_spirit1_shutdown_high(void);
static void _bc_spirit1_cs(int state);
static uint8_t _bc_spirit1_transfer(uint8_t value);
static void _bc_spirit1_gpio_init(void);
static void _bc_spirit1_gpio_deinit(void);
static void _bc_spirit1_spi_init(void);
static void _bc_spirit1_spi_deinit(void);

static void _bc_spirit1_task(void *param);
static void _bc_spirit1_interrupt(bc_exti_line_t line, void *param);

bool bc_spirit1_init(void)
{
    if (_bc_spirit1.initialized_semaphore > 0)
    {
        _bc_spirit1.initialized_semaphore++;

        return true;
    }

    memset(&_bc_spirit1, 0, sizeof(_bc_spirit1));

    SpiritRadioSetXtalFrequency(_BC_SPIRIT1_XTAL_FREQUENCY);

    // Initialize timer
    bc_timer_init();

    // Initialize GPIO
    _bc_spirit1_gpio_init();

    // Initialize SPI
    _bc_spirit1_spi_init();

    // Activate shutdown (forces delay)
    _bc_spirit1_shutdown_high();

    // Spirit ON
    _bc_spirit1_shutdown_low();

    // Spirit GPIO_0 IRQ config
    _bc_spirit1_write_register(GPIO0_CONF_BASE, CONF_GPIO_MODE_DIG_OUTL | CONF_GPIO_OUT_nIRQ);

    if ((_bc_spirit1.status.MC_STATE != MC_STATE_READY ) && (_bc_spirit1.status.CONST != 5))
    {
        _bc_spirit1_shutdown_high();

        _bc_spirit1_spi_deinit();

        _bc_spirit1_gpio_deinit();

        return false;
    }

    // Workaround for Vtune
    _bc_spirit1_write_register(SYNTH_CONFIG0_BASE, VCOTH_BASE);

    // Calculates the offset respect to RF frequency and according to xtal_ppm parameter: (xtal_ppm*FBase)/10^6
    int32_t FOffsetTmp = (int32_t)(((float)_BC_SPIRIT1_XTAL_OFFSET_PPM * _BC_SPIRIT1_BASE_FREQUENCY) / 1000000);

    bc_spirit1_command(COMMAND_STANDBY);

    uint8_t count = 10;

    do
    {
        for(volatile uint8_t i=0; i!=0xFF; i++); // Delay for state transition

        _bc_spirit1_refresh_status();

        if (count-- == 0)
        {
            return false;
        }
    }
    while (_bc_spirit1.status.MC_STATE != MC_STATE_STANDBY);

    uint8_t value;

    // Enables the synthesizer reference divider.
    value = _bc_spirit1_read_register(XO_RCO_TEST_BASE);
    _bc_spirit1_write_register(XO_RCO_TEST_BASE, value & 0xf7);

    // Goes in READY state
    bc_spirit1_command(COMMAND_READY);

    do
    {
        for(volatile uint8_t i=0; i!=0xFF; i++); // Delay for state transition

        _bc_spirit1_refresh_status();
    }
    while (_bc_spirit1.status.MC_STATE != MC_STATE_READY);

    int16_t xtalOffsetFactor;
    uint8_t anaRadioRegArray[8];
    uint8_t digRadioRegArray[4];
    uint8_t drM, drE, FdevM, FdevE, bwM, bwE;

    xtalOffsetFactor = (int16_t)(((float)FOffsetTmp * FBASE_DIVIDER) / _BC_SPIRIT1_XTAL_FREQUENCY);

    anaRadioRegArray[2] = (uint8_t)((((uint16_t) xtalOffsetFactor ) >> 8) & 0x0F);
    anaRadioRegArray[3] = (uint8_t)(xtalOffsetFactor);

    // Calculates the channel space factor
    anaRadioRegArray[0] = ((uint32_t)_BC_SPIRIT1_CHANNEL_SPACE << 9) / (_BC_SPIRIT1_XTAL_FREQUENCY >> 6) + 1;

    SpiritManagementWaTRxFcMem(_BC_SPIRIT1_BASE_FREQUENCY);

    // 2nd order DEM algorithm enabling
    value = _bc_spirit1_read_register(0xA3);
    _bc_spirit1_write_register(0xA3, value & ~0x02);

    // Calculates the datarate mantissa and exponent
    SpiritRadioSearchDatarateME(_BC_SPIRIT1_DATARATE, &drM, &drE);
    digRadioRegArray[0] = (uint8_t)(drM);
    digRadioRegArray[1] = (uint8_t)(0x00 | _BC_SPIRIT1_MODULATION_SELECT | drE);

    // Read the fdev register to preserve the clock recovery algo bit
    value = _bc_spirit1_read_register(FDEV0_BASE);

    // Calculates the frequency deviation mantissa and exponent
    SpiritRadioSearchFreqDevME(_BC_SPIRIT1_FREQ_DEVIATION, &FdevM, &FdevE);
    digRadioRegArray[2] = (uint8_t)((FdevE <<4 ) | (value & 0x08) | FdevM);

    // Calculates the channel filter mantissa and exponent
    SpiritRadioSearchChannelBwME(_BC_SPIRIT1_BANDWIDTH, &bwM, &bwE);

    digRadioRegArray[3] = (uint8_t)((bwM << 4) | bwE);

    float if_off = (3.0 * 480140) / (_BC_SPIRIT1_XTAL_FREQUENCY >> 12) - 64;

    _bc_spirit1_write_register(IF_OFFSET_ANA_BASE, ROUND(if_off));

    if_off = (3.0 * 480140) / (_BC_SPIRIT1_XTAL_FREQUENCY >> 13) - 64;

    anaRadioRegArray[1] = ROUND(if_off);

    // Sets the Xtal configuration in the ANA_FUNC_CONF0 register
    value = _bc_spirit1_read_register(ANA_FUNC_CONF0_BASE);
    _bc_spirit1_write_register(ANA_FUNC_CONF0_BASE, value | SELECT_24_26_MHZ_MASK);

    // Sets the channel number in the corresponding register
    _bc_spirit1_write_register(CHNUM_BASE, _BC_SPIRIT1_CHANNEL_NUMBER);


    // Configures the Analog Radio registers
    bc_spirit1_write(CHSPACE_BASE, anaRadioRegArray, 4);

    // Configures the Digital Radio registers
    bc_spirit1_write(MOD1_BASE, digRadioRegArray, 4);

    // Enable the freeze option of the AFC on the SYNC word
    value = _bc_spirit1_read_register(AFC2_BASE);
    _bc_spirit1_write_register(AFC2_BASE, value | AFC2_AFC_FREEZE_ON_SYNC_MASK);

    // Set the IQC correction optimal value
    anaRadioRegArray[0] = 0x80;
    anaRadioRegArray[1] = 0xE3;
    bc_spirit1_write(0x99, anaRadioRegArray, 2);

    anaRadioRegArray[0] = 0x22;
    bc_spirit1_write(0xBC, anaRadioRegArray, 1);

    if (SpiritRadioSetFrequencyBase(_BC_SPIRIT1_BASE_FREQUENCY) != 0)
    {
        _bc_spirit1_shutdown_high();

        _bc_spirit1_spi_deinit();

        _bc_spirit1_gpio_deinit();

        return false;
    }

    // Initializes the SPIRIT Basic packet

    // Always set the automatic packet filtering
    _bc_spirit1_write_register(PROTOCOL1_BASE, PROTOCOL1_AUTO_PCKT_FLT_MASK);

    // Always reset the control and source filtering (also if it is not present in basic), CRC check bit
    _bc_spirit1_write_register(PCKT_FLT_OPTIONS_BASE, PCKT_FLT_OPTIONS_RX_TIMEOUT_AND_OR_SELECT | PCKT_FLT_OPTIONS_CRC_CHECK_MASK);

    // Disable destination address and set control field in bytes
    _bc_spirit1_write_register(PCKTCTRL4_BASE, (0x00 | _BC_SPIRIT1_CONTROL_LENGTH));

    // Packet format and width length setting
    _bc_spirit1_write_register(PCKTCTRL3_BASE, PCKTCTRL3_PCKT_FRMT_BASIC | (_BC_SPIRIT1_LENGTH_WIDTH - 1));

    // Preamble, sync and fixed or variable length setting
    _bc_spirit1_write_register(PCKTCTRL2_BASE, _BC_SPIRIT1_PREAMBLE_LENGTH | _BC_SPIRIT1_SYNC_LENGTH | _BC_SPIRIT1_LENGTH_TYPE);

    // CRC length, whitening and FEC setting
    _bc_spirit1_write_register(PCKTCTRL1_BASE, _BC_SPIRIT1_CRC_MODE | PCKTCTRL1_WHIT_MASK);

    // Sync words
    uint32_t sync_word = (uint32_t) _BC_SPIRIT1_SYNC_WORD;
    bc_spirit1_write(SYNC4_BASE, &sync_word, 4);

    _bc_spirit1.desired_state = BC_SPIRIT1_STATE_SLEEP;

    _bc_spirit1.task_id = bc_scheduler_register(_bc_spirit1_task, NULL, 0);

    _bc_spirit1.initialized_semaphore++;

    return true;
}

bool bc_spirit1_deinit(void)
{
    if (--_bc_spirit1.initialized_semaphore != 0)
    {
        return false;
    }

    _bc_spirit1_shutdown_high();

    _bc_spirit1_spi_deinit();

    _bc_spirit1_gpio_deinit();

    bc_scheduler_unregister(_bc_spirit1.task_id);

    bc_exti_unregister(BC_EXTI_LINE_PA7);

    return true;
}

void bc_spirit1_set_event_handler(void (*event_handler)(bc_spirit1_event_t, void *), void *event_param)
{
    _bc_spirit1.event_handler = event_handler;
    _bc_spirit1.event_param = event_param;
}

void *bc_spirit1_get_tx_buffer(void)
{
    return _bc_spirit1.tx_buffer;
}

void bc_spirit1_set_tx_length(size_t length)
{
    _bc_spirit1.tx_length = length;
}

size_t bc_spirit1_get_tx_length(void)
{
    return _bc_spirit1.tx_length;
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

    if (_bc_spirit1.current_state == BC_SPIRIT1_STATE_RX)
    {
        if (_bc_spirit1.rx_timeout == BC_TICK_INFINITY)
        {
            _bc_spirit1.rx_tick_timeout = BC_TICK_INFINITY;
        }
        else
        {
            _bc_spirit1.rx_tick_timeout = bc_tick_get() + _bc_spirit1.rx_timeout;
        }

        if (_bc_spirit1.initialized_semaphore > 0)
        {
            bc_scheduler_plan_absolute(_bc_spirit1.task_id, _bc_spirit1.rx_tick_timeout);
        }
    }
}

void bc_spirit1_tx(void)
{
    _bc_spirit1.desired_state = BC_SPIRIT1_STATE_TX;

    if (_bc_spirit1.initialized_semaphore > 0)
    {
        bc_scheduler_plan_now(_bc_spirit1.task_id);
    }
}

void bc_spirit1_rx(void)
{
    _bc_spirit1.desired_state = BC_SPIRIT1_STATE_RX;

    if (_bc_spirit1.initialized_semaphore > 0)
    {
        bc_scheduler_plan_now(_bc_spirit1.task_id);
    }
}

void bc_spirit1_sleep(void)
{
    _bc_spirit1.desired_state = BC_SPIRIT1_STATE_SLEEP;

    if (_bc_spirit1.initialized_semaphore > 0)
    {
        bc_scheduler_plan_now(_bc_spirit1.task_id);
    }
}

static void _bc_spirit1_task(void *param)
{
    (void) param;

    if ((_bc_spirit1.current_state == BC_SPIRIT1_STATE_RX) && (bc_tick_get() >= _bc_spirit1.rx_tick_timeout))
    {
        if (_bc_spirit1.event_handler != NULL)
        {
            _bc_spirit1.event_handler(BC_SPIRIT1_EVENT_RX_TIMEOUT, _bc_spirit1.event_param);
        }
    }

    if (_bc_spirit1.desired_state != _bc_spirit1.current_state)
    {
        if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_TX)
        {
            _bc_spirit1_enter_state_tx();

            return;
        }
        else if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_RX)
        {
            _bc_spirit1_enter_state_rx();

            return;
        }
        else if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_SLEEP)
        {
            _bc_spirit1_enter_state_sleep();

            return;
        }

        return;
    }

    if (_bc_spirit1.current_state == BC_SPIRIT1_STATE_TX)
    {
        _bc_spirit1_check_state_tx();

        return;
    }
    else if (_bc_spirit1.current_state == BC_SPIRIT1_STATE_RX)
    {
        _bc_spirit1_check_state_rx();

        return;
    }
}

static void _bc_spirit1_enter_state_tx(void)
{
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD7_1;

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

    bc_exti_register(BC_EXTI_LINE_PA7, BC_EXTI_EDGE_FALLING, _bc_spirit1_interrupt, NULL);

    SpiritCmdStrobeTx();
}

static void _bc_spirit1_check_state_tx(void)
{
    SpiritIrqs xIrqStatus;

    SpiritIrqGetStatus(&xIrqStatus);

    if (xIrqStatus.IRQ_TX_DATA_SENT)
    {
        SpiritIrqClearStatus();

        _bc_spirit1.desired_state = BC_SPIRIT1_STATE_SLEEP;

        if (_bc_spirit1.event_handler != NULL)
        {
            _bc_spirit1.event_handler(BC_SPIRIT1_EVENT_TX_DONE, _bc_spirit1.event_param);
        }

        if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_RX)
        {
            _bc_spirit1_enter_state_rx();
        }
        else if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_SLEEP)
        {
            _bc_spirit1_enter_state_sleep();
        }
        else if (_bc_spirit1.desired_state == BC_SPIRIT1_STATE_TX)
        {
            _bc_spirit1_enter_state_tx();
        }
    }
}

static void _bc_spirit1_enter_state_rx(void)
{
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD7_1;

    _bc_spirit1.current_state = BC_SPIRIT1_STATE_RX;

    if (_bc_spirit1.rx_timeout == BC_TICK_INFINITY)
    {
        _bc_spirit1.rx_tick_timeout = BC_TICK_INFINITY;
    }
    else
    {
        _bc_spirit1.rx_tick_timeout = bc_tick_get() + _bc_spirit1.rx_timeout;
    }

    bc_scheduler_plan_current_absolute(_bc_spirit1.rx_tick_timeout);

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

    bc_exti_register(BC_EXTI_LINE_PA7, BC_EXTI_EDGE_FALLING, _bc_spirit1_interrupt, NULL);

    /* RX command */
    SpiritCmdStrobeRx();
}

static void _bc_spirit1_check_state_rx(void)
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

        if (cRxData <= BC_SPIRIT1_MAX_PACKET_SIZE)
        {
            /* Read the RX FIFO */
            SpiritSpiReadLinearFifo(cRxData, _bc_spirit1.rx_buffer);

            _bc_spirit1.rx_length = cRxData;

            if (_bc_spirit1.rx_timeout == BC_TICK_INFINITY)
            {
                _bc_spirit1.rx_tick_timeout = BC_TICK_INFINITY;
            }
            else
            {
                _bc_spirit1.rx_tick_timeout = bc_tick_get() + _bc_spirit1.rx_timeout;
            }

            bc_scheduler_plan_current_absolute(_bc_spirit1.rx_tick_timeout);

            if (_bc_spirit1.event_handler != NULL)
            {
                _bc_spirit1.event_handler(BC_SPIRIT1_EVENT_RX_DONE, _bc_spirit1.event_param);
            }
        }
    }

    /* Flush the RX FIFO */
    SpiritCmdStrobeFlushRxFifo();

    /* RX command - to ensure the device will be ready for the next reception */
    SpiritCmdStrobeRx();
}

static void _bc_spirit1_enter_state_sleep(void)
{
    _bc_spirit1.current_state = BC_SPIRIT1_STATE_SLEEP;

    SpiritCmdStrobeSabort();
    SpiritCmdStrobeReady();
    SpiritIrqDeInit(NULL);
    SpiritIrqClearStatus();
    SpiritCmdStrobeStandby();

    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD7_1;
}

bc_spirit_status_t bc_spirit1_command(uint8_t command)
{
    // Enable PLL
    bc_system_pll_enable();

    // Set chip select low
    _bc_spirit1_cs(0);

    uint16_t *status = (uint16_t *) &_bc_spirit1.status;

    // Write header byte and read status bits (MSB)
    *status = _bc_spirit1_transfer(0x80) << 8;

    // Write memory map address and read status bits (LSB)
    *status |= _bc_spirit1_transfer(command);

    // Set chip select high
    _bc_spirit1_cs(1);

    // Disable PLL
    bc_system_pll_disable();

    // TODO Why this cast?
    return *((bc_spirit_status_t *) status);
}

bc_spirit_status_t bc_spirit1_write(uint8_t address, const void *buffer, size_t length)
{
    // Enable PLL
    bc_system_pll_enable();

    // Set chip select low
    _bc_spirit1_cs(0);

    uint16_t *status = (uint16_t *) &_bc_spirit1.status;

    // Write header byte and read status bits (MSB)
    *status = _bc_spirit1_transfer(0) << 8;

    // Write memory map address and read status bits (LSB)
    *status |= _bc_spirit1_transfer(address);

    // Write buffer
    for (size_t i = 0; i < length; i++)
    {
        // Write data
        _bc_spirit1_transfer(*((uint8_t *) buffer + i));
    }

    // Set chip select high
    _bc_spirit1_cs(1);

    // Disable PLL
    bc_system_pll_disable();

    // TODO Why this cast?
    return *((bc_spirit_status_t *) status);
}

bc_spirit_status_t bc_spirit1_read(uint8_t address, void *buffer, size_t length)
{
    // Enable PLL
    bc_system_pll_enable();

    // Set chip select low
    _bc_spirit1_cs(0);

    uint16_t *status = (uint16_t *) &_bc_spirit1.status;

    // Write header byte and read status bits (MSB)
    *status = _bc_spirit1_transfer(1) << 8;

    // Write memory map address and read status bits (LSB)
    *status |= _bc_spirit1_transfer(address);

    // Read buffer
    for (size_t i = 0; i < length; i++)
    {
        // Write dummy byte and read data
        *((uint8_t *) buffer + i) = _bc_spirit1_transfer(0);
    }

    // Set chip select high
    _bc_spirit1_cs(1);

    // Disable PLL
    bc_system_pll_disable();

    // TODO Why this cast?
    return *((bc_spirit_status_t *) status);
}

static void _bc_spirit1_write_register(uint8_t address, uint8_t value)
{
    bc_spirit1_write(address, &value, 1);
}

static uint8_t _bc_spirit1_read_register(uint8_t address)
{
    uint8_t value;

    bc_spirit1_read(address, &value, 1);

    return value;
}

static bool _bc_spirit1_refresh_status(void)
{
    uint8_t buffer[2];

    uint8_t *status = (uint8_t *) &_bc_spirit1.status;

    for (uint8_t i = 0; i < 10; i++)
    {
        bc_spirit1_read(MC_STATE1_BASE, buffer, 2);

        if (((status[0] == buffer[1]) && ((status[1] & 0x0F) == buffer[0])))
        {
        return true;
        }
    }

    return false;
}

static void _bc_spirit1_shutdown_low(void)
{
    // Output log. 0 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BR_7;

    // Output log. 1 on CS pin
    GPIOA->BSRR = GPIO_BSRR_BS_15;

    bc_timer_start();

    bc_timer_delay(1000);

    bc_timer_stop();

    // Disable Extra current consumption after power-on

    _bc_spirit1_write_register(0xB2, 0xCA);

    _bc_spirit1_write_register(0xA8, 0x04);

    _bc_spirit1_read_register(0xA8); // just a read to loose some microsecs more

    _bc_spirit1_write_register(0xA8, 0x00);
}

static void _bc_spirit1_shutdown_high(void)
{
    // Output log. 0 on CS pin
    GPIOA->BSRR = GPIO_BSRR_BR_15;

    // Output log. 1 on SDN pin
    GPIOB->BSRR = GPIO_BSRR_BS_7;

    bc_timer_start();

    bc_timer_delay(50000);

    bc_timer_clear();

    bc_timer_delay(50000);

    bc_timer_stop();
}

static void _bc_spirit1_cs(int state)
{
    bc_timer_start();

    while(bc_timer_get_microseconds() < 4)
    {
        continue;
    }

    if (state == 0)
    {
        GPIOA->BSRR = GPIO_BSRR_BR_15;
    }
    else
    {
        GPIOA->BSRR = GPIO_BSRR_BS_15;

        while(bc_timer_get_microseconds() < 8)
        {
            continue;
        }
    }

    bc_timer_stop();
}

static uint8_t _bc_spirit1_transfer(uint8_t value)
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

static void _bc_spirit1_gpio_init(void)
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

static void _bc_spirit1_gpio_deinit(void)
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

static void _bc_spirit1_spi_init(void)
{
    // Enable clock for SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Software slave management, baud rate control = fPCLK / 8, master configuration
    SPI1->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_1 | SPI_CR1_MSTR;

    // Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

static void _bc_spirit1_spi_deinit(void)
{
    // Disable SPI
    SPI1->CR1 = 0;

    // Disable clock for SPI1
    RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN;
}

static void _bc_spirit1_interrupt(bc_exti_line_t line, void *param)
{
    (void) line;
    (void) param;

    bc_scheduler_plan_now(_bc_spirit1.task_id);
}
