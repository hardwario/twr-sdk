#include <bc_hc_sr04.h>
#include <bc_system.h>
#include <bc_timer.h>

static void _bc_hc_sr04_task_interval(void *param);
static void _bc_hc_sr04_task_measure(void *param);

void bc_hc_sr04_init(bc_hc_sr04_t *self, bc_gpio_channel_t channel_echo, bc_gpio_channel_t channel_trig)
{
    memset(self, 0, sizeof(*self));
    self->_channel_echo = channel_echo;
    self->_channel_trig = channel_trig;

    self->_task_id_interval = bc_scheduler_register(_bc_hc_sr04_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_hc_sr04_task_measure, self, BC_TICK_INFINITY);

    // Pin Echo
    bc_gpio_init(self->_channel_echo);
    bc_gpio_set_mode(self->_channel_echo, BC_GPIO_MODE_INPUT);
    // Pin Trig
    bc_gpio_init(self->_channel_trig);
    bc_gpio_set_mode(self->_channel_trig, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(self->_channel_trig, 0);
}

void bc_hc_sr04_set_event_handler(bc_hc_sr04_t *self, void (*event_handler)(bc_hc_sr04_t *, bc_hc_sr04_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_hc_sr04_set_update_interval(bc_hc_sr04_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_hc_sr04_measure(self);
    }
}

bool bc_hc_sr04_measure(bc_hc_sr04_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

static void _bc_hc_sr04_task_interval(void *param)
{
    bc_hc_sr04_t *self = param;

    bc_hc_sr04_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

uint32_t pulseIn(bc_gpio_channel_t channel, uint8_t state, uint32_t timeout)
{
    uint16_t t_max = bc_timer_get_microseconds() + timeout;

    // wait for any previous pulse to end
    while (bc_gpio_get_input(channel) == state)
    {
        if (bc_timer_get_microseconds() >= t_max)
        {
            return 0;
        }
    }

    // wait for the pulse to start
    while (bc_gpio_get_input(channel) != state)
    {
        if (bc_timer_get_microseconds() >= t_max)
        {
            return 0;
        }
    }

    uint16_t t_start = bc_timer_get_microseconds();

    // wait for the pulse to stop
    while (bc_gpio_get_input(channel) == state)
    {
        if (bc_timer_get_microseconds() >= t_max)
        {
            return 0;
        }
    }

    return bc_timer_get_microseconds() - t_start;
}

static void _bc_hc_sr04_task_measure(void *param)
{
    bc_hc_sr04_t *self = param;

    bc_system_pll_enable();
    bc_timer_init();
    bc_timer_start();

    bc_gpio_set_output(self->_channel_trig, 1);
    bc_timer_delay(10);
    bc_gpio_set_output(self->_channel_trig, 0);

    self->_echo_duration = pulseIn(self->_channel_echo, 1, 30000);

    bc_timer_stop();
    bc_system_pll_disable();

    bc_hc_sr04_event_t event;
    if (self->_echo_duration > 0) {
        self->_measurement_valid = true;
        event = BC_HC_SR04_EVENT_UPDATE;
    } else {
        self->_measurement_valid = false;
        event = BC_HC_SR04_EVENT_ERROR;
    }
    self->_measurement_active = false;

    if (self->_event_handler != NULL)
    {
        self->_event_handler(self, event, self->_event_param);
    }
}

bool bc_hc_sr04_get_distance_millimeter(bc_hc_sr04_t *self, float *millimeter)
{
    if (!self->_measurement_valid)
    {
        return false;
    }

    *millimeter = (float)self->_echo_duration / 5.8;

    return true;
}
