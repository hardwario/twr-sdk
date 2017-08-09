#include <application.h>
#include <bc_onewire_relay.h>

// 1-wire relay instance
bc_onewire_relay_t relay;

// lookup table
bc_onewire_relay_channel_t q_table[] = {
    BC_ONEWIRE_RELAY_CHANNEL_Q1,
    BC_ONEWIRE_RELAY_CHANNEL_Q2,
    BC_ONEWIRE_RELAY_CHANNEL_Q3,
    BC_ONEWIRE_RELAY_CHANNEL_Q4,
    BC_ONEWIRE_RELAY_CHANNEL_Q5,
    BC_ONEWIRE_RELAY_CHANNEL_Q6,
    BC_ONEWIRE_RELAY_CHANNEL_Q7,
    BC_ONEWIRE_RELAY_CHANNEL_Q8
};

// actual active relay
int q_i = 7;

void application_init(void)
{
    bc_module_sensor_init();

    bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_A, BC_MODULE_SENSOR_PULL_UP_4K7);

    bc_onewire_relay_init(&relay, BC_GPIO_P4, 0x00);
}

void application_task()
{
    bc_onewire_relay_set_state(&relay, q_table[q_i], false);

    q_i++;

    if (q_i == 8)
    {
        q_i = 0;
    }

    bc_onewire_relay_set_state(&relay, q_table[q_i], true);

    bc_scheduler_plan_current_relative(1000);
}
