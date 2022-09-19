/*
This example explains how to use scheduler to handle more tasks
*/

#include <application.h>

// LED instance
twr_led_t led;

// To control the tasks we need to save task IDs
twr_scheduler_task_id_t id_task_turn_on;
twr_scheduler_task_id_t id_task_turn_off;

void task_turn_on(void *param)
{
    // Convert param from void* type to twr_led_t
    twr_led_t *param_led = (twr_led_t*)param;
    twr_led_set_mode(param_led, TWR_LED_MODE_ON);
    // Schedule the start of the LED turn off task in 500 ms
    twr_scheduler_plan_relative(id_task_turn_off, 500);
}

void task_turn_off(void *param)
{
    twr_led_t *param_led = (twr_led_t*)param;
    twr_led_set_mode(param_led, TWR_LED_MODE_OFF);
}

// Setup the LED and register two tasks
void application_init(void)
{
    twr_led_init(&led, TWR_GPIO_LED, false, false);

    // Register two tasks
    // We pass the &led pointer as a parameter to the task functions
    // Also register function returns the task ID which we use later.
    // TWR_TICK_INFINITY means that the task is not run automatically
    id_task_turn_on = twr_scheduler_register(task_turn_on, &led, TWR_TICK_INFINITY);
    id_task_turn_off = twr_scheduler_register(task_turn_off, &led, TWR_TICK_INFINITY);
}

void application_task(void)
{
    // Start the task to turn LED on
    twr_scheduler_plan_now(id_task_turn_on);
    // Schedule calling this function again in 1000 ms
    twr_scheduler_plan_current_relative(1000);
}
