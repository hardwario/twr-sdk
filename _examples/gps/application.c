#include <application.h>

twr_led_t led;
twr_led_t gps_led_r;
twr_led_t gps_led_g;

void gps_module_event_handler(twr_module_gps_event_t event, void *event_param);

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    twr_led_init(&led, TWR_GPIO_LED, false, false);

    if (!twr_module_gps_init())
    {
        twr_led_set_mode(&led, TWR_LED_MODE_BLINK);
    }
    else
    {
        twr_module_gps_set_event_handler(gps_module_event_handler, NULL);
        twr_module_gps_start();
    }

    twr_led_init_virtual(&gps_led_r, TWR_MODULE_GPS_LED_RED, twr_module_gps_get_led_driver(), 0);
    twr_led_init_virtual(&gps_led_g, TWR_MODULE_GPS_LED_GREEN, twr_module_gps_get_led_driver(), 0);
}

void gps_module_event_handler(twr_module_gps_event_t event, void *event_param)
{
    if (event == TWR_MODULE_GPS_EVENT_START)
    {
        twr_led_set_mode(&gps_led_g, TWR_LED_MODE_ON);
    }
    else if (event == TWR_MODULE_GPS_EVENT_STOP)
    {
        twr_led_set_mode(&gps_led_g, TWR_LED_MODE_OFF);
    }
    else if (event == TWR_MODULE_GPS_EVENT_UPDATE)
    {
        twr_led_pulse(&gps_led_r, 100);

        twr_module_gps_position_t position;

        if (twr_module_gps_get_position(&position))
        {
            twr_log_info("Lat: %03.6f", position.latitude);
            twr_log_info("Lon: %03.6f", position.longitude);
        }

        twr_module_gps_altitude_t altitude;

        if (twr_module_gps_get_altitude(&altitude))
        {
            twr_log_info("Altitude: %.1f %c", altitude.altitude, tolower(altitude.units));
        }

        twr_module_gps_quality_t quality;

        if (twr_module_gps_get_quality(&quality))
        {
            twr_log_info("Fix quality: %d", quality.fix_quality);
            twr_log_info("Satellites: %d", quality.satellites_tracked);
        }

        twr_module_gps_accuracy_t accuracy;

        if (twr_module_gps_get_accuracy(&accuracy))
        {
            twr_log_info("Horizontal accuracy: %.1f", accuracy.horizontal);
            twr_log_info("Vertical accuracy: %.1f", accuracy.vertical);
        }

        twr_module_gps_invalidate();
    }
    else if (event == TWR_MODULE_GPS_EVENT_ERROR)
    {
        twr_led_set_mode(&gps_led_g, TWR_LED_MODE_BLINK);
    }
}
