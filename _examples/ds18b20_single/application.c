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

static hio_ds18b20_t ds18b20;


void ds18b20_event_handler(hio_ds18b20_t *self, uint64_t device_address, hio_ds18b20_event_t event, void *param)
{
    (void) param;

    float temperature = NAN;

    if (event == HIO_DS18B20_EVENT_UPDATE)
    {
        hio_ds18b20_get_temperature_celsius(self, device_address, &temperature);

        hio_log_debug("UPDATE %" PRIx64 " = %0.2f", device_address, temperature);
    }
    else
    {
        hio_log_error("%" PRIx64, device_address);
    }
}

void application_init(void)
{
    hio_log_init(HIO_LOG_LEVEL_DEBUG, HIO_LOG_TIMESTAMP_ABS);

    hio_ds18b20_init_single(&ds18b20, HIO_DS18B20_RESOLUTION_BITS_12);
    hio_ds18b20_set_event_handler(&ds18b20, ds18b20_event_handler, NULL);
    hio_ds18b20_set_update_interval(&ds18b20, 5 * 1000);
}
