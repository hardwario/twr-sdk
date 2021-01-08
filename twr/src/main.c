#include <twr_scheduler.h>
#include <twr_system.h>
#include <twr_error.h>
#include <twr_log.h>
#include <twr_led.h>
#include <twr_timer.h>
#include <twr_sleep.h>

void application_init(void);

void application_task(void *param);

void application_error(twr_error_t code);

int main(void)
{
    twr_system_init();

    while (twr_tick_get() < 500)
    {
        continue;
    }

    twr_scheduler_init();

    twr_scheduler_register(application_task, NULL, 0);

    application_init();

    twr_scheduler_run();
}

__attribute__((weak)) void application_init(void)
{
}

__attribute__((weak)) void application_task(void *param)
{
    (void) param;
}

__attribute__((weak)) void application_idle()
{
    twr_sleep();
}

__attribute__((weak)) void application_error(twr_error_t code)
{
#ifdef RELEASE

    (void) code;

    twr_system_reset();

#else

    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    twr_tick_t timeout = 0;

    int cnt = 0;

    twr_gpio_set_mode(TWR_GPIO_LED, TWR_GPIO_MODE_OUTPUT);
    twr_gpio_set_output(TWR_GPIO_LED, 0);

    while (true)
    {
        if (cnt % 3 == 0)
        {
            switch (code)
            {
                case TWR_ERROR_NOT_ENOUGH_TASKS:
                {
                    twr_log_error("TWR_ERROR_NOT_ENOUGH_TASKS");
                    break;
                }
                case TWR_ERROR_LOG_NOT_INITIALIZED:
                {
                    twr_log_error("TWR_ERROR_LOG_NOT_INITIALIZED");
                    break;
                }
                case TWR_ERROR_ERROR_UNLOCK:
                {
                    twr_log_error("TWR_ERROR_ERROR_UNLOCK");
                    break;
                }
                case TWR_ERROR_CALLBACK:
                {
                    twr_log_error("TWR_ERROR_CALLBACK");
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        twr_gpio_set_output(TWR_GPIO_LED, 1);

        timeout = twr_tick_get() + ((cnt > 1 && cnt < 5) ? 1000 : 300);

        while (timeout > twr_tick_get())
        {
            continue;
        }

        twr_gpio_set_output(TWR_GPIO_LED, 0);

        timeout = twr_tick_get() + (cnt == 7 ? 2000 : 300);

        while (timeout > twr_tick_get())
        {
            continue;
        }

        if (cnt++ == 8)
        {
            cnt = 0;
        }
    }

#endif
}

void NMI_Handler(void)
{
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}
