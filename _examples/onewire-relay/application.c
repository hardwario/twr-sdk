#include <application.h>

// 1-wire relay instance
hio_onewire_relay_t relay;

// lookup table
hio_onewire_relay_channel_t q_table[] = {
    HIO_ONEWIRE_RELAY_CHANNEL_Q1,
    HIO_ONEWIRE_RELAY_CHANNEL_Q2,
    HIO_ONEWIRE_RELAY_CHANNEL_Q3,
    HIO_ONEWIRE_RELAY_CHANNEL_Q4,
    HIO_ONEWIRE_RELAY_CHANNEL_Q5,
    HIO_ONEWIRE_RELAY_CHANNEL_Q6,
    HIO_ONEWIRE_RELAY_CHANNEL_Q7,
    HIO_ONEWIRE_RELAY_CHANNEL_Q8
};

// actual active relay
int q_i = 7;

void application_init(void)
{
    hio_module_sensor_init();

    hio_module_sensor_onewire_power_up();

    hio_onewire_relay_init(&relay, hio_module_sensor_get_onewire(), 0x00);
}

void application_task()
{
    hio_onewire_relay_set_state(&relay, q_table[q_i], false);

    q_i++;

    if (q_i == 8)
    {
        q_i = 0;
    }

    hio_onewire_relay_set_state(&relay, q_table[q_i], true);

    hio_scheduler_plan_current_relative(1000);
}
