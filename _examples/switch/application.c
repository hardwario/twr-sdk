#include <application.h>

// Example for Button on core module

hio_switch_t button;

// Example for Magnetic Contact Reed Switch Window Door Sensor

hio_switch_t door_sensor;

// Example for button between P0 and Vdd

hio_switch_t p0;

void button_event_handler(hio_switch_t *self, hio_switch_event_t event, void *event_param)
{
    if (event == HIO_SWITCH_EVENT_OPENED)
    {
        hio_log_info("Button HIO_SWITCH_EVENT_OPENED");
    }
    else if (event == HIO_SWITCH_EVENT_CLOSED)
    {
        hio_log_info("Button HIO_SWITCH_EVENT_CLOSED");
    }

    hio_log_debug("Button state %s", hio_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void door_sensor_event_handler(hio_switch_t *self, hio_switch_event_t event, void *event_param)
{
    if (event == HIO_SWITCH_EVENT_OPENED)
    {
        hio_log_info("Door HIO_SWITCH_EVENT_OPENED");
    }
    else if (event == HIO_SWITCH_EVENT_CLOSED)
    {
        hio_log_info("Door HIO_SWITCH_EVENT_CLOSED");
    }

    hio_log_debug("Door state %s", hio_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void p0_event_handler(hio_switch_t *self, hio_switch_event_t event, void *event_param)
{
    if (event == HIO_SWITCH_EVENT_OPENED)
    {
        hio_log_info("P0 HIO_SWITCH_EVENT_OPENED");
    }
    else if (event == HIO_SWITCH_EVENT_CLOSED)
    {
        hio_log_info("P0 HIO_SWITCH_EVENT_CLOSED");
    }

    hio_log_debug("P0 state %s", hio_switch_get_state(self) ? "OPEN" : "CLOSE");
}

void application_init(void)
{
    hio_log_init(HIO_LOG_LEVEL_DEBUG, HIO_LOG_TIMESTAMP_ABS);

    hio_switch_init(&button, HIO_GPIO_BUTTON, HIO_SWITCH_TYPE_NO, HIO_SWITCH_PULL_NONE);

    hio_switch_set_event_handler(&button, button_event_handler, NULL);

    hio_switch_init(&door_sensor, HIO_GPIO_P4, HIO_SWITCH_TYPE_NC, HIO_SWITCH_PULL_UP_DYNAMIC);

    hio_switch_set_event_handler(&door_sensor, door_sensor_event_handler, NULL);

    hio_switch_init(&p0, HIO_GPIO_P0, HIO_SWITCH_TYPE_NO, HIO_SWITCH_PULL_DOWN);

    hio_switch_set_event_handler(&p0, p0_event_handler, NULL);
}
