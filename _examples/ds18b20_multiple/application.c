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

#define DS18B20_SENSOR_COUNT 10

static bc_ds18b20_t ds18b20;
// ds18b20 sensors array
static bc_ds18b20_sensor_t ds18b20_sensors[DS18B20_SENSOR_COUNT];

void ds18b20_event_handler(bc_ds18b20_t *self, uint64_t device_address, bc_ds18b20_event_t e, void *p)
{
    (void) p;

    float temperature = NAN;
    int device_index;

    if (e == BC_DS18B20_EVENT_UPDATE)
    {
        bc_ds18b20_get_temperature_celsius(self, device_address, &temperature);

        device_index = bc_ds18b20_get_index_by_device_address(self, device_address);

        bc_log_debug("UPDATE %" PRIx64 "(%d) = %f", device_address, device_index, temperature);
    }
    else
    {
        bc_log_error("%" PRIx64, device_address);
    }
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);

    bc_ds18b20_init_multiple(&ds18b20, ds18b20_sensors, DS18B20_SENSOR_COUNT, BC_DS18B20_RESOLUTION_BITS_12);
    bc_ds18b20_set_event_handler(&ds18b20, ds18b20_event_handler, NULL);
    bc_ds18b20_set_update_interval(&ds18b20, 5 * 1000);
}
