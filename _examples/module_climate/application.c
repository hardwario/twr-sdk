#include <bc_usb_cdc.h>
#include "application.h"


void climate_event_handler(bc_module_climate_event_t event, void* params)
{
    (void) params;
    float data[4] = {0.0};

    if (event == BC_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER) {
        // get temperature
        bc_module_climate_get_temperature_celsius(&data[0]);
        bc_module_climate_get_humidity_percentage(&data[1]);
        bc_module_climate_get_illuminance_lux(&data[2]);
        bc_module_climate_get_pressure_pascal(&data[3]);

        char buffer[100];
        sprintf(buffer, "%.4f, %.4f, %.2f, %.2f", data[0], data[1], data[2], data[3]);

        bc_usb_cdc_write(buffer, strlen(buffer));
        bc_usb_cdc_write("\r\n", 2);
    }
}

void application_init(void)
{
    // initialize USB communication and lux meter tag
    bc_usb_cdc_init();

    // initialize Climate Module and set update interval to 2500 ms
    bc_module_climate_init();
    bc_module_climate_set_event_handler(climate_event_handler, NULL);
    bc_module_climate_set_update_interval_all_sensors(2500);
}