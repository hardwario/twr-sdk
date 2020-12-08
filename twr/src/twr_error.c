#include <twr_error.h>

void application_error(twr_error_t code);

void twr_error(twr_error_t code)
{
    application_error(code);
}
