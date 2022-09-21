#include <application.h>

twr_button_t button;

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        size_t eeprom = twr_eeprom_get_size();
        char readEeprom[13];
        float readFloat;

        twr_eeprom_read(0, &readFloat, 4);
        twr_eeprom_read(4, readEeprom, 12);
        readEeprom[12] = '\0';

        twr_log_debug("EEPROM size: %d\r\nData:\r\n%f\r\n%s", eeprom, readFloat, readEeprom);
    }
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    float toWriteFloat = 3.14159;
    char toWrite[] = "hello world!";
    twr_eeprom_write(0, &toWriteFloat, sizeof(toWriteFloat));
    twr_eeprom_write(sizeof(toWriteFloat), toWrite, sizeof(toWrite));

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);
}
