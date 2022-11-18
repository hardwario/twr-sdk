#include <application.h>

// Example structure that save configuration of PIR detector
typedef struct config_t
{
    uint16_t report_interval;
    uint8_t pir_sensitivity;
    uint16_t pir_deadtime;

} config_t;

config_t config;

void application_init()
{
    // Load configuration
    twr_config_init(0x12345678, &config, sizeof(config), NULL);

    // Change parameter
    config.report_interval = 500;

    // Save config to EEPROM
    twr_config_save();

    // Reset configuration
    twr_config_reset();
}
