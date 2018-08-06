#include <application.h>

// Example for Button on core module

bc_switch_t button;

// Example for Magnetic Contact Reed Switch Window Door Sensor

bc_switch_t door_sensor;

// Example for button between P0 and Vdd

bc_switch_t p0;

void button_event_handler(bc_switch_t *self, bc_switch_event_t event, void *event_param)
{
    if (event == BC_SWITCH_EVENT_OPENED)
    {
        bc_log_info("Button BC_SWITCH_EVENT_OPENED");
    }
    else if (event == BC_SWITCH_EVENT_CLOSED)
    {
        bc_log_info("Button BC_SWITCH_EVENT_CLOSED");
    }

    bc_log_debug("Button state %s", bc_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void door_sensor_event_handler(bc_switch_t *self, bc_switch_event_t event, void *event_param)
{
    if (event == BC_SWITCH_EVENT_OPENED)
    {
        bc_log_info("Door BC_SWITCH_EVENT_OPENED");
    }
    else if (event == BC_SWITCH_EVENT_CLOSED)
    {
        bc_log_info("Door BC_SWITCH_EVENT_CLOSED");
    }

    bc_log_debug("Door state %s", bc_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void p0_event_handler(bc_switch_t *self, bc_switch_event_t event, void *event_param)
{
    if (event == BC_SWITCH_EVENT_OPENED)
    {
        bc_log_info("P0 BC_SWITCH_EVENT_OPENED");
    }
    else if (event == BC_SWITCH_EVENT_CLOSED)
    {
        bc_log_info("P0 BC_SWITCH_EVENT_CLOSED");
    }

    bc_log_debug("P0 state %s", bc_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);

    bc_switch_init(&button, BC_GPIO_BUTTON, BC_SWITCH_TYPE_NO, BC_SWITCH_PULL_NONE);

    bc_switch_set_event_handler(&button, button_event_handler, NULL);

    bc_switch_init(&door_sensor, BC_GPIO_P4, BC_SWITCH_TYPE_NC, BC_SWITCH_PULL_UP_DYNAMIC);

    bc_switch_set_event_handler(&door_sensor, door_sensor_event_handler, NULL);

    bc_switch_init(&p0, BC_GPIO_P0, BC_SWITCH_TYPE_NO, BC_SWITCH_PULL_DOWN);

    bc_switch_set_event_handler(&p0, p0_event_handler, NULL);
}
