#include <application.h>
#include <hio.h>

float temperature;

void temperature_tag_event_handler(hio_tag_temperature_t *self, hio_tag_temperature_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (hio_tag_temperature_get_temperature_celsius(self, &temperature))
        {

        }
    }
}
void application_init(void)
{
    static hio_tag_temperature_t tag;
    hio_tag_temperature_init(&tag, HIO_I2C_I2C_1W, HIO_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT);
    hio_tag_temperature_set_update_interval(&tag, 1000);
    hio_tag_temperature_set_event_handler(&tag, temperature_tag_event_handler, NULL);
}

