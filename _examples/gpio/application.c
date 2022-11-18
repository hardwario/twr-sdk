#include <application.h>

void application_init(void)
{
    twr_gpio_init(TWR_GPIO_LED);
    twr_gpio_set_mode(TWR_GPIO_LED, TWR_GPIO_MODE_OUTPUT);

    twr_gpio_init(TWR_GPIO_BUTTON);
    twr_gpio_set_mode(TWR_GPIO_BUTTON, TWR_GPIO_MODE_INPUT);

    // The Core Module has hardware pull-down so next call is commented out
    // twr_gpio_set_pull(TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN);
}

void application_task()
{
    uint8_t button_state = twr_gpio_get_input(TWR_GPIO_BUTTON);
    twr_gpio_set_output(TWR_GPIO_LED, button_state);

    // Repeat this task again after 10 ms
    twr_scheduler_plan_current_relative(10);
}
