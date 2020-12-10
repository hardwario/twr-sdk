#include <application.h>
#include <twr.h>

float temperature;

void temperature_tag_event_handler(twr_tag_temperature_t *self, twr_tag_temperature_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (twr_tag_temperature_get_temperature_celsius(self, &temperature))
        {

        }
    }
}
void application_init(void)
{
    static twr_tag_temperature_t tag;
    twr_tag_temperature_init(&tag, TWR_I2C_I2C_1W, TWR_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT);
    twr_tag_temperature_set_update_interval(&tag, 1000);
    twr_tag_temperature_set_event_handler(&tag, temperature_tag_event_handler, NULL);
}

