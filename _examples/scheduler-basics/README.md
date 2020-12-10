#  Scheduler basic example

This example explains how the sheduler plans the tasks.
The application_task() Task is created automatically by SDK.
If you need more tasks then call the twr_scheduler_register()

This example demonstrates scheduling the task to blink a LED. The right and more
efficient way would be use just the twr_led_set_mode(&led, TWR_LED_MODE_BLINK) function
to set the blinking speed and the toggling will be handled in the background.

For more information please see http://sdk.hardwario.com/group__twr__scheduler.html
