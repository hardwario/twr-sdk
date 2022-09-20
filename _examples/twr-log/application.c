#include <application.h>


void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DUMP, TWR_LOG_TIMESTAMP_ABS);

    twr_log_debug("Initialize twr_log by calling twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);");
    twr_log_debug("twr_log functions can use also %%s %%d and others formatting parameters.");

    twr_log_debug("Debug output using %s", "twr_log_debug()");
    twr_log_info("Info output using twr_log_info()");
    twr_log_warning("Warning output using twr_log_warning()");
    twr_log_error("Error output using twr_log_error()");

    uint8_t buffer[15];

    for (size_t i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = i;
    }

    twr_log_dump(buffer, sizeof(buffer), "Dump output using twr_log_dump()");

    char *demo = "Dump String DEMO";

    twr_log_dump(demo, strlen(demo) + 1, "Dump output using twr_log_dump()");
}
