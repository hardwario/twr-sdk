/*
This example explains how the sheduler plans the tasks.
The application_task() Task is created automatically by SDK.
If you need more tasks then call the bc_scheduler_register()

This example demonstrates scheduling the task to blink a LED. The right and more
efficient way would be use just the bc_led_set_mode(&led, BC_LED_MODE_BLINK) function
to set the blinking speed and the toggling will be handled in the background.

For more information please see http://sdk.bigclown.com/group__bc__scheduler.html

*/

#include <application.h>

// LED instance
bc_led_t led;

// Setup the LED
void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);
}

// application_task() is like the Arduino loop() but more power efficient.
// At the end of the function you have to specify when
// the function will be called next. Otherwise it won't be called again.
void application_task(void)
{
    bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);

    // Shedule this function to be called 500 ms later
    // during the 500 ms the MCU will sleep and conserve power
    bc_scheduler_plan_current_relative(500);
}
