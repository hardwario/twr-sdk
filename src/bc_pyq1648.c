#include <bc_pyq1648.h>
#include <bc_scheduler.h>
#include <bc_irq.h>
#include <bc_gpio.h>
#include <stm32l0xx.h>

extern GPIO_TypeDef *bc_gpio_port[];
extern uint16_t bc_gpio_16_bit_mask[];
extern uint32_t bc_gpio_32_bit_upper_mask[];

#define PYQ1648_SENSITIVITY_MASK 0xff
#define PYQ1648_BLIND_TIME_MASK 0x0f
#define PYQ1648_PULSE_CONTER_MASK 0x03
#define PYQ1648_WINDOW_TIME_MASK 0x03
#define PYQ1648_OPERATION_MASK 0x03
#define PYQ1648_FILTER_SOURCE_MASK 0x03
#define PYQ1648_RESERVED_MASK 0x1f

#define PYQ1648_SENSITIVITY_LEN 0x08
#define PYQ1648_BLIND_TIME_LEN 0x04
#define PYQ1648_PULSE_CONTER_LEN 0x02
#define PYQ1648_WINDOW_TIME_LEN 0x02
#define PYQ1648_OPERATION_MODE_LEN 0x02
#define PYQ1648_FILTER_SOURCE_LEN 0x02
#define PYQ1648_RESERVED_LEN 0x05

#define BC_PYQ1648_CONNECTION_CHECK true
#define BC_PYQ1648_DELAY_RUN 50
#define BC_PYQ1648_DELAY_INITIALIZATION 10
#define BC_PYQ1648_UPDATE_INTERVAL 50
#define BC_PYQ1648_CONNECTION_CHECK_INTERVAL 5000
#define BC_PYQ1648_DUMMY_EVENT_UNIT_CONFIG 0x1000010

static inline void _bc_pyq1648_msp_init(bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl);
static void _bc_pyq1648_dev_init(bc_pyq1648_t *self);
static void _bc_pyq1648_compose_event_unit_config(bc_pyq1648_t *self);
static void _bc_pyq1648_delay_100us(unsigned int i);
static void _bc_pyq1648_task(void *param);
static inline bool _bc_pyq1648_echo(bc_pyq1648_t *self);
static inline void _bc_pyq1648_set_dummy_forced_read_out(bc_pyq1648_t *self);
static inline bool _bc_pyq1648_is_pir_module_present(bc_pyq1648_t *self);

static const uint8_t _bc_pyq1648_sensitivity_table[4] =
{
    [BC_PYQ1648_SENSITIVITY_LOW] = 150,
    [BC_PYQ1648_SENSITIVITY_MEDIUM] = 70,
    [BC_PYQ1648_SENSITIVITY_HIGH] = 20,
    [BC_PYQ1648_SENSITIVITY_VERY_HIGH] = 9 };

extern GPIO_TypeDef * bc_gpio_port[];
extern uint16_t bc_gpio_16_bit_mask[];
extern uint32_t bc_gpio_32_bit_upper_mask[];

static GPIO_TypeDef **_pyq1648_gpiox_table = bc_gpio_port;
static uint16_t *_pyq1648_set_mask = bc_gpio_16_bit_mask;
static uint32_t *_pyq1648_reset_mask = bc_gpio_32_bit_upper_mask;

void bc_pyq1648_init(bc_pyq1648_t *self, bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl)
{
    // Initialize self structure
    memset(self, 0, sizeof(*self));

    // Initialize self default values
    self->_sensitivity = _bc_pyq1648_sensitivity_table[BC_PYQ1648_SENSITIVITY_HIGH];
    self->_blank_period = 1000;

    // Initialize self actually used pins
    self->_gpio_channel_serin = gpio_channel_serin;
    self->_gpio_channel_dl = gpio_channel_dl;

    // TODO ... test
    // Initialize low level
    // _bc_pyq1648_msp_init(gpio_channel_serin, gpio_channel_dl);

    // Initialize self event_unit_configuration register value
    _bc_pyq1648_compose_event_unit_config(self);

    // Register task
    bc_scheduler_register(_bc_pyq1648_task, self, BC_PYQ1648_DELAY_RUN);
}

void bc_pyq1648_set_event_handler(bc_pyq1648_t *self, void (*event_handler)(bc_pyq1648_t *, bc_pyq1648_event_t, void *), void *event_param)
{
    // Set event handler
    self->_event_handler = event_handler;

    // Set event param
    self->_event_param = event_param;
}

void bc_pyq1648_set_sensitivity(bc_pyq1648_t *self, bc_pyq1648_sensitivity_t sensitivity)
{
    // Edit self sensitivity to desired value
    self->_sensitivity = _bc_pyq1648_sensitivity_table[sensitivity];

    // Initialize self event unit configuration register value
    _bc_pyq1648_compose_event_unit_config(self);
}

void bc_pyq1648_set_blank_period(bc_pyq1648_t *self, bc_tick_t blank_period)
{
    // Edit self blank period
    self->_blank_period = blank_period;
}

void _bc_pyq1648_compose_event_unit_config(bc_pyq1648_t *self)
{
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |                                                         Event unit configuration                                                     |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // | 7bit sensitivity | 4bit blind time | 2bit pulse counter | 2bit window time | 2bit operatin mode | 2bit filter source | 5bit reserved |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |     from self    |  handled by SW  |        0x00        |       0x00       |    wake up mode    |     mode => BPF    | has to be 16  |
    //  --------------------------------------------------------------------------------------------------------------------------------------

    // TODO ... workaround
    self->_config = 0x00000000;
    self->_config |= self->_sensitivity;
    self->_config <<= PYQ1648_BLIND_TIME_LEN + PYQ1648_PULSE_CONTER_LEN + PYQ1648_WINDOW_TIME_LEN + PYQ1648_OPERATION_MODE_LEN;
    self->_config |= 0x02; // Event mode
    self->_config <<= PYQ1648_FILTER_SOURCE_LEN;
    // self->_config |= 0x00; // Band pass filter
    self->_config |= 0x01; // Low pass filter
    self->_config <<= PYQ1648_RESERVED_LEN;
    self->_config |= 0x10;
}

static inline void _bc_pyq1648_msp_init(bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl)
{
    // Initialize SerialIn (SERIN) GPIO pin
    bc_gpio_init(gpio_channel_serin);
    bc_gpio_set_mode(gpio_channel_serin, BC_GPIO_MODE_OUTPUT);

    // Initialize DirectLink (DL) GPIO pin
    bc_gpio_init(gpio_channel_dl);
    bc_gpio_set_mode(gpio_channel_dl, BC_GPIO_MODE_INPUT);
}

static void _bc_pyq1648_clear_event(bc_pyq1648_t *self)
{
    // Clear event by pull DL low
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(self->_gpio_channel_dl, false);
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);
}

static void _bc_pyq1648_dev_init(bc_pyq1648_t *self)
{
    // Disable interrupts
    bc_irq_disable();

    // Load desired event unit configuration
    uint32_t regval = self->_config;
    uint32_t regmask = 0x1000000;

    // Initialize used mask for SERIN pin
    uint32_t bsrr_mask[2] =
    {
        [0] = _pyq1648_reset_mask[self->_gpio_channel_serin],
        [1] = _pyq1648_set_mask[self->_gpio_channel_serin] 
    };

    // Initialize pointer to GPIO BSRR register of SERIN pin
    // (very fast operations with pins are needed)
    GPIO_TypeDef *GPIOx = _pyq1648_gpiox_table[self->_gpio_channel_serin];
    volatile uint32_t *GPIOx_BSRR = &GPIOx->BSRR;

    // Low level pin initialization
    _bc_pyq1648_msp_init(self->_gpio_channel_serin, self->_gpio_channel_dl);

    // Transmit event unit configuration
    for (int i = 0; i < 25; i++)
    {
        bool next_bit = (regval & regmask) != 0 ? true : false;
        regmask >>= 1;

        *GPIOx_BSRR = bsrr_mask[0];
        *GPIOx_BSRR = bsrr_mask[1];
        *GPIOx_BSRR = bsrr_mask[next_bit];
        // TODO ... replace with wait
        _bc_pyq1648_delay_100us(1);
    }

    bc_gpio_set_output(self->_gpio_channel_serin, false);
    // TODO ... replace with wait
    _bc_pyq1648_delay_100us(6);

    // Clear event that appears almost every time that PIR is initialized
    _bc_pyq1648_clear_event(self);

    // Enable interrupts
    bc_irq_enable();
}

// TODO ... workaround !!!
static void _bc_pyq1648_delay_100us(unsigned int i)
{
    for (i *= 170; i >= 2; i--)
    {
        __asm("nop");
    }
}

static inline bool _bc_pyq1648_echo(bc_pyq1648_t *self)
{
    bool pir_module_present;
    uint32_t event_unit_config = self->_config;

    // Set PIR to forced read out mode
    self->_config = BC_PYQ1648_DUMMY_EVENT_UNIT_CONFIG;

    // Initialize PIR
    _bc_pyq1648_dev_init(self);

    self->_config = event_unit_config;

    // If PIR present ...
    if (_bc_pyq1648_is_pir_module_present(self))
    {
        // ... restore original configuration
        _bc_pyq1648_dev_init(self);
        return true;
    }
    else
    {
        return false;
    }
}

static inline bool _bc_pyq1648_is_pir_module_present(bc_pyq1648_t *self)
{
    int32_t i;

    int32_t PIRval_temp;

    // Disable interrupts
    bc_irq_disable();

    // Initialize used masks for DL pin
    uint32_t bsrr_mask[2] =
    {
        [0] = _pyq1648_reset_mask[self->_gpio_channel_dl],
        [1] = _pyq1648_set_mask[self->_gpio_channel_dl]
    };
    uint32_t idr_mask = _pyq1648_set_mask[self->_gpio_channel_dl];

    // Initialize pointers to GPIO BSRR and IDR registers of DL pin
    // (very fast operations with pins are needed)
    GPIO_TypeDef *GPIOx = _pyq1648_gpiox_table[self->_gpio_channel_dl];
    volatile uint32_t *GPIOx_BSRR = &GPIOx->BSRR;
    volatile uint32_t *GPIOx_IDR = &GPIOx->IDR;

    // Pull DL high, to force uC controlled DL read out
    *GPIOx_BSRR = bsrr_mask[1];
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_OUTPUT);
    _bc_pyq1648_delay_100us(1);

    // get first 15bit out-off-range and ADC value
    PIRval_temp = 0;
    for (i = 0; i < 14; i++)
    {
        // Set next BitPos
        PIRval_temp <<= 1;

        // create low to high transition
        *GPIOx_BSRR = bsrr_mask[0];
        bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_OUTPUT);
        *GPIOx_BSRR = bsrr_mask[1];

        // Configure DL as Input
        bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);
        
        // If DL is high ...
        if ((*GPIOx_IDR & idr_mask) != 0)
        {
            // ... set corresponding bit
            PIRval_temp |= 0x01;
        }
    }
    
    // Pull DL down
    *GPIOx_BSRR = bsrr_mask[0];
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);

    // Configure DL as Input
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);

    // Enable interrupts
    bc_irq_enable();

    // If readout PIR value and configuration not valid ...
    if ((PIRval_temp == 0x3fff) || (PIRval_temp == 0x00))
    {
        // ...
        return false;
    }
    else
    {
        return true;
    }

}

static void _bc_pyq1648_task(void *param)
{
    bc_pyq1648_t *self = param;

start:

    switch (self->_state)
    {
        case BC_PYQ1648_STATE_ERROR:
        {
            self->_event_valid = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_PYQ1648_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_PYQ1648_STATE_INITIALIZE;

            bc_scheduler_plan_current_relative(BC_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        case BC_PYQ1648_STATE_INITIALIZE:
        {
            self->_state = BC_PYQ1648_STATE_ERROR;

            _bc_pyq1648_dev_init(self);

            self->_state = BC_PYQ1648_STATE_IGNORE;

            bc_scheduler_plan_current_relative(BC_PYQ1648_DELAY_INITIALIZATION);

            return;
        }
        case BC_PYQ1648_STATE_IGNORE:
        {
            self->_state = BC_PYQ1648_STATE_ERROR;

            if (self->_ignore_untill == 0)
            {
                // TODO ... acquire !!!
                self->_ignore_untill = bc_tick_get() + (75000 / 15);
            }

            if (bc_gpio_get_input(self->_gpio_channel_dl))
            {
                _bc_pyq1648_clear_event(self);
            }

            if (bc_tick_get() >= self->_ignore_untill)
            {
                self->_state = BC_PYQ1648_STATE_CHECK;
            }
            else
            {
                self->_state = BC_PYQ1648_STATE_IGNORE;
            }

            bc_scheduler_plan_current_relative(BC_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        case BC_PYQ1648_STATE_CHECK:
        {
            bc_tick_t tick_now = bc_tick_get();

            self->_state = BC_PYQ1648_STATE_ERROR;

            if (bc_gpio_get_input(self->_gpio_channel_dl))
            {
                if (tick_now >= self->_aware_time)
                {
                    self->_event_handler(self, BC_PYQ1648_EVENT_MOTION, self->_event_param);
                    self->_aware_time = tick_now + self->_blank_period;
                    self->_event_valid = true;
                }
                _bc_pyq1648_clear_event(self);
            }

#if (BC_PYQ1648_CONNECTION_CHECK)
            if (tick_now >= self->_connection_check)
            {
                self->_connection_check = tick_now + BC_PYQ1648_CONNECTION_CHECK_INTERVAL;
                if (_bc_pyq1648_echo(self) == false)
                {
                    // self->_event_valid = false;
                    // self->_event_handler(self, BC_PYQ1648_EVENT_ERROR, self->_event_param);
                    goto start;
                }
            }
#endif

            self->_state = BC_PYQ1648_STATE_CHECK;

            bc_scheduler_plan_current_relative(BC_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        default:
        {
            self->_state = BC_PYQ1648_STATE_ERROR;

            goto start;
        }
    }
}
