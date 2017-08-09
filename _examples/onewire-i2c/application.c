#include <application.h>
#include <bcl.h>

float temperature;

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_TAG_TEMPERATURE_EVENT_UPDATE)
    {
        if (bc_tag_temperature_get_temperature_celsius(self, &temperature))
        {

        }
    }
}
void application_init(void)
{
    static bc_tag_temperature_t tag;
    bc_tag_temperature_init(&tag, BC_I2C_I2C_1W, BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT);
    bc_tag_temperature_set_update_interval(&tag, 1000);
    bc_tag_temperature_set_event_handler(&tag, temperature_tag_event_handler, NULL);
}

