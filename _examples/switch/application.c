#include <application.h>

// Example for Button on core module

twr_switch_t button;

// Example for Magnetic Contact Reed Switch Window Door Sensor

twr_switch_t door_sensor;

// Example for button between P0 and Vdd

twr_switch_t p0;

void button_event_handler(twr_switch_t *self, twr_switch_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_SWITCH_EVENT_OPENED)
    {
        twr_log_info("Button TWR_SWITCH_EVENT_OPENED");
    }
    else if (event == TWR_SWITCH_EVENT_CLOSED)
    {
        twr_log_info("Button TWR_SWITCH_EVENT_CLOSED");
    }

    twr_log_debug("Button state %s", twr_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void door_sensor_event_handler(twr_switch_t *self, twr_switch_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_SWITCH_EVENT_OPENED)
    {
        twr_log_info("Door TWR_SWITCH_EVENT_OPENED");
    }
    else if (event == TWR_SWITCH_EVENT_CLOSED)
    {
        twr_log_info("Door TWR_SWITCH_EVENT_CLOSED");
    }

    twr_log_debug("Door state %s", twr_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void p0_event_handler(twr_switch_t *self, twr_switch_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_SWITCH_EVENT_OPENED)
    {
        twr_log_info("P0 TWR_SWITCH_EVENT_OPENED");
    }
    else if (event == TWR_SWITCH_EVENT_CLOSED)
    {
        twr_log_info("P0 TWR_SWITCH_EVENT_CLOSED");
    }

    twr_log_debug("P0 state %s", twr_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    twr_switch_init(&button, TWR_GPIO_BUTTON, TWR_SWITCH_TYPE_NO, TWR_SWITCH_PULL_NONE);

    twr_switch_set_event_handler(&button, button_event_handler, NULL);

    twr_switch_init(&door_sensor, TWR_GPIO_P4, TWR_SWITCH_TYPE_NC, TWR_SWITCH_PULL_UP_DYNAMIC);

    twr_switch_set_event_handler(&door_sensor, door_sensor_event_handler, NULL);

    twr_switch_init(&p0, TWR_GPIO_P0, TWR_SWITCH_TYPE_NO, TWR_SWITCH_PULL_DOWN);

    twr_switch_set_event_handler(&p0, p0_event_handler, NULL);
}
