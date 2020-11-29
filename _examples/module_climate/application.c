#include "application.h"


void climate_event_handler(hio_module_climate_event_t event, void* params)
{
    (void) params;
    float data[4] = {0.0};

    if (event == HIO_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER) {
        // get temperature
        hio_module_climate_get_temperature_celsius(&data[0]);
        hio_module_climate_get_humidity_percentage(&data[1]);
        hio_module_climate_get_illuminance_lux(&data[2]);
        hio_module_climate_get_pressure_pascal(&data[3]);

        char buffer[100];
        sprintf(buffer, "%.4f, %.4f, %.2f, %.2f", data[0], data[1], data[2], data[3]);

        hio_usb_cdc_write(buffer, strlen(buffer));
        hio_usb_cdc_write("\r\n", 2);
    }
}

void application_init(void)
{
    // initialize USB communication and lux meter tag
    hio_usb_cdc_init();

    // initialize Climate Module and set update interval to 2500 ms
    hio_module_climate_init();
    hio_module_climate_set_event_handler(climate_event_handler, NULL);
    hio_module_climate_set_update_interval_all_sensors(2500);
}
