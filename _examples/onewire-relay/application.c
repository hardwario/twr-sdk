#include <application.h>

// 1-wire relay instance
twr_onewire_relay_t relay;

// lookup table
twr_onewire_relay_channel_t q_table[] = {
    TWR_ONEWIRE_RELAY_CHANNEL_Q1,
    TWR_ONEWIRE_RELAY_CHANNEL_Q2,
    TWR_ONEWIRE_RELAY_CHANNEL_Q3,
    TWR_ONEWIRE_RELAY_CHANNEL_Q4,
    TWR_ONEWIRE_RELAY_CHANNEL_Q5,
    TWR_ONEWIRE_RELAY_CHANNEL_Q6,
    TWR_ONEWIRE_RELAY_CHANNEL_Q7,
    TWR_ONEWIRE_RELAY_CHANNEL_Q8
};

// actual active relay
int q_i = 7;

void application_init(void)
{
    twr_module_sensor_init();

    twr_module_sensor_onewire_power_up();

    twr_onewire_relay_init(&relay, twr_module_sensor_get_onewire(), 0x00);
}

void application_task()
{
    twr_onewire_relay_set_state(&relay, q_table[q_i], false);

    q_i++;

    if (q_i == 8)
    {
        q_i = 0;
    }

    twr_onewire_relay_set_state(&relay, q_table[q_i], true);

    twr_scheduler_plan_current_relative(1000);
}
