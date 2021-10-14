#include <application.h>

/*

 SENSOR MODULE CONNECTION
==========================

Sensor Module R1.0 - 4 pin connector
VCC, GND, - , DATA

Sensor Module R1.1 - 5 pin connector
- , GND , VCC , - , DATA


 DS18B20 sensor pinout
=======================
VCC - red
GND - black
DATA- yellow (white)

*/

static twr_ds18b20_t ds18b20;


void ds18b20_event_handler(twr_ds18b20_t *self, uint64_t device_address, twr_ds18b20_event_t event, void *param)
{
    (void) param;

    float temperature = NAN;

    if (event == TWR_DS18B20_EVENT_UPDATE)
    {
        twr_ds18b20_get_temperature_celsius(self, device_address, &temperature);

        twr_log_debug("UPDATE %" PRIx64 " = %0.2f", device_address, temperature);
    }
    else
    {
        twr_log_error("%" PRIx64, device_address);
    }
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    twr_module_sensor_init();

    twr_ds18b20_init_single(&ds18b20, TWR_DS18B20_RESOLUTION_BITS_12);
    twr_ds18b20_set_event_handler(&ds18b20, ds18b20_event_handler, NULL);
    twr_ds18b20_set_update_interval(&ds18b20, 5 * 1000);
}
