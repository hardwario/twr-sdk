#include <bc_pyq1648.h>
#include <bc_scheduler.h>

#include <stm32l083xx.h>
#include <bc_gpio.h>

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

#define PYQ1648_UPDATE_INTERVAL 100

typedef enum
{
    // not datasheet related !!!
    BC_PYQ1648_OPERATION_MODE_MANUAL = 0x00,
    BC_PYQ1648_OPERATION_MODE_EVENT = 0x02,
} bc_pyq1648_operation_mode_t;

typedef enum
{
    BC_PYQ1648_FILTER_SOURCE_PIR_BPF = 0,
    BC_PYQ1648_FILTER_SOURCE_PIR_LPF = 1,
    BC_PYQ1648_FILTER_SOURCE_TEMPERATURE_SENSOR = 3,
} bc_pyq1648_filter_source_t;

static bc_pyq1648_t bc_pyq1648_default = {
    ._gpio_channel_serin = 0,
    ._gpio_channel_dl = 0,
    ._event_handler = NULL,
    ._update_interval = 100,
    ._state = BC_PYQ1648_STATE_INITIALIZE,
    ._event_valid = false,
    ._sensitivity = 20,
    ._config = 0
};

#define BC_PYQ1648_DELAY_RUN 50
#define BC_PYQ1648_DELAY_INITIALIZATION 50
#define BC_PYQ1648_DELAY_MEASUREMENT 50

static void _bc_pyq1648_msp_init(bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl);
static void _bc_pyq1648_dev_init(bc_pyq1648_t *self);
static void _bc_pyq1648_compose_event_unit_config(bc_pyq1648_t *self);
static void _bc_pyq1648_delay_100us(unsigned int i);
static bc_tick_t _bc_pyq1648_task(void *param);

static const uint8_t sensitivity_lut[3] = {
    [BC_PYQ1648_SENSITIVITY_LOW] = 100,
    [BC_PYQ1648_SENSITIVITY_MEDIUM] = 50,
    [BC_PYQ1648_SENSITIVITY_HIGH] = 15,
};

GPIO_TypeDef **_pyq1648_gpiox_table = (GPIO_TypeDef **)bc_gpio_port;
/*
uint16_t *_pyq1648_set_mask = bc_gpio_16_bit_mask[];
uint32_t *_pyq1648_reset_mask = bc_gpio_32_bit_upper_mask[];
*/

void bc_pyq1648_init(bc_pyq1648_t *self, bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl)
{
    memcpy(self, &bc_pyq1648_default, sizeof(bc_pyq1648_t));

    self->_gpio_channel_serin = gpio_channel_serin;
    self->_gpio_channel_dl = gpio_channel_dl;

    _bc_pyq1648_msp_init(gpio_channel_serin, gpio_channel_dl);
    _bc_pyq1648_compose_event_unit_config(self);

    bc_scheduler_register(_bc_pyq1648_task, self, BC_PYQ1648_DELAY_RUN);
}

void bc_pyq1648_set_event_handler(bc_pyq1648_t *self, void (*event_handler)(bc_pyq1648_t *, bc_pyq1648_event_t))
{
    self->_event_handler = event_handler;
}

void bc_pyq1648_set_sensitivity(bc_pyq1648_t *self, bc_pyq1648_sensitivity_t sensitivity)
{
    self->_sensitivity = sensitivity_lut[sensitivity];
    _bc_pyq1648_compose_event_unit_config(self);
}

void _bc_pyq1648_compose_event_unit_config(bc_pyq1648_t *self)
{
    self->_config = 0x00000000;
    self->_config |= self->_sensitivity;
    self->_config <<= PYQ1648_BLIND_TIME_LEN + PYQ1648_PULSE_CONTER_LEN + PYQ1648_WINDOW_TIME_LEN + PYQ1648_OPERATION_MODE_LEN;
    self->_config |= BC_PYQ1648_OPERATION_MODE_EVENT;
    self->_config <<= PYQ1648_FILTER_SOURCE_LEN;
    self->_config |= BC_PYQ1648_FILTER_SOURCE_PIR_BPF;
    self->_config <<= PYQ1648_RESERVED_LEN;
    self->_config |= 0x10;
}

static void _bc_pyq1648_msp_init(bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl)
{
    bc_gpio_init(gpio_channel_serin);
    bc_gpio_set_mode(gpio_channel_serin, BC_GPIO_MODE_OUTPUT);

    bc_gpio_init(gpio_channel_dl);
    bc_gpio_set_mode(gpio_channel_dl, BC_GPIO_MODE_INPUT);
}

static void _bc_pyq1648_clear_event(bc_pyq1648_t *self)
{
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(self->_gpio_channel_dl, false);
    bc_gpio_set_mode(self->_gpio_channel_dl, BC_GPIO_MODE_INPUT);
}

static void _bc_pyq1648_dev_init(bc_pyq1648_t *self)
{
    bc_irq_disable();


    /*
    uint32_t regval = self->_config;
    uint32_t regmask = 0x1000000;

    uint32_t bsrr_mask[2] = {
        [0] = _pyq1648_reset_mask[self->_gpio_channel_serin],
        [1] = _pyq1648_set_mask[self->_gpio_channel_serin]
    };
    */
    GPIO_TypeDef *GPIOx = _pyq1648_gpiox_table[self->_gpio_channel_serin];
    /*
    uint32_t *GPIOx_BSRR = (uint32_t *)&GPIOx->BSRR;

    GPIOx_BSRR = 0x10;
*/
    /*
    *GPIOx_BSRR = bsrr_mask[0];
    *GPIOx_BSRR = bsrr_mask[1];
    *GPIOx_BSRR = bsrr_mask[0];
    *GPIOx_BSRR = bsrr_mask[1];
    *GPIOx_BSRR = bsrr_mask[0];
    *GPIOx_BSRR = bsrr_mask[1];
*/

    /*
    bc_gpio_set_output(self->_gpio_channel_serin, false);

    for (int i = 0; i < 25; i++)
    {
        bool next_bit = (regval & regmask) != 0 ? true : false;
        regmask >>= 1;
        bc_gpio_set_output(self->_gpio_channel_serin, false);
        bc_gpio_set_output(self->_gpio_channel_serin, true);
        *serin_bsrr = (next_bit == true) ? upper_mask : lower_mask;
        // GPIOB->BSRR = (next_bit == false) ? (0b100 << 16) : 0b100;
        // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, next_bit);
        // bc_gpio_set_output(self->_gpio_channel_serin, next_bit);
        // TODO ... replace with wait
        _bc_pyq1648_delay_100us(1);
    }

    bc_gpio_set_output(self->_gpio_channel_serin, false);
    // TODO ... replace with wait
    _bc_pyq1648_delay_100us(6);
    */

    bc_irq_enable();
}

static void _bc_pyq1648_delay_100us(unsigned int i)
{
    for (i *= 170; i >= 2; i--)
    {
        __asm("nop");
    }
}

static bc_tick_t _bc_pyq1648_task(void *param)
{
    bc_pyq1648_t *self = param;

    // TODO ... env9m jak to, pokud v;bec, ykontrolovat
    //          nejak7 test se d2lat mus9... pokus9m se ya49t vz49t8n9 dat y DL, pokud to pujde je PIR modul p5ipojen

    start:

    switch (self->_state)
    {
    case BC_PYQ1648_STATE_ERROR:
    {
        /* Error is probably impossible */

        self->_event_valid = false;

        if (self->_event_handler != NULL)
        {
            self->_event_handler(self, BC_PYQ1648_STATE_ERROR);
        }

        self->_state = BC_PYQ1648_STATE_INITIALIZE;

        return PYQ1648_UPDATE_INTERVAL;
    }
    case BC_PYQ1648_STATE_INITIALIZE:
    {
        self->_state = BC_PYQ1648_STATE_ERROR;

        // TODO ... kontrola pøítomnosti modulu, kdyžtak goto start;

        _bc_pyq1648_dev_init(self);

        self->_state = BC_PYQ1648_STATE_CHECK;
    }
    case BC_PYQ1648_STATE_CHECK:
    {
        /* If event occurred call event handler and update _blind_time (ten Pavlùv, ne nastavení) */

        if(bc_gpio_get_input(self->_gpio_channel_dl))
        {
            self->_event_handler(self, BC_PYQ1648_EVENT_MOTION);
        }
    }
    default:
    {
        self->_state = BC_PYQ1648_STATE_ERROR;

        goto start;
    }
    }
}
