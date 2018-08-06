#include <application.h>

bc_magnetic_switch_t magnetic_switch;

void magnetic_switch_event_handler(bc_magnetic_switch_t *self, bc_magnetic_switch_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_MAGNETIC_SWITCH_EVENT_STATE_CHANGE)
    {
        bc_log_info("magnetic switch is %s", bc_magnetic_switch_is_open(self) ? "open" : "close");
    }
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);
    
    // Initialize magnetic switch for sensor module channel A
    // I used BC_GPIO_PULL_UP, because second wire is connected to GND.
    bc_magnetic_switch_init(&magnetic_switch, BC_GPIO_P4, BC_GPIO_PULL_UP);
    
    bc_magnetic_switch_set_event_handler(&magnetic_switch, magnetic_switch_event_handler, NULL);

    bc_log_info("magnetic switch is %s", bc_magnetic_switch_is_open(&magnetic_switch) ? "open" : "close");
}
