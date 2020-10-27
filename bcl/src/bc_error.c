#include <bc_error.h>

void application_error(bc_error_t code);

void bc_error(bc_error_t code)
{
    application_error(code);
}
