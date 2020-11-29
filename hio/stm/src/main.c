#include <hio_scheduler.h>
#include <hio_system.h>
#include <hio_error.h>
#include <hio_log.h>
#include <hio_led.h>
#include <hio_timer.h>

void application_init(void);

void application_task(void *param);

void application_error(hio_error_t code);

int main(void)
{
    hio_system_init();

    while (hio_tick_get() < 500)
    {
        continue;
    }

    hio_scheduler_init();

    hio_scheduler_register(application_task, NULL, 0);

    application_init();

    hio_scheduler_run();
}

__attribute__((weak)) void application_init(void)
{
}

__attribute__((weak)) void application_task(void *param)
{
    (void) param;
}

__attribute__((weak)) void application_error(hio_error_t code)
{
#ifdef RELEASE

    (void) code;

    hio_system_reset();

#else

    hio_log_init(HIO_LOG_LEVEL_DEBUG, HIO_LOG_TIMESTAMP_ABS);

    hio_led_t led;

    hio_tick_t timeout = 0;

    int cnt = 0;

    hio_led_init(&led, HIO_GPIO_LED, false, false);

    while (true)
    {
        if (cnt == 0)
        {
            switch (code)
            {
                case HIO_ERROR_NOT_ENOUGH_TASKS:
                {
                    hio_log_error("HIO_ERROR_NOT_ENOUGH_TASKS");
                    break;
                }
                case HIO_ERROR_LOG_NOT_INITIALIZED:
                {
                    hio_log_error("HIO_ERROR_LOG_NOT_INITIALIZED");
                    break;
                }
                case HIO_ERROR_ERROR_UNLOCK:
                {
                    hio_log_error("HIO_ERROR_ERROR_UNLOCK");
                    break;
                }
                case HIO_ERROR_CALLBACK:
                {
                    hio_log_error("HIO_ERROR_CALLBACK");
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        hio_led_set_mode(&led, HIO_LED_MODE_ON);

        timeout = hio_tick_get() + ((cnt > 1 && cnt < 5) ? 1000 : 300);

        while (timeout > hio_tick_get())
        {
            continue;
        }

        hio_led_set_mode(&led, HIO_LED_MODE_OFF);

        timeout = hio_tick_get() + (cnt == 7 ? 2000 : 300);

        while (timeout > hio_tick_get())
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
