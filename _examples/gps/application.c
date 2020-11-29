#include <application.h>

hio_led_t led;
hio_led_t gps_led_r;
hio_led_t gps_led_g;

void gps_module_event_handler(hio_module_gps_event_t event, void *event_param);

void application_init(void)
{
    hio_log_init(HIO_LOG_LEVEL_DUMP, HIO_LOG_TIMESTAMP_ABS);

    hio_led_init(&led, HIO_GPIO_LED, false, false);

    if (!hio_module_gps_init())
    {
        hio_led_set_mode(&led, HIO_LED_MODE_BLINK);
    }
    else
    {
        hio_module_gps_set_event_handler(gps_module_event_handler, NULL);
        hio_module_gps_start();
    }

    hio_led_init_virtual(&gps_led_r, HIO_MODULE_GPS_LED_RED, hio_module_gps_get_led_driver(), 0);
    hio_led_init_virtual(&gps_led_g, HIO_MODULE_GPS_LED_GREEN, hio_module_gps_get_led_driver(), 0);
}

void gps_module_event_handler(hio_module_gps_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_MODULE_GPS_EVENT_START)
    {
        hio_led_set_mode(&gps_led_g, HIO_LED_MODE_ON);
    }
    else if (event == HIO_MODULE_GPS_EVENT_STOP)
    {
        hio_led_set_mode(&gps_led_g, HIO_LED_MODE_OFF);
    }
    else if (event == HIO_MODULE_GPS_EVENT_UPDATE)
    {
        hio_led_pulse(&gps_led_r, 100);

        hio_module_gps_position_t position;

        if (hio_module_gps_get_position(&position))
        {
            hio_log_info("Lat: %03.6f", position.latitude);
            hio_log_info("Lon: %03.6f", position.longitude);
        }

        hio_module_gps_altitude_t altitude;

        if (hio_module_gps_get_altitude(&altitude))
        {
            hio_log_info("Altitude: %.1f %c", altitude.altitude, tolower(altitude.units));
        }

        hio_module_gps_quality_t quality;

        if (hio_module_gps_get_quality(&quality))
        {
            hio_log_info("Fix quality: %d", quality.fix_quality);
            hio_log_info("Satellites: %d", quality.satellites_tracked);
        }

        hio_module_gps_accuracy_t accuracy;

        if (hio_module_gps_get_accuracy(&accuracy))
        {
            hio_log_info("Horizontal accuracy: %.1f", accuracy.horizontal);
            hio_log_info("Vertical accuracy: %.1f", accuracy.vertical);
        }

        hio_module_gps_invalidate();
    }
    else if (event == HIO_MODULE_GPS_EVENT_ERROR)
    {
        hio_led_set_mode(&gps_led_g, HIO_LED_MODE_BLINK);
    }
}
