#include <application.h>


void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);
    
    bc_log_debug("Initialize bc_log by calling bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);");
    bc_log_debug("bc_log functions can use also %%s %%d and others formatting parameters.");
    
    bc_log_debug("Debug output using %s", "bc_log_debug()");
    bc_log_info("Info output using bc_log_info()");
    bc_log_warning("Warning output using bc_log_warning()");
    bc_log_error("Error output using bc_log_error()");
    
    uint8_t buffer[15];

    for (size_t i = 0; i < sizeof(buffer); i++)
    {
        buffer[i] = i;
    }

    bc_log_dump(buffer, sizeof(buffer), "Dump output using bc_log_dump()");

    char *demo = "Dump String DEMO";

    bc_log_dump(demo, strlen(demo) + 1, "Dump output using bc_log_dump()");
}
