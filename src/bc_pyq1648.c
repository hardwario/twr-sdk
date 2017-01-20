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

static bc_pyq1648_t bc_pyq1648_default =
{
    ._gpio_channel_serin = BC_GPIO_P9, /* PIR module board (PB2) */
    ._gpio_channel_dl = BC_GPIO_P8, /* PIR module board (PB0) */
    ._event_handler = NULL,
    ._update_interval = 100, /* Check for event from PI every 100ms */
    ._aware_time = 0,
    ._state = BC_PYQ1648_STATE_INITIALIZE,
    ._event_valid = false,
    ._sensitivity = 20,
    ._blank_period = 1000, /* 1000ms */
    ._config = 0 };

#define BC_PYQ1648_DELAY_RUN 50
// TODO ... doladit init delay
#define BC_PYQ1648_DELAY_INITIALIZATION 50
#define PYQ1648_UPDATE_INTERVAL 100

static void _bc_pyq1648_msp_init(bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl);
static void _bc_pyq1648_dev_init(bc_pyq1648_t *self);
static void _bc_pyq1648_compose_event_unit_config(bc_pyq1648_t *self);
static void _bc_pyq1648_delay_100us(unsigned int i);
static bc_tick_t _bc_pyq1648_task(void *param);

static const uint8_t sensitivity_lut[3] =
{
    [BC_PYQ1648_SENSITIVITY_LOW] = 150,
    [BC_PYQ1648_SENSITIVITY_MEDIUM] = 70,
    [BC_PYQ1648_SENSITIVITY_HIGH] = 15, };

extern GPIO_TypeDef * bc_gpio_port[];
extern uint16_t bc_gpio_16_bit_mask[];
extern uint32_t bc_gpio_32_bit_upper_mask[];

GPIO_TypeDef **_pyq1648_gpiox_table = bc_gpio_port;
uint16_t *_pyq1648_set_mask = bc_gpio_16_bit_mask;
uint32_t *_pyq1648_reset_mask = bc_gpio_32_bit_upper_mask;

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
    self->_config |= 0x02; /* Event mode */
    self->_config <<= PYQ1648_FILTER_SOURCE_LEN;
    self->_config |= 0x01; /* Low pass filter */
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

    uint32_t regval = self->_config;
    uint32_t regmask = 0x1000000;

    uint32_t bsrr_mask[2] =
    {
        [0] = _pyq1648_reset_mask[self->_gpio_channel_serin],
        [1] = _pyq1648_set_mask[self->_gpio_channel_serin] };

    GPIO_TypeDef *GPIOx = _pyq1648_gpiox_table[self->_gpio_channel_serin];
    uint32_t *GPIOx_BSRR = (uint32_t *) &GPIOx->BSRR;

    bc_gpio_set_output(self->_gpio_channel_serin, false);

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

    bc_irq_enable();
}

static void _bc_pyq1648_delay_100us(unsigned int i)
{
    for (i *= 170; i >= 2; i--)
    {
        __asm("nop");
    }
}

static bool _bc_pyq1648_echo(bc_pyq1648_t *self)
{
    // nastavit jako manual read out
    _bc_pyq1648_set_dummy_forced_read_out(self);

    // pokusit se vz49st... pokud bude v3e nula, nebo sam0 jedni4kz vr8t9 error (false)


    // Vr8tit spr8vnou _config
}

static void _bc_pyq1648_set_dummy_forced_read_out(bc_pyq1648_t *self)
{
    uint32_t dummy_event_unit_config = 0b1000000000000000000010000;

    self->_config = dummy_event_unit_config;

    _bc_pyq1648_dev_init(self);

    _bc_pyq1648_compose_event_unit_config(self);
}

static void _bc_pyq1648_get_forcet_read_out(bc_pyq1648_t *self)
{
    // pulse po dobu setup time (TODO)

    // 39x puls d0lkz max 2 us, p5enastavit an vstup a vz49st bit (perioda jednoho bitu je max 22us)
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
            self->_event_handler(self, BC_PYQ1648_EVENT_ERROR);
        }

        self->_state = BC_PYQ1648_STATE_INITIALIZE;

        return PYQ1648_UPDATE_INTERVAL;
    }
    case BC_PYQ1648_STATE_INITIALIZE:
    {
        self->_state = BC_PYQ1648_STATE_ERROR;

        // TODO ... kontrola pøítomnosti modulu, kdyžtak goto start;

        _bc_pyq1648_dev_init(self);
        _bc_pyq1648_clear_event(self);

        self->_state = BC_PYQ1648_STATE_CHECK;

        return BC_PYQ1648_DELAY_INITIALIZATION;
    }
    case BC_PYQ1648_STATE_CHECK:
    {
        bc_tick_t tick_now = bc_tick_get();

        /*

         TODO ... check if module is connected,
         if so continue, else ...state = BC_PYQ1648_STATE_ERROR

         */

        if (tick_now >= self->_aware_time)
        {

            if (bc_gpio_get_input(self->_gpio_channel_dl))
            {
                self->_event_handler(self, BC_PYQ1648_EVENT_MOTION);
                self->_aware_time = tick_now + self->_blank_period;
            }

            _bc_pyq1648_clear_event(self);
        }

        self->_state = BC_PYQ1648_STATE_CHECK;

        return PYQ1648_UPDATE_INTERVAL;
    }
    default:
    {
        self->_state = BC_PYQ1648_STATE_ERROR;

        goto start;
    }
    }
}
