#include "application.h"

// lux meter instance
hio_opt3001_t lux;

// error message to display when error occures during lux tag update (e.g. when disconnected
// Core module
#define LUXMETER_ERROR_MSG "Error"

void lux_module_event_handler(hio_opt3001_t *self, hio_opt3001_event_t event, void *event_param) {
    (void) event_param;
    char buffer[13];
    float illumination = 0.0;

    if (event == HIO_OPT3001_EVENT_UPDATE) {
        hio_opt3001_get_illuminance_lux(self, &illumination);

        sprintf(buffer, "%012.5f", illumination);
        buffer[12] = '\0';

        hio_usb_cdc_write(buffer, strlen(buffer)+1);
        hio_usb_cdc_write("\r\n", 2);
    } else {
        hio_usb_cdc_write(LUXMETER_ERROR_MSG, strlen(LUXMETER_ERROR_MSG));
        hio_usb_cdc_write("\r\n", 2);
    }
}

void application_init(void)
{
    // initialize USB communication and lux meter tag
    hio_usb_cdc_init();
    hio_opt3001_init(&lux, HIO_I2C_I2C0, 0x44);

    // set update interval
    hio_opt3001_set_update_interval(&lux, 1000);

    // set evend handler (what to do when tag update is triggered)
    hio_opt3001_set_event_handler(&lux, lux_module_event_handler, NULL);
}
