#include <application.h>
#include <bcl.h>
#include <bc_module_sensor.h>
#include <bc_1wire_relay.h>

// 1-wire relay instance
bc_1wire_relay_t relay;

// lookup table
bc_1wire_relay_q_t q_table[] = {
	BC_1WIRE_RELAY_Q1,
	BC_1WIRE_RELAY_Q2,
	BC_1WIRE_RELAY_Q3,
	BC_1WIRE_RELAY_Q4,
	BC_1WIRE_RELAY_Q5,
	BC_1WIRE_RELAY_Q6,
	BC_1WIRE_RELAY_Q7,
	BC_1WIRE_RELAY_Q8
};

// actual active relay
int q_i = 0;

void application_init(void)
{
	bc_1wire_relay_init(&relay, 0x00, BC_MODULE_SENSOR_CHANNEL_A);
}

void application_task()
{

	bc_1wire_relay_set_state(&relay, q_table[q_i], false);

	q_i++;

	if (q_i == 8)
	{
		q_i = 0;
	}

	bc_1wire_relay_set_state(&relay, q_table[q_i], true);

    bc_scheduler_plan_current_relative(1000);

}
