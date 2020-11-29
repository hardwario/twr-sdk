#include <application.h>


void application_init(void)
{
    hio_log_init(HIO_LOG_LEVEL_DUMP, HIO_LOG_TIMESTAMP_ABS);

    hio_log_debug("Initialize hio_log by calling hio_log_init(HIO_LOG_LEVEL_DEBUG, HIO_LOG_TIMESTAMP_ABS);");
    hio_log_debug("hio_log functions can use also %%s %%d and others formatting parameters.");

    hio_log_debug("Debug output using %s", "hio_log_debug()");
    hio_log_info("Info output using hio_log_info()");
    hio_log_warning("Warning output using hio_log_warning()");
    hio_log_error("Error output using hio_log_error()");

    uint8_t buffer[15];

    for (size_t i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = i;
    }

    hio_log_dump(buffer, sizeof(buffer), "Dump output using hio_log_dump()");

    char *demo = "Dump String DEMO";

    hio_log_dump(demo, strlen(demo) + 1, "Dump output using hio_log_dump()");
}
