#include <bc_scheduler.h>
#include <bc_system.h>
#include <bc_error.h>
#include <bc_log.h>
#include <bc_led.h>
#include <bc_timer.h>

void application_init(void);

void application_task(void *param);

void application_error(bc_error_t code);

int main(void)
{
    bc_system_init();

    while (bc_tick_get() < 500)
    {
        continue;
    }

    bc_scheduler_init();

    bc_scheduler_register(application_task, NULL, 0);

    application_init();

    bc_scheduler_run();
}

__attribute__((weak)) void application_init(void)
{
}

__attribute__((weak)) void application_task(void *param)
{
    (void) param;
}

__attribute__((weak)) void application_error(bc_error_t code)
{

#ifdef RELEASE

    bc_system_reset();

#else

    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);

    bc_led_t led;

    bc_tick_t timeout = 0;

    int cnt = 0;

    bc_led_init(&led, BC_GPIO_LED, false, false);

    while (true)
    {
        if (cnt == 0)
        {
            switch (code)
            {
                case BC_ERROR_NOT_ENOUGH_TASKS:
                {
                    bc_log_error("BC_ERROR_NOT_ENOUGH_TASKS");
                    break;
                }
                case BC_ERROR_LOG_NOT_INITIALIZED:
                {
                    bc_log_error("BC_ERROR_LOG_NOT_INITIALIZED");
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        bc_led_set_mode(&led, BC_LED_MODE_ON);

        timeout = bc_tick_get() + ((cnt > 1 && cnt < 5) ? 1000 : 300);

        while (timeout > bc_tick_get())
        {
            continue;
        }

        bc_led_set_mode(&led, BC_LED_MODE_OFF);

        timeout = bc_tick_get() + (cnt == 7 ? 2000 : 300);

        while (timeout > bc_tick_get())
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
