/*
This example explains how the sheduler plans the tasks.
The application_task() Task is created automatically by SDK.
If you need more tasks then call the twr_scheduler_register()

This example demonstrates scheduling the task to blink a LED. The right and more
efficient way would be use just the twr_led_set_mode(&led, TWR_LED_MODE_BLINK) function
to set the blinking speed and the toggling will be handled in the background.

For more information please see http://sdk.hardwario.com/group__twr__scheduler.html

*/

#include <application.h>

// LED instance
twr_led_t led;

// Setup the LED
void application_init(void)
{
    twr_led_init(&led, TWR_GPIO_LED, false, false);
}

// application_task() is like the Arduino loop() but more power efficient.
// At the end of the function you have to specify when
// the function will be called next. Otherwise it won't be called again.
void application_task(void)
{
    twr_led_set_mode(&led, TWR_LED_MODE_TOGGLE);

    // Shedule this function to be called 500 ms later
    // during the 500 ms the MCU will sleep and conserve power
    twr_scheduler_plan_current_relative(500);
}
