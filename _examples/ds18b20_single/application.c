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

static bc_ds18b20_t ds18b20;


void ds18b20_event_handler(bc_ds18b20_t *self, uint64_t device_address, bc_ds18b20_event_t event, void *param)
{
    (void) param;

    float temperature = NAN;

    if (event == BC_DS18B20_EVENT_UPDATE)
    {
        bc_ds18b20_get_temperature_celsius(self, device_address, &temperature);

        bc_log_debug("UPDATE %" PRIx64 " = %0.2f", device_address, temperature);
    }
    else
    {
        bc_log_error("%" PRIx64, device_address);
    }
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);

    bc_ds18b20_init_single(&ds18b20, BC_DS18B20_RESOLUTION_BITS_12);
    bc_ds18b20_set_event_handler(&ds18b20, ds18b20_event_handler, NULL);
    bc_ds18b20_set_update_interval(&ds18b20, 5 * 1000);
}
