#include "application.h"

// lux meter instance
twr_opt3001_t lux;

// error message to display when error occures during lux tag update (e.g. when disconnected
// Core module
#define LUXMETER_ERROR_MSG "Error"

void lux_module_event_handler(twr_opt3001_t *self, twr_opt3001_event_t event, void *event_param) {
    (void) event_param;
    char buffer[13];
    float illumination = 0.0;

    if (event == TWR_OPT3001_EVENT_UPDATE) {
        twr_opt3001_get_illuminance_lux(self, &illumination);

        sprintf(buffer, "%012.5f", illumination);
        buffer[12] = '\0';

        twr_usb_cdc_write(buffer, strlen(buffer)+1);
        twr_usb_cdc_write("\r\n", 2);
    } else {
        twr_usb_cdc_write(LUXMETER_ERROR_MSG, strlen(LUXMETER_ERROR_MSG));
        twr_usb_cdc_write("\r\n", 2);
    }
}

void application_init(void)
{
    // initialize USB communication and lux meter tag
    twr_usb_cdc_init();
    twr_opt3001_init(&lux, TWR_I2C_I2C0, 0x44);

    // set update interval
    twr_opt3001_set_update_interval(&lux, 1000);

    // set evend handler (what to do when tag update is triggered)
    twr_opt3001_set_event_handler(&lux, lux_module_event_handler, NULL);
}
