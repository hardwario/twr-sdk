#include <bc_usb_cdc.h>
#include "application.h"

// lux meter instance
bc_opt3001_t lux;

// error message to display when error occures during lux tag update (e.g. when disconnected
// Core module
#define LUXMETER_ERROR_MSG "Error"

void lux_module_event_handler(bc_opt3001_t *self, bc_opt3001_event_t event, void *event_param) {
    (void) event_param;
    char buffer[13];
    float illumination = 0.0;

    if (event == BC_OPT3001_EVENT_UPDATE) {
        bc_opt3001_get_illuminance_lux(self, &illumination);

        sprintf(buffer, "%012.5f", illumination);
        buffer[12] = '\0';

        bc_usb_cdc_write(buffer, strlen(buffer)+1);
        bc_usb_cdc_write("\r\n", 2);
    } else {
        bc_usb_cdc_write(LUXMETER_ERROR_MSG, strlen(LUXMETER_ERROR_MSG));
        bc_usb_cdc_write("\r\n", 2);
    }
}

void application_init(void)
{
    // initialize USB communication and lux meter tag
    bc_usb_cdc_init();
    bc_opt3001_init(&lux, BC_I2C_I2C0, 0x44);

    // set update interval
    bc_opt3001_set_update_interval(&lux, 1000);

    // set evend handler (what to do when tag update is triggered)
    bc_opt3001_set_event_handler(&lux, lux_module_event_handler, NULL);
}