#include <application.h>

bc_led_t led;
bc_led_t gps_led_r;
bc_led_t gps_led_g;

void gps_module_event_handler(bc_module_gps_event_t event, void *event_param);

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    bc_led_init(&led, BC_GPIO_LED, false, false);

    if (!bc_module_gps_init())
    {
        bc_led_set_mode(&led, BC_LED_MODE_BLINK);
    }
    else
    {
        bc_module_gps_set_event_handler(gps_module_event_handler, NULL);
        bc_module_gps_start();
    }

    bc_led_init_virtual(&gps_led_r, BC_MODULE_GPS_LED_RED, bc_module_gps_get_led_driver(), 0);
    bc_led_init_virtual(&gps_led_g, BC_MODULE_GPS_LED_GREEN, bc_module_gps_get_led_driver(), 0);
}

void gps_module_event_handler(bc_module_gps_event_t event, void *event_param)
{
    if (event == BC_MODULE_GPS_EVENT_START)
    {
        bc_led_set_mode(&gps_led_g, BC_LED_MODE_ON);
    }
    else if (event == BC_MODULE_GPS_EVENT_STOP)
    {
        bc_led_set_mode(&gps_led_g, BC_LED_MODE_OFF);
    }
    else if (event == BC_MODULE_GPS_EVENT_UPDATE)
    {
        bc_led_pulse(&gps_led_r, 100);

        bc_module_gps_position_t position;

        if (bc_module_gps_get_position(&position))
        {
            bc_log_info("Lat: %03.6f", position.latitude);
            bc_log_info("Lon: %03.6f", position.longitude);
        }

        bc_module_gps_altitude_t altitude;

        if (bc_module_gps_get_altitude(&altitude))
        {
            bc_log_info("Altitude: %.1f %c", altitude.altitude, tolower(altitude.units));
        }

        bc_module_gps_quality_t quality;

        if (bc_module_gps_get_quality(&quality))
        {
            bc_log_info("Fix quality: %d", quality.fix_quality);
            bc_log_info("Satellites: %d", quality.satellites_tracked);
        }

        bc_module_gps_accuracy_t accuracy;

        if (bc_module_gps_get_accuracy(&accuracy))
        {
            bc_log_info("Horizontal accuracy: %.1f", accuracy.horizontal);
            bc_log_info("Vertical accuracy: %.1f", accuracy.vertical);
        }

        bc_module_gps_invalidate();
    }
    else if (event == BC_MODULE_GPS_EVENT_ERROR)
    {
        bc_led_set_mode(&gps_led_g, BC_LED_MODE_BLINK);
    }
}
