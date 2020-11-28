#include <hio_pyq1648.h>
#include <hio_irq.h>
#include <hio_gpio.h>
#include <hio_system.h>
#include <hio_timer.h>
#include <stm32l0xx.h>

#define HIO_PYQ1648_BPF 0x00
#define HIO_PYQ1648_LPF 0x01
#define HIO_PYQ1648_WAKE_UP_MODE 0x02

#define HIO_PYQ1648_DELAY_RUN 50
#define HIO_PYQ1648_DELAY_INITIALIZATION 10
#define HIO_PYQ1648_UPDATE_INTERVAL 100

static inline void _hio_pyq1648_msp_init(hio_gpio_channel_t gpio_channel_serin, hio_gpio_channel_t gpio_channel_dl);
static inline void _hio_pyq1648_dev_init(hio_pyq1648_t *self);
static inline void _hio_pyq1648_compose_event_unit_config(hio_pyq1648_t *self);
static void _hio_pyq1648_task(void *param);

static const uint8_t _hio_pyq1648_sensitivity_table[4] =
{
    [HIO_PYQ1648_SENSITIVITY_LOW] = 150,
    [HIO_PYQ1648_SENSITIVITY_MEDIUM] = 70,
    [HIO_PYQ1648_SENSITIVITY_HIGH] = 20,
    [HIO_PYQ1648_SENSITIVITY_VERY_HIGH] = 9
};

extern GPIO_TypeDef * hio_gpio_port[];
extern uint16_t hio_gpio_16_bit_mask[];
extern uint32_t hio_gpio_32_bit_upper_mask[];

static GPIO_TypeDef **_pyq1648_gpiox_table = hio_gpio_port;
static uint16_t *_pyq1648_set_mask = hio_gpio_16_bit_mask;
static uint32_t *_pyq1648_reset_mask = hio_gpio_32_bit_upper_mask;

void hio_pyq1648_init(hio_pyq1648_t *self, hio_gpio_channel_t gpio_channel_serin, hio_gpio_channel_t gpio_channel_dl)
{
    // Initialize structure
    memset(self, 0, sizeof(*self));

    // Initialize default values
    self->_sensitivity = _hio_pyq1648_sensitivity_table[HIO_PYQ1648_SENSITIVITY_HIGH];
    self->_blank_period = 1000;

    // Initialize actually used pins
    self->_gpio_channel_serin = gpio_channel_serin;
    self->_gpio_channel_dl = gpio_channel_dl;

    // Initialize event_unit_configuration register value
    // _hio_pyq1648_compose_event_unit_config(self);

    // Register task
    self->_task_id = hio_scheduler_register(_hio_pyq1648_task, self, HIO_PYQ1648_DELAY_RUN);

    hio_timer_init();
}

void hio_pyq1648_set_event_handler(hio_pyq1648_t *self, void (*event_handler)(hio_pyq1648_t *, hio_pyq1648_event_t, void *), void *event_param)
{
    // Set event handler
    self->_event_handler = event_handler;

    // Set event param
    self->_event_param = event_param;
}

void hio_pyq1648_set_sensitivity(hio_pyq1648_t *self, hio_pyq1648_sensitivity_t sensitivity)
{
    // Set sensitivity to desired value
    self->_sensitivity = _hio_pyq1648_sensitivity_table[sensitivity];

    self->_state = HIO_PYQ1648_STATE_INITIALIZE;

    hio_scheduler_plan_now(self->_task_id);
}

void hio_pyq1648_set_blank_period(hio_pyq1648_t *self, hio_tick_t blank_period)
{
    // Set blank period
    self->_blank_period = blank_period;
}

void _hio_pyq1648_compose_event_unit_config(hio_pyq1648_t *self)
{
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |                                                         Event unit configuration                                                     |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // | 7bit sensitivity | 4bit blind time | 2bit pulse counter | 2bit window time | 2bit operatin mode | 2bit filter source | 5bit reserved |
    //  --------------------------------------------------------------------------------------------------------------------------------------
    // |     from self    |  handled by SW  |        0x00        |       0x00       |    wake up mode    |  Band pass filter  | has to be 16  |
    //  --------------------------------------------------------------------------------------------------------------------------------------

    self->_config = 0x00000000;
    self->_config |= (self->_sensitivity << 17) | (HIO_PYQ1648_WAKE_UP_MODE << 7) | (HIO_PYQ1648_BPF << 5) |0x10;
}

static inline void _hio_pyq1648_msp_init(hio_gpio_channel_t gpio_channel_serin, hio_gpio_channel_t gpio_channel_dl)
{
    // Initialize SerialIn (SERIN) GPIO pin
    hio_gpio_init(gpio_channel_serin);
    hio_gpio_set_mode(gpio_channel_serin, HIO_GPIO_MODE_OUTPUT);

    // Initialize DirectLink (DL) GPIO pin
    hio_gpio_init(gpio_channel_dl);
    hio_gpio_set_mode(gpio_channel_dl, HIO_GPIO_MODE_INPUT);
    hio_gpio_set_pull(gpio_channel_dl, HIO_GPIO_PULL_DOWN);
}

static void _hio_pyq1648_clear_event(hio_pyq1648_t *self)
{
    // Clear event by pull down DL
    hio_gpio_set_mode(self->_gpio_channel_dl, HIO_GPIO_MODE_OUTPUT);
    hio_gpio_set_output(self->_gpio_channel_dl, 0);
    hio_gpio_set_mode(self->_gpio_channel_dl, HIO_GPIO_MODE_INPUT);
}

// TODO Consider using "OneWire"
static inline void _hio_pyq1648_dev_init(hio_pyq1648_t *self)
{
    // Disable interrupts
    hio_irq_disable();

    // Enable PLL
    hio_system_pll_enable();

    // Load desired event unit configuration
    uint32_t regval = self->_config;
    uint32_t regmask = 0x1000000;
    bool next_bit;

    // Prepare fast GPIO access
    uint32_t bsrr_mask[2] =
    {
        [0] = _pyq1648_reset_mask[self->_gpio_channel_serin],
        [1] = _pyq1648_set_mask[self->_gpio_channel_serin]
    };
    GPIO_TypeDef *GPIOx = _pyq1648_gpiox_table[self->_gpio_channel_serin];
    volatile uint32_t *GPIOx_BSRR = &GPIOx->BSRR;

    // Low level pin initialization
    _hio_pyq1648_msp_init(self->_gpio_channel_serin, self->_gpio_channel_dl);

    // Transmit event unit configuration
    for (int i = 0; i < 25; i++)
    {
        next_bit = (regval & regmask) != 0 ? true : false;
        regmask >>= 1;

        hio_timer_start();

        *GPIOx_BSRR = bsrr_mask[0];

        while (hio_timer_get_microseconds() < 1)
        {
            continue;
        }

        *GPIOx_BSRR = bsrr_mask[1];

        while (hio_timer_get_microseconds() < 2)
        {
            continue;
        }

        *GPIOx_BSRR = bsrr_mask[next_bit];

        while (hio_timer_get_microseconds() < 83)
        {
            continue;
        }

        hio_timer_stop();
    }

    hio_gpio_set_output(self->_gpio_channel_serin, 0);

    // Disable PLL
    hio_system_pll_disable();

    // Enable interrupts
    hio_irq_enable();
}

static void _hio_pyq1648_task(void *param)
{
    hio_pyq1648_t *self = param;

start:

    switch (self->_state)
    {
        case HIO_PYQ1648_STATE_ERROR:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, HIO_PYQ1648_EVENT_ERROR, self->_event_param);
            }

            self->_state = HIO_PYQ1648_STATE_INITIALIZE;

            hio_scheduler_plan_current_relative(HIO_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        case HIO_PYQ1648_STATE_INITIALIZE:
        {
            self->_state = HIO_PYQ1648_STATE_ERROR;

            _hio_pyq1648_compose_event_unit_config(self);

            _hio_pyq1648_dev_init(self);

            self->_state = HIO_PYQ1648_STATE_IGNORE;

            hio_scheduler_plan_current_relative(HIO_PYQ1648_DELAY_INITIALIZATION);

            return;
        }
        case HIO_PYQ1648_STATE_IGNORE:
        {
            self->_state = HIO_PYQ1648_STATE_ERROR;

            if (self->_ignore_untill == 0)
            {
                // TODO ... acquire !!!
                self->_ignore_untill = hio_tick_get() + (75000 / 15);
            }

            if (hio_gpio_get_input(self->_gpio_channel_dl) != 0)
            {
                _hio_pyq1648_clear_event(self);
            }

            if (hio_tick_get() >= self->_ignore_untill)
            {
                self->_state = HIO_PYQ1648_STATE_CHECK;
            }
            else
            {
                self->_state = HIO_PYQ1648_STATE_IGNORE;
            }

            hio_scheduler_plan_current_relative(HIO_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        case HIO_PYQ1648_STATE_CHECK:
        {
            hio_tick_t tick_now = hio_tick_get();

            self->_state = HIO_PYQ1648_STATE_ERROR;

            if (hio_gpio_get_input(self->_gpio_channel_dl) != 0)
            {
                if (tick_now >= self->_aware_time)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, HIO_PYQ1648_EVENT_MOTION, self->_event_param);
                    }

                    self->_aware_time = tick_now + self->_blank_period;
                }
                _hio_pyq1648_clear_event(self);
            }

            self->_state = HIO_PYQ1648_STATE_CHECK;

            hio_scheduler_plan_current_relative(HIO_PYQ1648_UPDATE_INTERVAL);

            return;
        }
        default:
        {
            self->_state = HIO_PYQ1648_STATE_ERROR;

            goto start;
        }
    }
}
